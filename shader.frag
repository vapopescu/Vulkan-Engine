#version 450

layout(location = 0) in vec2 v_TexCoord;

layout(set = 1, binding = 0) uniform sampler2D qt_Texture;

layout(location = 0) out vec4 f_FragColor;

void main()
{
    f_FragColor = vec4(1,1,0,1);//texture(qt_Texture, v_TexCoord);
}
