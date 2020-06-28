#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(binding = 0) uniform DirectVP  {
    mat4 vp;
} directVP;

layout(location = 0) out vec4 fragColor;

void main() {
    
    gl_Position = directVP.vp * vec4(inPosition, 0, 1);
    fragColor = inColor;
}
