#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;

//output color to fragment shader
out vec3 color;
//output texCoords to fragment shader
out vec2 texCoord;

//declaration of uniform variable that can be used from the code
//if it's not used here anywhere, then it will be deleted here
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

void main()
{
	//only one type! (int, float, etc.)
	gl_Position = projectionMatrix * modelMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	color = aColor;
	texCoord = aTex;
}