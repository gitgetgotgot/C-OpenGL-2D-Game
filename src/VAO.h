#pragma once

#include <glad/glad.h>
#include "VBO.h"

class VAO {
public:
	GLuint id;
	VAO();
	void link_Attribute(VBO& vbo, GLuint layout, GLuint numOfComps, GLenum type, GLsizeiptr stride, void* offset);
	void link_Attribute_with_Divisor(VBO& vbo, GLuint layout, GLuint numOfComps, GLenum type, GLsizeiptr stride, void* offset, GLuint divisor);
	void bind_VAO();
	void unbind_VAO();
	void delete_VAO();
};