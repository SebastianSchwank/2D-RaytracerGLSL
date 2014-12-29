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

vec4 texelFetch(sampler2D smplr,vec2 coords,vec2 size){
    return texture2D(smplr,vec2( floor(2.0*coords.x - 1.0)/(2.0*size.x),floor(2.0*coords.y - 1.0)/(2.0*size.y)) );
}

void main()
{
    vec4 renderedImagePixel = texture2D(CalculatedImage,v_texcoord);

    if(fbo){

        float randomFl0 = unpack(texture2D(random,vec2((v_texcoord.x+seedX)/2.0,(v_texcoord.y+seedY))/2.0));

        vec4 currTexel = vec4(randomFl0,randomFl0,randomFl0,1.0);
        //if(prevZIndex != -2) currTexel = vec4(1.0,0,0,1.0);
        renderedImagePixel = (renderedImagePixel * float(numRenderPass) + currTexel)/float(numRenderPass+1);

        //renderedImagePixel = vec4(unpack(texelFetch(Objects,vec2(x*float(width),y*float(height)),vec2(float(numObjects),9.0))));
        //renderedImagePixel = vec4(intersecBuffer.x,intersecBuffer.y,intersecBuffer.z,1.0);
    }

    gl_FragColor = renderedImagePixel;

}

