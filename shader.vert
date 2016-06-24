#version 450

layout(location = 0) in vec4 qt_Position;
layout(location = 1) in vec2 qt_TexCoord;

layout(push_constant) uniform Uniforms
{
    mat4 mvpMatrix;
} qt;

layout(location = 0) out vec2 v_TexCoord;

void main()
{
    gl_Position = /*qt.mvpMatrix * */qt_Position;
    v_TexCoord = qt_TexCoord;
}
