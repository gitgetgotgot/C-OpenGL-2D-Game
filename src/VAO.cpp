#include "VAO.h"

VAO::VAO() {
	glGenVertexArrays(1, &this->id);
}

void VAO::link_Attribute(VBO& vbo, GLuint layout, GLuint numOfComps, GLenum type, GLsizeiptr stride, void* offset) {
	vbo.bind_VBO();
	glVertexAttribPointer(layout, numOfComps, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	vbo.unbind_VBO();
}

void VAO::link_Attribute_with_Divisor(VBO& vbo, GLuint layout, GLuint numOfComps, GLenum type, GLsizeiptr stride, void* offset, GLuint divisor) {
	vbo.bind_VBO();
	glVertexAttribPointer(layout, numOfComps, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	glVertexAttribDivisor(layout, divisor);
	vbo.unbind_VBO();
}

void VAO::bind_VAO() {
	glBindVertexArray(this->id);
}

void VAO::unbind_VAO() {
	glBindVertexArray(0);
}

void VAO::delete_VAO() {
	glDeleteVertexArrays(1, &this->id);
}
