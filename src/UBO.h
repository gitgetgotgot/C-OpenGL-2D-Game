#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class UBO {
public:
	GLuint id;
	UBO();
	void set_matrices_data(glm::mat4* matrices, GLsizeiptr size, GLenum usage);
	void bind_UBO();
	void unbind_UBO();
	void delete_UBO();
};