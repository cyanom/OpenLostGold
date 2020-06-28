#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform BoxData {
    mat4 mvp;
    vec4 color;
} boxData;

layout(location = 0) out vec4 fragColor;

vec2 positions[6] = vec2[](
    vec2(-0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, 0.5),
    vec2(0.5, 0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, -0.5)
);

void main() {
    gl_Position = boxData.mvp * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = boxData.color;
}
