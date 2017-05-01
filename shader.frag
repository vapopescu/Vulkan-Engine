#version 450

layout(location = 0) in vec2 in_TexCoord;

layout(binding = 10) uniform sampler2D u_ColorTexture;

layout(location = 0) out vec4 out_FragColor;

void main()
{
    out_FragColor = texture(u_ColorTexture, in_TexCoord);
}
