#version 330 core

// Input: Received from Vertex Shader
in vec3 v_Color;

// Output: Final pixel color
layout(location = 0) out vec4 o_Color;

void main() {
    o_Color = vec4(v_Color, 1.0);
}