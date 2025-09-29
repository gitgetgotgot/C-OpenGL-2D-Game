#version 460 core
out vec4 fragColor;

in vec2 texCoord;
in float texIndex;
in float lightLevel;
in float opacity;

uniform sampler2D u_textures[2];

void main()
{
    int index = int(texIndex);
    if(texture(u_textures[index], texCoord).a < 0.1) discard;

    vec4 texColor = texture(u_textures[index], texCoord);

    //default texture color based on ambient lighting
    fragColor = vec4(lightLevel * texColor.rgb, texColor.a * opacity);
}