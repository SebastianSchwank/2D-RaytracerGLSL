#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

//RT2DShader

varying vec2 v_texcoord;

uniform sampler2D CalculatedImage;
uniform int numRenderPass;

uniform sampler2D Objects;
uniform int numObjects;

uniform int width;
uniform int height;

uniform float period;

const float pi = 3.14159265359;

uniform float seed;

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

void main()
{
    vec4 renderedImagePixel = texture2D(CalculatedImage,v_texcoord);

    float rand = rand(vec3(v_texcoord.x,v_texcoord.st.y,seed));

    vec4 currTexel = vec4(rand,rand,rand,1.0);
    //if(prevZIndex != -2) currTexel = vec4(1.0,0,0,1.0);
    renderedImagePixel = (renderedImagePixel * float(numRenderPass) + currTexel)/float(numRenderPass+1);

    //renderedImagePixel = vec4(unpack(texelFetch(Objects,ivec2(x*numObjects,y*9),0)));
    //renderedImagePixel = vec4(intersecBuffer.x,intersecBuffer.y,intersecBuffer.z,1.0);

    gl_FragColor = renderedImagePixel;
}

