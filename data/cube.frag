#version 330 core
in vec3 vNormal;
uniform vec3 uColor;
uniform bool uSelected;
uniform bool uWireframe;
out vec4 FragColor;
void main() {
    if (uWireframe) {
        FragColor = vec4(uColor, 1.0);
        return;
    }
    vec3 n = normalize(vNormal);
    float diff = max(dot(n, normalize(vec3(0.5, 1.0, 0.8))), 0.0);
    vec3 color = uColor * (0.3 + 0.7 * diff);
    if (uSelected)
        color = mix(color, vec3(1.0, 0.8, 0.1), 0.5);
    FragColor = vec4(color, 1.0);
}
