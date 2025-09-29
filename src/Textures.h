#pragma once
#include <glad/glad.h>
#include <stb_image.h>

class Texture {
	GLuint* textures;
	int current_index = 0, size;
public:
	Texture(int size);
	~Texture();
	GLuint get_texture_index(int id);
	void load_2D_texture(const char* fileName, bool isPixelised);
	void load_text_bitmap(const char* filename, bool isPixelised, unsigned char** image_bytes, int& numOfChannels);
	void bind(const int id);
};