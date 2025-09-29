#include "ShaderClass.h"

std::string get_file_contents(const char* fileName) {
	std::ifstream in(fileName, std::ios::binary);
	if (in) {
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	}
	throw(errno);
}

ShaderProgram::ShaderProgram(const char* vertexFile, const char* fragmentFile) {
	std::string vertexContents = get_file_contents(vertexFile);
	std::string fragmentContetns = get_file_contents(fragmentFile);
	const char* vertexSource = vertexContents.c_str();
	const char* fragmentSource = fragmentContetns.c_str();

	GLint compile;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compile);
	if (!compile) {
		GLchar info[1024];
		glGetShaderInfoLog(vertexShader, 1024, nullptr, info);
		std::cerr << "VERTEX SHADER ERROR: " << info << std::endl;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compile);
	if (!compile) {
		GLchar info[1024];
		glGetShaderInfoLog(fragmentShader, 1024, nullptr, info);
		std::cerr << "FRAGMENT SHADER ERROR: " << info << std::endl;
	}

	this->id = glCreateProgram();
	glAttachShader(this->id, vertexShader);
	glAttachShader(this->id, fragmentShader);
	glLinkProgram(this->id);

	glGetProgramiv(id, GL_LINK_STATUS, &compile);
	if (!compile) {
		GLchar info[1024];
		glGetShaderInfoLog(id, 1024, nullptr, info);
		std::cerr << "SHADER PROGRAM ERROR: " << info << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void ShaderProgram::activate_shader() const {
	glUseProgram(this->id);
}

void ShaderProgram::delete_shader() const {
	glDeleteProgram(this->id);
}

void ShaderProgram::set_Uniform_Int(const char* uniform_name, const int value) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniform1i(uniformID, value);
}

void ShaderProgram::set_uniform_float(const char* uniform_name, const float value) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniform1f(uniformID, value);
}

void ShaderProgram::set_uniform_float_array(const char* uniform_name, const float* f_array, const int size) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniform1fv(uniformID, size, f_array);
}

void ShaderProgram::set_Uniform_Mat4(const char* uniform_name, const glm::mat4& glm_matrix) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniformMatrix4fv(uniformID, 1, GL_FALSE, glm::value_ptr(glm_matrix));
}

void ShaderProgram::set_Uniform_Vec2(const char* uniform_name, const glm::vec2 glm_vec2) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniform2fv(uniformID, 1, glm::value_ptr(glm_vec2));
}

void ShaderProgram::set_Uniform_iVec2(const char* uniform_name, const glm::ivec2 glm_vec2) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniform2iv(uniformID, 1, glm::value_ptr(glm_vec2));
}

void ShaderProgram::set_Uniform_Vec3(const char* uniform_name, const glm::vec3 glm_vec3) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniform3fv(uniformID, 1, glm::value_ptr(glm_vec3));
}

void ShaderProgram::set_Uniform_Vec4(const char* uniform_name, const glm::vec4 glm_vec4) const {
	GLuint uniformID = glGetUniformLocation(this->id, uniform_name);
	glUniform4fv(uniformID, 1, glm::value_ptr(glm_vec4));
}

void ShaderProgram::set_ubo(const char* uniform_block_name, const UBO& ubo, const GLuint index) const {
	GLuint blockIndex = glGetUniformBlockIndex(this->id, uniform_block_name);
	glUniformBlockBinding(this->id, blockIndex, index);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, ubo.id);
}

void ShaderProgram::set_ssbo(const char* uniform_block_name, const SSBO& ssbo, const GLuint index) const {
	GLuint blockIndex = glGetProgramResourceIndex(this->id, GL_SHADER_STORAGE_BLOCK, uniform_block_name);
	glShaderStorageBlockBinding(this->id, blockIndex, index);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo.id);
}
