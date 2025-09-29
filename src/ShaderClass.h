#pragma once

#include "UBO.h"
#include "SSBO.h"

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <string>
#include <unordered_map>

std::string get_file_contents(const char*);

class ShaderProgram {
public:
	GLuint id;
	ShaderProgram(const char* vertexFile, const char* fragmentFile);
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram() = delete;
	void activate_shader() const;
	void delete_shader() const;
	void set_Uniform_Int(const char* uniform_name, const int value) const;
	void set_uniform_float(const char* uniform_name, const float value) const;
	void set_uniform_float_array(const char* uniform_name, const float* f_array, const int size) const;
	void set_Uniform_Mat4(const char* uniform_name, const glm::mat4& glm_matrix) const;
	void set_Uniform_Vec2(const char* uniform_name, const glm::vec2 glm_vec2) const;
	void set_Uniform_iVec2(const char* uniform_name, const glm::ivec2 glm_vec2) const;
	void set_Uniform_Vec3(const char* uniform_name, const glm::vec3 glm_vec3) const;
	void set_Uniform_Vec4(const char* uniform_name, const glm::vec4 glm_vec4) const;
	void set_ubo(const char* uniform_block_name, const UBO& ubo, const GLuint index) const;
	void set_ssbo(const char* uniform_block_name, const SSBO& ssbo, const GLuint index) const;
};