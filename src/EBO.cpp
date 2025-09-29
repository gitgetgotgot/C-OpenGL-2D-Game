#include "EBO.h"

EBO::EBO() {
	glGenBuffers(1, &this->id);
}

EBO::EBO(GLuint* indices, GLsizeiptr size) {
	glGenBuffers(1, &this->id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}

void EBO::set_data(GLuint* indices, GLsizeiptr size, GLenum usage) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, usage);
}

void EBO::bind_EBO() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

void EBO::unbind_EBO() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

void EBO::delete_EBO() {
	glDeleteBuffers(1, &this->id);
}
