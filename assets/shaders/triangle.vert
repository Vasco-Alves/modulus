#version 330 core

// Input: Layout 0 is Position, Layout 1 is Color
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

// Output: We send the color to the fragment shader
out vec3 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = vec4(a_Position, 1.0);
}