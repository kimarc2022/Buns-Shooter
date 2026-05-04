#version 140
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec2 aPos;   // [0..1] unit quad

uniform vec2 uPos;    // top-left corner in NDC
uniform vec2 uSize;   // width/height in NDC

void main() {
    // aPos.y inverted so (0,0) is top-left
    vec2 ndc = uPos + vec2(aPos.x, -aPos.y) * uSize;
    gl_Position = vec4(ndc, 0.0, 1.0);
}
