#include "UBO.h"

UBO::UBO() {
	glGenBuffers(1, &this->id);
}

void UBO::set_matrices_data(glm::mat4* matrices, GLsizeiptr size, GLenum usage) {
	glBindBuffer(GL_UNIFORM_BUFFER, this->id);
	glBufferData(GL_UNIFORM_BUFFER, size, matrices, usage);
}

void UBO::bind_UBO() {
	glBindBuffer(GL_UNIFORM_BUFFER, this->id);
}

void UBO::unbind_UBO() {
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBO::delete_UBO() {
	glDeleteBuffers(1, &this->id);
}
