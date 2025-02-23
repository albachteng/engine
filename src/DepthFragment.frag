#version 330 core
in vec4 ViewPos;
in vec3 currColor;

out vec4 FragColor;

float near = 0.1;
float far = 15;

void main() {
    float depth = (-ViewPos.z - near) / (far - near);
    FragColor = vec4(vec3(depth) * currColor, 1.0);
}
