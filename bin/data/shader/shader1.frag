uniform sampler2D tex;

void main(void) {
    gl_FragColor = texture2D(tex, TexCoord[0]);
}

