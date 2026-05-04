#version 140

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uFont;
uniform vec4      uColor;

void main() {
    float a = texture(uFont, vUV).r;
    FragColor = vec4(uColor.rgb, uColor.a * a);
}
