#version 450

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform TilesetMVP  {
    mat4 mvp;
} tilesetMVP;

vec2 positions[6] = vec2[](
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, -1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(1, 1)
);

vec2 texCoords[6] = vec2[](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
);

void main() {
    gl_Position = tilesetMVP.mvp * vec4(positions[gl_VertexIndex], 0, 1.0);
    fragColor = vec3(1.0, 0, 0);
    fragTexCoord = texCoords[gl_VertexIndex];
}
