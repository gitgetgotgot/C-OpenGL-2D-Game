#version 460 core
layout (location = 0) in vec2 aPos;

struct Data {
	mat4 model;
	vec2 texCoords[4];
	float tex_id;
};

layout(std430) buffer SSBO {
    Data objects[];
};

out vec2 texCoord;
out float texIndex;
out vec2 globalCoord;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
	mat4 modelMatrix = objects[gl_InstanceID].model;

	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 0.0, 1.0);

	texCoord = objects[gl_InstanceID].texCoords[gl_VertexID % 4];
	texIndex = objects[gl_InstanceID].tex_id;

	globalCoord = (modelMatrix * vec4(aPos, 0.0, 1.0)).xy;
}