#include "VBO.h"

VBO::VBO() {
	glGenBuffers(1, &this->id);
}

VBO::VBO(GLfloat* vertices, GLsizeiptr size) {
	glGenBuffers(1, &this->id);
	glBindBuffer(GL_ARRAY_BUFFER, this->id);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VBO::set_data(GLfloat* vertices, GLsizeiptr size, GLenum usage) {
	glBindBuffer(GL_ARRAY_BUFFER, this->id);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, usage);
}

void VBO::bind_VBO() {
	glBindBuffer(GL_ARRAY_BUFFER, this->id);
}

void VBO::unbind_VBO() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::delete_VBO() {
	glDeleteBuffers(1, &this->id);
}
