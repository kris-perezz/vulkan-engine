#version 450

layout (location = 1) in vec3 fragColour;
layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(fragColour, 1.0);
}