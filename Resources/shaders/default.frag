#version 460 core
out vec4 Fragcolor;

in vec3 color; //input vec3 color from vertex shader for custom color

in vec2 texCoord;
uniform sampler2D tex0; //tells which texture to use (there is a limit on amount that can be used at once)

void main()
{
	//Fragcolor = vec4(color, 1.0);
	Fragcolor = texture(tex0, texCoord);
}