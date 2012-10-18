uniform sampler2D waveTextureId;
uniform sampler2D waveTextureIdRef;

uniform float waveTime;
varying vec3 vTexCoord;


void main()
{
    
    vec4 color1 = texture2D(waveTextureId, vec2(gl_TexCoord[0]));
    vec4 color2 = texture2D(waveTextureIdRef, vec2(vTexCoord));
    
    
    gl_FragColor = 0.6 * vec4(color1 + color2) * vec4(0.0, 1.0, 1.0, 0.50); 
}