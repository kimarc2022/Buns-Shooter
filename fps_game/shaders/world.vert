#version 140
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vNormal;
out vec3 vWorld;

void main() {
    vec4 wp = uModel * vec4(aPos, 1.0);
    vWorld  = wp.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uProj * uView * wp;
}
