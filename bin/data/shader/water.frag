#version 110


//uniform sampler2D tex;
//void main(void)
//{
//    gl_FragColor= gl_Color(tex.rgb, tex.rgb*tex.a);
//}

uniform sampler2D tex;
varying vec2 uv;

void main() {
gl_FragColor = gl_Color;
}