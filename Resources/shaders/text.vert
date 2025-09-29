#version 460 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec4 textColor;

out vec2 texCoords;
out vec4 color;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * vec4(position, 0.0, 1.0);
	texCoords = tex;
	color = textColor;
}