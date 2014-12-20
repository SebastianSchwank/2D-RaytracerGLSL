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
    s1 = r1;
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

void main()
{
    vec4 renderedImagePixel = texture(CalculatedImage,gl_TexCoord[0].st);

    float randomFl = rand(vec3(gl_TexCoord[0].st.x,gl_TexCoord[0].st.y,seed));

    float alpha = randomFl*pi*2.0;
    float x = gl_TexCoord[0].st.x;
    float y = gl_TexCoord[0].st.y;

    vec3 intersecBuffer = vec3(0.0,0.0,9999.9999);
    float accZBuffer = 0.0;
    int zIndex = -1;

    for(int j = 0; j < numObjects; j++){
        highp float x1,x2,y1,y2;
        x1 = unpack(texelFetch(Objects,ivec2(j,7),0));
        y1 = unpack(texelFetch(Objects,ivec2(j,6),0));
        x2 = unpack(texelFetch(Objects,ivec2(j,5),0));
        y2 = unpack(texelFetch(Objects,ivec2(j,4),0));

        vec3 currBuff = lineSegmentIntersection(vec2(x,y),vec2(cos(alpha),sin(alpha)),
                                                vec2(x1,y1),vec2(x2,y2));

        if( (currBuff.z < intersecBuffer.z) && (currBuff.z >= 0.0)){
            intersecBuffer = currBuff;
            zIndex = j;
        }
    }

    accZBuffer += intersecBuffer.z;

    vec4 color = texelFetch(Objects,ivec2(zIndex,2),0);

    float phase = unpack(texelFetch(Objects,ivec2(zIndex,1),0));
    float wavelength = unpack(texelFetch(Objects,ivec2(zIndex,0),0));

    float bright = unpack(texelFetch(Objects,ivec2(zIndex,3),0))*5.0;

    float amplitude = bright * (1.0+sin(accZBuffer*2*pi*1.0/wavelength+period*2*pi+phase*2*pi));

    //color = vec4(gl_TexCoord[0].st.x,gl_TexCoord[0].st.y,0.0,0.0);

    vec4 currTexel = vec4(amplitude*color.r,amplitude*color.g,amplitude*color.b,1.0);
    //if(zIndex == false) currTexel = vec4(1.0,0,0,1.0);
    //else currTexel = vec4(0.0,1.0,1.0,1.0);

    renderedImagePixel = (renderedImagePixel * numRenderPass + currTexel)/(numRenderPass+1);

    //renderedImagePixel = texelFetch(Objects,ivec2(x*14,y*6),0);

    gl_FragColor = renderedImagePixel;
}

