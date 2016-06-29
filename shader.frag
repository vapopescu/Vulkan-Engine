#version 450

layout(location = 0) in vec2 in_TexCoord;

layout(set = 1, binding = 0) uniform sampler2D u_ColorTexture;

layout(location = 0) out vec4 out_FragColor;

void main()
{
    out_FragColor = vec4(in_TexCoord,1,1);//texture(u_ColorTexture, in_TexCoord);
}
