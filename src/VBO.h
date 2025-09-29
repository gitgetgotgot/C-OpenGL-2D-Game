#pragma once

#include <glad/glad.h>

class VBO {
public:
	GLuint id;
	VBO();
	VBO(GLfloat* vertices, GLsizeiptr size);
	void set_data(GLfloat* vertices, GLsizeiptr size, GLenum usage);
	void bind_VBO();
	void unbind_VBO();
	void delete_VBO();
};