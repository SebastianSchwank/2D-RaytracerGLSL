#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision highp float;
#endif

//RT2DShader

varying vec2 v_texcoord;

uniform sampler2D CalculatedImage;
uniform int numRenderPass;

uniform sampler2D Objects;
uniform int numObjects;

uniform sampler2D random;

uniform int width;
uniform int height;

uniform float period;

const float pi = 3.14159265359;

uniform float seedX,seedY;

uniform bool fbo;

// Unpacking a [0-1] float value from a 4D vector where each component was a 8-bits integer
float unpack(const vec4 value)
{
   const vec4 bitSh = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
   return(dot(value, bitSh));
}

//Calculate intersection between a Vector and a Line
vec3 lineSegmentIntersection(vec2 r0, vec2 r1, vec2 a, vec2 b)
{
    vec2 s1, s2;
    s1 = r1 - r0;
    s2 = b - a;

    highp float s, t;
    s = (-s1.y * (r0.x - a.x) + s1.x * (r0.y - a.y)) / (-s2.x * s1.y + s1.x * s2.y);
    t = (s2.x * (r0.y - a.y) - s2.y * (r0.x - a.x)) / (-s2.x * s1.y + s1.x * s2.y);

    if ((s >= 0.0) && (s <= 1.0) && (t >= 0.0) && (t <= 1.0))
    {
        // Collision detected
        // Return the point of intersection
        float xI = r0.x + (t * s1.x);
        float yI = r0.y + (t * s1.y);
        float dist = sqrt((r0.x-xI)*(r0.x-xI)+(r0.y-yI)*(r0.y-yI));
        return vec3(xI, yI, dist);
    }

    return vec3(0,0,-1.0); // No collision
}

//Calculate reflectionangle
vec2 reflect(vec2 V,vec2 a,vec2 b){
    vec2 N = vec2(-(b-a).y,(b-a).x);
         N = N / sqrt((N.x*N.x)+(N.y*N.y));
    vec2 O = V - 2 * dot(V,N) * N;
    //O = 9.0*O/sqrt((O.x+O.x)+(O.y+O.y));
    return O;
}

//Emulation of the Old Texel Fetch in OGL
vec4 texelFetch(sampler2D smplr,vec2 coords,vec2 size){
    return texture2D(smplr, vec2( (2.0*coords.x + 1.0)/(2.0*size.x),(2.0*coords.y + 1.0)/(2.0*size.y)) );
}

void main()
{
    vec4 renderedImagePixel = texture2D(CalculatedImage,v_texcoord);

    if(fbo){

        float randomFl0 = unpack(texture2D(random,vec2((v_texcoord.x+seedX)/2.0,(v_texcoord.y+seedY))/2.0));
        float randomFl1 = unpack(texture2D(random,vec2((v_texcoord.x+seedY)/2.0,(v_texcoord.y+seedX))/2.0));

        //Supersampling
        float x = v_texcoord.x + randomFl0/width;
        float y = v_texcoord.y + randomFl1/height;
        //Random Angle Calculation
        float alpha = randomFl0*pi*2.0;
        vec2 ray = vec2(cos(alpha)*9.0,sin(alpha)*9.0);

        vec3 intersecBuffer = vec3(0.0,0.0,9999.9999);
        float accZBuffer = 0.0;
        int zIndex = -1;
        int prevZIndex = -2;

        vec4 mulColor = vec4(1.0,1.0,1.0,1.0);

        for(int j = 0; j < numObjects; j++){
            if(j != prevZIndex){
                highp float x1,x2,y1,y2;
                x1 = unpack(texelFetch(Objects,vec2(j,8),vec2(numObjects,9)));
                y1 = unpack(texelFetch(Objects,vec2(j,7),vec2(numObjects,9)));
                x2 = unpack(texelFetch(Objects,vec2(j,6),vec2(numObjects,9)));
                y2 = unpack(texelFetch(Objects,vec2(j,5),vec2(numObjects,9)));

                vec3 currBuff = lineSegmentIntersection(vec2(x,y),ray,
                                                        vec2(x1,y1),vec2(x2,y2));

                if( (currBuff.z < intersecBuffer.z) && (currBuff.z > 0.0) ){
                    intersecBuffer = currBuff;
                    zIndex = j;
                }
            }
        }


        vec4 color = mulColor * texelFetch(Objects,ivec2(zIndex,3),vec2(numObjects,9));

        float phase = unpack(texelFetch(Objects,ivec2(zIndex,2),vec2(numObjects,9)));
        float wavelength = unpack(texelFetch(Objects,ivec2(zIndex,1),vec2(numObjects,9)));

        float bright = unpack(texelFetch(Objects,ivec2(zIndex,4),vec2(numObjects,9)));

        float amplitude = 5.0 * bright *
                          (1.0+sin(accZBuffer*2.0*pi*wavelength*100.0 +
                                   phase * 2.0 * pi +
                                   period * 2.0 * pi))/2.0;

        //color = vec4(gl_TexCoord[0].st.x,gl_TexCoord[0].st.y,0.0,0.0);

        highp vec4 currTexel = vec4(amplitude*color.r,amplitude*color.g,amplitude*color.b,1.0);

        //if(prevZIndex != -2) currTexel = vec4(1.0,0,0,1.0);
        renderedImagePixel = (renderedImagePixel * float(numRenderPass) + currTexel)/float(numRenderPass+1);

        renderedImagePixel = vec4(unpack(texelFetch(Objects,vec2(x*numObjects,y*8.0),vec2(float(numObjects),8.0))));
        //renderedImagePixel = vec4(intersecBuffer.x,intersecBuffer.y,intersecBuffer.z,1.0);
    }

    gl_FragColor = renderedImagePixel;

}

