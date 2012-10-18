    // Based on a Gray Scott Frag shader from rdex-fluxus    
    // https://code.google.com/p/rdex-fluxus/source/browse/trunk/reactiondiffusion.frag    


#version 120    
#define KERNEL_SIZE 9    

float kernel[KERNEL_SIZE];    
uniform float width;    

vec2 offset[KERNEL_SIZE];    

uniform sampler2D srcTexture;    
uniform sampler2D texture; // U := r, V := g, other channels ignored    
uniform float ru;          // rate of diffusion of U    
uniform float rv;          // rate of diffusion of V    
uniform float f;           // some coupling parameter    
uniform float k;           // another coupling parameter    

void main(void)    
{    
    vec2 texCoord   = gl_TexCoord[0].st;        // center coordinates    
    float w         = 1.0/width;    
    float h         = 1.0/width;    
    float w2        = w*2.0;    
    float h2        = h*2.0;    
    
    kernel[0] = 0.707106781;    
    kernel[1] = 1.0;    
    kernel[2] = 0.707106781;    
    kernel[3] = 1.0;    
    kernel[4] =-6.82842712;    
    kernel[5] = 1.0;    
    kernel[6] = 0.707106781;    
    kernel[7] = 1.0;    
    kernel[8] = 0.707106781;    
    
    offset[0] = vec2( -w, -h);    
    offset[1] = vec2(0.0, -h);    
    offset[2] = vec2(  w, -h);    
    
    offset[3] = vec2( -w, 0.0);    
    offset[4] = vec2(0.0, 0.0);    
    offset[5] = vec2(  w, 0.0);    
    
    offset[6] = vec2( -w, h);    
    offset[7] = vec2(0.0, h);    
    offset[8] = vec2(  w, h);    
    
    
    vec2 texColor       = texture2D( texture, texCoord ).rb;    
    float srcTexColor   = texture2D( srcTexture, texCoord ).r;    
    
    vec2 sum            = vec2( 0.0, 0.0 );    
    
    for( int i=0; i<KERNEL_SIZE; i++ ){    
        vec2 tmp    = texture2D( texture, texCoord + offset[i] ).rb;    
        sum         += tmp * kernel[i];    
    }    
    
    
    float F     = f + srcTexColor * 0.025 - 0.0005;    
    float K     = k + srcTexColor * 0.025 - 0.0005;    
    
    float u     = texColor.r;    
    float v     = texColor.g;    
    float uvv   = u * v * v;    
        //============================================================================    
    float du    = ru * sum.r - uvv + F * (1.0 - u);     // Gray-Scott equation    
    float dv    = rv * sum.g + uvv - (F + K) * v;       // diffusion+-reaction    
                                                        //============================================================================    
    u += du*0.6;    
    v += dv*0.6;    
    gl_FragColor = vec4( clamp( u, 0.0, 1.0 ), 1.0 - u/v, clamp( v, 0.0, 1.0 ), 1.0 );    
}       