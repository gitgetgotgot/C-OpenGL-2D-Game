#version 460 core
out vec4 fragColor;

in vec2 texCoords;
in vec4 color;

uniform sampler2D tex0;

void main()
{
	if(texture(tex0, texCoords).a < 0.1) discard;
	fragColor = color * texture(tex0, texCoords);
}