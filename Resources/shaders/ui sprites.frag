#version 460 core

out vec4 fragColor;

in vec2 texCoord;
in float texIndex;

uniform sampler2D u_textures[2];

void main()
{
    int index = int(texIndex);
    if(texture(u_textures[index], texCoord).a < 0.1) discard;
    fragColor = texture(u_textures[index], texCoord);
}