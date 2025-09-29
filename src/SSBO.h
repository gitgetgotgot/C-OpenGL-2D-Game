#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class SSBO {
public:
	GLuint id;
	SSBO();
	void set_data(GLfloat* data, GLsizeiptr size, GLenum usage);
	void set_persistent_storage_data(GLfloat* data, GLsizeiptr size);
	void bind_SSBO();
	void unbind_SSBO();
	void delete_SSBO();
};