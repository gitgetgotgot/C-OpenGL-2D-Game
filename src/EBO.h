#pragma once

#include <glad/glad.h>

class EBO {
public:
	GLuint id;
	EBO();
	EBO(GLuint* indices, GLsizeiptr size);
	void set_data(GLuint* indices, GLsizeiptr size, GLenum usage);
	void bind_EBO();
	void unbind_EBO();
	void delete_EBO();
};