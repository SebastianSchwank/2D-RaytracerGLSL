#version 400

//RT2DShader

uniform sampler2D CalculatedImage;
uniform int numRenderPass;

uniform sampler2D Objects;
uniform int numObjects;

uniform int width;
uniform int height;

uniform float period;

const float pi = 3.14159265359;

uniform float seed = pi;

// Unpacking a [0-1] float value from a 4D vector where each component was a 8-bits integer
float unpack(const vec4 value)
{
   const vec4 bitSh = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
   return(dot(value, bitSh));
}

float rand(vec3 co)
{
    return fract(sin( dot(co.xyz ,vec3(12.9898,78.233,45.5432) )) * 43758.5453);
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

void main()
{
    highp vec4 renderedImagePixel = texture(CalculatedImage,gl_TexCoord[0].st);

    float randomFl = rand(vec3(gl_TexCoord[0].st.x,gl_TexCoord[0].st.y,seed));
    float randomFl1= rand(vec3(gl_TexCoord[0].st.x,gl_TexCoord[0].st.y,randomFl));

    float alpha = randomFl*pi*2.0;
    float x = gl_TexCoord[0].st.x + randomFl/width;
    float y = gl_TexCoord[0].st.y + randomFl1/height;
    vec2 ray = vec2(cos(alpha)*9.0,sin(alpha)*9.0);

    vec3 intersecBuffer = vec3(0.0,0.0,9999.9999);
    float accZBuffer = 0.0;
    int zIndex = -1;
    int prevZIndex = -2;

    vec4 mulColor = vec4(1.0,1.0,1.0,1.0);

    for(int i = 0; i < 100; i++){

        for(int j = 0; j < numObjects; j++){
            if(j != prevZIndex){
                highp float x1,x2,y1,y2;
                x1 = unpack(texelFetch(Objects,ivec2(j,8),0));
                y1 = unpack(texelFetch(Objects,ivec2(j,7),0));
                x2 = unpack(texelFetch(Objects,ivec2(j,6),0));
                y2 = unpack(texelFetch(Objects,ivec2(j,5),0));

                vec3 currBuff = lineSegmentIntersection(vec2(x,y),ray,
                                                        vec2(x1,y1),vec2(x2,y2));

                if( (currBuff.z < intersecBuffer.z) && (currBuff.z > 0.0) ){
                    intersecBuffer = currBuff;
                    zIndex = j;
                }
            }
        }

        accZBuffer += intersecBuffer.z;

        float reflectivity = unpack(texelFetch(Objects,ivec2(zIndex,0),0));

        if( reflectivity > rand(vec3(intersecBuffer.x,intersecBuffer.y,seed)) ){
            highp float x1,x2,y1,y2;
            x1 = unpack(texelFetch(Objects,ivec2(zIndex,8),0));
            y1 = unpack(texelFetch(Objects,ivec2(zIndex,7),0));
            x2 = unpack(texelFetch(Objects,ivec2(zIndex,6),0));
            y2 = unpack(texelFetch(Objects,ivec2(zIndex,5),0));

            mulColor = mulColor * texelFetch(Objects,ivec2(zIndex,3),0);

            ray = reflect(ray,vec2(x1,y1),vec2(x2,y2));

            //alpha = randomFl1*pi*2.0;
            //ray = vec2(cos(alpha)*9.0,sin(alpha)*9.0);


            x = intersecBuffer.x;
            y = intersecBuffer.y;

            intersecBuffer = vec3(0.0,0.0,9999.9999);
            prevZIndex = zIndex;
            zIndex = -1;
        }else{
            i = 666;
        }

    }

    vec4 color = mulColor * texelFetch(Objects,ivec2(zIndex,3),0);

    float phase = unpack(texelFetch(Objects,ivec2(zIndex,2),0));
    float wavelength = unpack(texelFetch(Objects,ivec2(zIndex,1),0));

    float bright = unpack(texelFetch(Objects,ivec2(zIndex,4),0));

    float amplitude = 5.0 * bright *
                      (1.0+sin(accZBuffer*2.0*pi*wavelength*100.0 +
                               phase * 2.0 * pi +
                               period * 2.0 * pi))/2.0;

    //color = vec4(gl_TexCoord[0].st.x,gl_TexCoord[0].st.y,0.0,0.0);

    highp vec4 currTexel = vec4(amplitude*color.r,amplitude*color.g,amplitude*color.b,1.0);
    //if(prevZIndex != -2) currTexel = vec4(1.0,0,0,1.0);
    //else currTexel = vec4(0.0,1.0,1.0,1.0);

    //currTexel = vec4(intersecBuffer.x,intersecBuffer.y,intersecBuffer.z,1.0);

    //if(prevZIndex != -2) currTexel = vec4(1.0,0,0,1.0);
    renderedImagePixel = (renderedImagePixel * numRenderPass + currTexel)/(numRenderPass+1.0);

    //renderedImagePixel = vec4(unpack(texelFetch(Objects,ivec2(x*numObjects,y*9),0)));
    //renderedImagePixel = vec4(intersecBuffer.x,intersecBuffer.y,intersecBuffer.z,1.0);

    gl_FragColor = renderedImagePixel;
}

