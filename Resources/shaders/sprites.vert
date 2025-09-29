#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in float aTexIndex;

out vec2 texCoord;
out float texIndex;
out vec2 globalCoord;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 0.0, 1.0);
	texCoord = aTex;
	texIndex = aTexIndex;
	globalCoord = aPos;
}