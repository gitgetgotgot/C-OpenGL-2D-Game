#include "SSBO.h"

SSBO::SSBO() {
	glGenBuffers(1, &this->id);
}

void SSBO::set_data(GLfloat* data, GLsizeiptr size, GLenum usage) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
}

void SSBO::set_persistent_storage_data(GLfloat* data, GLsizeiptr size) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->id);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
}

void SSBO::bind_SSBO() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->id);
}

void SSBO::unbind_SSBO() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::delete_SSBO() {
	glDeleteBuffers(1, &this->id);
}

