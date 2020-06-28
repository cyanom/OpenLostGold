#version 450
#extension GL_ARB_separate_shader_objects : enable

vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, -1.0)
);

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inSize;
layout (location = 1) in vec4 inColor;

layout(binding = 0) uniform VP {
    mat4 vp;
} VP;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
