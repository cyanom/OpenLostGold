#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform DirectVP {
    mat4 vp;
} directVP;

layout(binding = 1) uniform SpriteBoxData {
    mat4 m;
    vec3 tint;
    vec2 uvPos;
    vec2 uvSize;
    int textureId;
} spriteBoxData;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, -1.0)
);

vec2 uvPositions[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0)
);

void main() {
    gl_Position = spriteBoxData.m * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = vec4(spriteBoxData.tint, 1.0);
    fragTexCoord = uvPositions[gl_VertexIndex] * spriteBoxData.uvSize + spriteBoxData.uvPos;
}
