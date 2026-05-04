#version 140

in vec3 vNormal;
in vec3 vWorld;
out vec4 FragColor;

uniform vec3  uColor;
uniform vec3  uLightDir;
uniform vec3  uViewPos;
uniform float uAlpha;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    float diff = max(dot(N, L), 0.0);

    vec3 V = normalize(uViewPos - vWorld);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 16.0) * 0.2;

    vec3 ambient = 0.30 * uColor;
    vec3 diffuse = 0.85 * diff * uColor;
    vec3 color = ambient + diffuse + vec3(spec);

    float d = length(uViewPos - vWorld);
    float fog = clamp((d - 60.0) / 200.0, 0.0, 1.0);
    vec3 sky = vec3(0.55, 0.75, 0.95);
    color = mix(color, sky, fog);

    FragColor = vec4(color, uAlpha);
}
