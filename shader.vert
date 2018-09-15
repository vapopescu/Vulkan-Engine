#version 450

layout(set = 0, binding = 0) uniform Uniforms
{
    mat4 matrix;
} mvp;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normals;

layout(location = 0) out vec2 out_TexCoord;

void main()
{
    gl_Position = mvp.matrix * vec4(in_Position, 1.0f);
    out_TexCoord = in_TexCoord;
}
