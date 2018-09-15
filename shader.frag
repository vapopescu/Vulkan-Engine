#version 450

layout(binding = 5) uniform Uniforms
{
    float gamma;
} u;

layout(binding = 10) uniform sampler2D diffuseMap;
layout(binding = 11) uniform sampler2D normalMap;
layout(binding = 12) uniform sampler2D specularMap;

layout(location = 0) in vec2 in_TexCoord;

layout(location = 0) out vec4 out_FragColor;

void main()
{
    vec4 gammaPow = vec4(u.gamma, u.gamma, u.gamma, 1.0f);

    vec4 color = texture(diffuseMap, in_TexCoord);
    //vec4 color = vec4(in_TexCoord, 0, 1);

    color = vec4(color.xyz, 1.0f);

    out_FragColor = pow(color, 1.0f / gammaPow);
    //out_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
