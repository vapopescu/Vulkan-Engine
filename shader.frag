#version 450

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 0) uniform sampler2D qt_Texture;

layout(location = 0) out vec4 f_FragColor;

void main()
{
    f_FragColor = texture(qt_Texture, v_TexCoord);
}
