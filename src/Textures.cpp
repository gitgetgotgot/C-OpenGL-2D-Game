#include "Textures.h"
Texture::Texture(int size) {
	this->textures = new GLuint[size];
	this->size = size;
}

Texture::~Texture() {
	for (int i = 0; i < this->current_index; i++) {
		glDeleteTextures(1, &this->textures[i]);
	}
	delete[]textures;
}

GLuint Texture::get_texture_index(int id) {
	return this->textures[id];
}

void Texture::load_2D_texture(const char* fileName, bool isPixelised) {
	if (current_index == size) return;

	int imgWidth, imgHeight, numOfColorChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* image_bytes = stbi_load(fileName, &imgWidth, &imgHeight, &numOfColorChannels, 0);

	glGenTextures(1, &textures[current_index]);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[current_index]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, isPixelised ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, isPixelised ? GL_NEAREST : GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, numOfColorChannels < 4 ? GL_RGB : GL_RGBA, imgWidth, imgHeight, 0, numOfColorChannels < 4 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, image_bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(image_bytes);
	glBindTexture(GL_TEXTURE_2D, 0);
	current_index++;
}

void Texture::load_text_bitmap(const char* filename, bool isPixelised, unsigned char** image_bytes, int& numOfChannels) {
	if (current_index == size) return;

	int imgWidth, imgHeight;
	stbi_set_flip_vertically_on_load(false);
	*image_bytes = stbi_load(filename, &imgWidth, &imgHeight, &numOfChannels, 0);

	glGenTextures(1, &textures[current_index]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[current_index]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, isPixelised ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, isPixelised ? GL_NEAREST : GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, numOfChannels < 4 ? GL_RGB : GL_RGBA, imgWidth, imgHeight, 0, numOfChannels < 4 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, *image_bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	current_index++;
}

void Texture::bind(const int id) {
	glBindTexture(GL_TEXTURE_2D, textures[id]);
}
