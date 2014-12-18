#version 400

//RT2DShader

uniform sampler2D CalculatedImage;
uniform int numRenderPass;

uniform sampler2D Objects;
uniform int numObjects;

uniform int width;
uniform int height;

const float pi = 3.14159265359;

uniform float seed = pi;

const float x = 1.0/255.0;
const float y = 1.0/65025.0;
const float z = 1.0/16581375.0;

vec4 pack( float v ) {
  vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
  enc = fract(enc);
  enc -= enc.yzww * vec4(x,x,x,0.0);
  return enc;
}

float unpack( vec4 rgba ) {
  return dot( rgba, vec4(1.0, x, y, z) );
}

//random [0..1]
highp float rand(vec2 co,float seed)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

vec3 get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
                           float p2_x, float p2_y, float p3_x, float p3_y)
{
    float noCollision = -1.0;

    float s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
    s10_x = p1_x - p0_x;
    s10_y = p1_y - p0_y;
    s32_x = p3_x - p2_x;
    s32_y = p3_y - p2_y;

    denom = s10_x * s32_y - s32_x * s10_y;
    if (denom == 0)
        return vec3(0,0,noCollision); // Collinear
    bool denomPositive = denom > 0;

    s02_x = p0_x - p2_x;
    s02_y = p0_y - p2_y;
    s_numer = s10_x * s02_y - s10_y * s02_x;
    if ((s_numer < 0) == denomPositive)
        return vec3(0,0,noCollision); // No collision

    t_numer = s32_x * s02_y - s32_y * s02_x;
    if ((t_numer < 0) == denomPositive)
        return vec3(0,0,noCollision); // No collision

    if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
        return vec3(0,0,noCollision); // No collision
    // Collision detected
    t = t_numer / denom;

    float i_x = p0_x + (t * s10_x);
    float i_y = p0_y + (t * s10_y);
    float dist = sqrt((i_x-p2_x)*(i_x-p2_x)+(i_y-p2_y)*(i_y-p2_y));

    return vec3(i_x,i_y,dist);
}

void main()
{
    vec4 renderedImagePixel = texture(CalculatedImage,gl_TexCoord[0].st);

    float alpha = rand(gl_TexCoord[0].st,seed)*2.0*pi;
    float x = 1-gl_TexCoord[0].st.x;
    float y = gl_TexCoord[0].st.y;

    vec3 intersecBuffer = vec3(0.0,0.0,99999.9999);
    int zIndex = -1;

    vec4 currColor = vec4(0,0,0,0);

    for(int i = 0; i < numObjects; i++){
        highp float x1,x2,y1,y2;
        x1 = unpack(texelFetch(Objects,ivec2(i,5),0));
        y1 = unpack(texelFetch(Objects,ivec2(i,4),0));
        x2 = unpack(texelFetch(Objects,ivec2(i,3),0));
        y2 = unpack(texelFetch(Objects,ivec2(i,2),0));

        vec3 currBuff = get_line_intersection(x,y,(x+cos(alpha)*9.0),(y+sin(alpha)*9.0),
                                              x1,y1,x2,y2);

        if( (currBuff.z < intersecBuffer.z) && (currBuff.z > 0.0)){
            intersecBuffer = currBuff;
            zIndex = i;
        }
    }

    float bright = unpack(texelFetch(Objects,ivec2(zIndex,1),0));
    vec4 color = texelFetch(Objects,ivec2(zIndex,0),0);

    //color = vec4(gl_TexCoord[0].st.x,gl_TexCoord[0].st.y,0.0,0.0);

    vec4 currTexel = vec4(bright*color.r,bright*color.g,bright*color.b,color.a);

    //renderedImagePixel = (renderedImagePixel * numRenderPass + currTexel)/(numRenderPass+1);

    renderedImagePixel = vec4(unpack(texelFetch(Objects,ivec2(x*5,y*6),0)),
                              unpack(texelFetch(Objects,ivec2(x*5,y*6),0)),
                              unpack(texelFetch(Objects,ivec2(x*5,y*6),0)),1.0);


    gl_FragColor = currTexel;
}

