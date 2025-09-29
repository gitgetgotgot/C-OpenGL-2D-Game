#version 460 core
out vec4 fragColor;

in vec4 color;

void main()
{
	fragColor = color;
}