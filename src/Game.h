#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include "AudioManager.h"
#include "ShaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "UBO.h"
#include "Textures.h"
#include "GameObjects.h"
#include "GameEntities.h"

class Text {
public:
	Text() {
		text_SP = new ShaderProgram("Resources/shaders/text.vert", "Resources/shaders/text.frag");
		text_SP->activate_shader();
		text_SP->set_Uniform_Int("tex0", 0);
		text_vao = new VAO();
		text_vao->bind_VAO();
		text_vbo = new VBO();
		text_vbo->set_data(nullptr, MAX_CHARACTERS_PER_DRAW * 4 * sizeof(TextData), GL_DYNAMIC_DRAW);
		text_vao->link_Attribute(*text_vbo, 0, 2, GL_FLOAT, 8 * sizeof(float), (void*)0);
		text_vao->link_Attribute(*text_vbo, 1, 2, GL_FLOAT, 8 * sizeof(float), (void*)(2 * sizeof(float)));
		text_vao->link_Attribute(*text_vbo, 2, 4, GL_FLOAT, 8 * sizeof(float), (void*)(4 * sizeof(float)));

		text_ebo = new EBO();
		text_index_buffer = new GLuint[MAX_CHARACTERS_PER_DRAW * 6];
		for (int i = 0; i < MAX_CHARACTERS_PER_DRAW; i++) {
			text_index_buffer[i * 6] = text_index_buffer[i * 6 + 3] = i * 4;
			text_index_buffer[i * 6 + 2] = text_index_buffer[i * 6 + 4] = text_index_buffer[i * 6] + 2;
			text_index_buffer[i * 6 + 1] = text_index_buffer[i * 6 + 2] - 1;
			text_index_buffer[i * 6 + 5] = text_index_buffer[i * 6 + 2] + 1;
		}
		text_ebo->set_data(text_index_buffer, MAX_CHARACTERS_PER_DRAW * 24, GL_STATIC_DRAW);

		setup_tex_and_tex_coords();
	}
	~Text() {
		text_vao->delete_VAO();
		text_vbo->delete_VBO();
		text_ebo->delete_EBO();
		text_SP->delete_shader();
		delete[]text_index_buffer;
	}
	void setup_tex_and_tex_coords() {
		unsigned char* image_bytes;
		int numOfChannels;
		int checkByte = 3;  //check alpha byte
		//texture.load_text_bitmap("Verdana.bmp", true, &image_bytes, numOfChannels); //1024x1024 bitmap
		texture.load_text_bitmap("Resources/textures/Verdana_alpha.png", true, &image_bytes, numOfChannels); //1024x1024 bitmap

		int pixelsPerChar = 1024 / 16; //16 by 16 characters on bitmap
		int Xstart, Ystart;
		int Xpixel, Ypixel;
		int xi;
		unsigned char alpha;
		for (int i = 0; i < 95; i++) { //from 0 to 94 (int(char) - 32) [from ASCII 32 to 127] english only
			Ystart = i / 16;
			Xstart = i % 16;
			Xpixel = Xstart * pixelsPerChar;
			Ypixel = Ystart * pixelsPerChar;
			for (xi = (Xpixel + pixelsPerChar - 1); xi > Xpixel; xi--) {          //calculate right border of character
				for (int yj = (Ypixel + pixelsPerChar - 1); yj > Ypixel; yj--) {
					alpha = image_bytes[ (yj * 1024 + xi) * numOfChannels + checkByte];
					if (alpha > 0) break;
				}
				if (alpha > 0) break;
			}
			xi += pixelsPerChar / 10;
			if (xi > Xpixel + pixelsPerChar - 1) xi = Xpixel + pixelsPerChar - 1;
			if (i == 0) xi = Xstart + pixelsPerChar / 2;

			glyph_coords[i].Xmin = Xstart * character_height;
			glyph_coords[i].Xmax = Xstart * character_height + character_height * (xi - Xpixel) / (float)pixelsPerChar; //right border based on dx pixels
			glyph_coords[i].Ymin = Ystart * character_height + character_height;
			glyph_coords[i].Ymax = Ystart * character_height;
			glyph_coords[i].height_ratio = (xi - Xpixel) / (float)pixelsPerChar;
		}

		stbi_image_free(image_bytes);
	}
	void set_projection_mat(glm::mat4 projection) {
		text_SP->activate_shader();
		text_SP->set_Uniform_Mat4("projectionMatrix", projection);
	}
	void set_view_matrix(glm::mat4 view) {
		text_SP->activate_shader();
		text_SP->set_Uniform_Mat4("viewMatrix", view);
	}
	//update buffer for just one single text
	void update_text(const char* text, float letter_height, glm::vec2 bottom_left_position, glm::vec4 text_color) {
		const int size = strlen(text);
		float stride = 0;
		text_size = size;
		TextData* ptr = text_buffer;
		for (int i = 0; i < size; i++) {
			Glyph& glyph = glyph_coords[int(text[i]) - 32];
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymin };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymin };
			ptr->color = text_color;
			ptr++;

			stride += letter_height * glyph.height_ratio;
		}
	}
	void add_text_to_buffer(const char* text, float letter_height, glm::vec2 bottom_left_position, glm::vec4 text_color) {
		TextData* ptr = text_buffer;
		ptr += text_size * 4;
		const int size = strlen(text);
		float stride = 0;
		text_size += size;
		for (int i = 0; i < size; i++) {
			Glyph& glyph = glyph_coords[int(text[i]) - 32];
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymin };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymin };
			ptr->color = text_color;
			ptr++;

			stride += letter_height * glyph.height_ratio;
		}
	}
	void update_centered_text(const char* text, float letter_height, glm::vec2 center_position, glm::vec4 text_color) {
		const int size = strlen(text);
		float stride = 0;
		text_size = size;
		float totalWidth = 0.f;
		for (int i = 0; i < size; i++) {
			totalWidth += letter_height * glyph_coords[int(text[i]) - 32].height_ratio;
		}
		glm::vec2 bottom_left_position(center_position.x - totalWidth * 0.5f, center_position.y - letter_height * 0.5);
		TextData* ptr = text_buffer;
		for (int i = 0; i < size; i++) {
			Glyph& glyph = glyph_coords[int(text[i]) - 32];
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymin };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymin };
			ptr->color = text_color;
			ptr++;

			stride += letter_height * glyph.height_ratio;
		}
	}
	//allows to add any kind of centered text to buffer, then render all added text with 1 draw call
	void add_centered_text_to_buffer(const char* text, float letter_height, glm::vec2 center_position, glm::vec4 text_color) {
		TextData* ptr = text_buffer;
		ptr += text_size * 4;
		const int size = strlen(text);
		float stride = 0;
		text_size += size;
		float totalWidth = 0.f;
		for (int i = 0; i < size; i++) {
			totalWidth += letter_height * glyph_coords[int(text[i]) - 32].height_ratio;
		}
		glm::vec2 bottom_left_position(center_position.x - totalWidth * 0.5f, center_position.y - letter_height * 0.5);
		for (int i = 0; i < size; i++) {
			Glyph& glyph = glyph_coords[int(text[i]) - 32];
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymin };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y + letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { bottom_left_position.x + stride + letter_height * glyph.height_ratio, bottom_left_position.y };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymin };
			ptr->color = text_color;
			ptr++;

			stride += letter_height * glyph.height_ratio;
		}
	}
	//update buffer for an array of values, where each value has it's own starting point on the screen
	//info_buffer contains the values(which will be converted to int) and their positions(x, y)
	void update_text_array(GLfloat* info_buffer, GLintptr amount_of_values, float letters_height, glm::vec4 text_color) {
		if (!info_buffer) return;
		int code, buffer_size = 0, v_index = 0, str_index = 0;
		float stride = 0;
		std::string values;
		for (int i = 0; i < amount_of_values; i++) {
			int value = info_buffer[i*3];
			values += std::to_string(value) + " ";
		}
		buffer_size = values.size() - amount_of_values;
		TextData* ptr = text_buffer;
		for (int v = 0; v < amount_of_values; v++) {
			stride = 0;
			while (true) {
				code = values[str_index] - 32;
				if (code == 0) {
					str_index++;
					break;
				}
				Glyph& glyph = glyph_coords[code];
				ptr->vertices = { info_buffer[v * 3 + 1] + stride, info_buffer[v * 3 + 2] };
				ptr->tex_coords = { glyph.Xmin, glyph.Ymin };
				ptr->color = text_color;
				ptr++;
				ptr->vertices = { info_buffer[v * 3 + 1] + stride, info_buffer[v * 3 + 2] + letters_height };
				ptr->tex_coords = { glyph.Xmin, glyph.Ymax };
				ptr->color = text_color;
				ptr++;
				ptr->vertices = { info_buffer[v * 3 + 1] + stride + letters_height * glyph.height_ratio, info_buffer[v * 3 + 2] + letters_height };
				ptr->tex_coords = { glyph.Xmax, glyph.Ymax };
				ptr->color = text_color;
				ptr++;
				ptr->vertices = { info_buffer[v * 3 + 1] + stride + letters_height * glyph.height_ratio, info_buffer[v * 3 + 2] };
				ptr->tex_coords = { glyph.Xmax, glyph.Ymin };
				ptr->color = text_color;
				ptr++;

				stride += letters_height * glyph.height_ratio;
				v_index++;
				str_index++;
			}
		}
		text_size = buffer_size;
	}
	//update text for object info box
	void update_object_info_box_text(const char* text, float letter_height, glm::vec2 upper_left_position, glm::vec4 text_color, glm::vec2& box_size) {
		int size = strlen(text);
		int v_index = 0;
		float stride = 0.f;
		float sizeY = 0.f;
		float sizeX = 0.f;
		text_size = 0;
		TextData* ptr = text_buffer;
		for (int i = 0; i < size; i++) {
			if (text[i] == '/') {
				sizeY += 1;
				stride = 0.f;
				continue;
			}
			text_size ++;
			Glyph& glyph = glyph_coords[int(text[i]) - 32];
			ptr->vertices = { upper_left_position.x + stride, upper_left_position.y - letter_height - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymin };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { upper_left_position.x + stride, upper_left_position.y - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { upper_left_position.x + stride + letter_height * glyph.height_ratio, upper_left_position.y - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { upper_left_position.x + stride + letter_height * glyph.height_ratio, upper_left_position.y - letter_height - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymin };
			ptr->color = text_color;
			ptr++;

			stride += letter_height * glyph.height_ratio;
			if (stride > sizeX) sizeX = stride;
			v_index++;
		}
		box_size = { sizeX, letter_height + sizeY * letter_height };
	}
	void add_info_box_text_to_buffer(const char* text, float letter_height, glm::vec2 upper_left_position, glm::vec4 text_color, glm::vec2& box_size) {
		TextData* ptr = text_buffer;
		ptr += text_size * 4;
		int size = strlen(text);
		int v_index = 0;
		float stride = 0.f;
		float sizeY = 0.f;
		float sizeX = 0.f;
		for (int i = 0; i < size; i++) {
			if (text[i] == '/') {
				sizeY += 1;
				stride = 0.f;
				continue;
			}
			text_size++;
			Glyph& glyph = glyph_coords[int(text[i]) - 32];
			ptr->vertices = { upper_left_position.x + stride, upper_left_position.y - letter_height - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymin };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { upper_left_position.x + stride, upper_left_position.y - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmin, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { upper_left_position.x + stride + letter_height * glyph.height_ratio, upper_left_position.y - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymax };
			ptr->color = text_color;
			ptr++;
			ptr->vertices = { upper_left_position.x + stride + letter_height * glyph.height_ratio, upper_left_position.y - letter_height - sizeY * letter_height };
			ptr->tex_coords = { glyph.Xmax, glyph.Ymin };
			ptr->color = text_color;
			ptr++;

			stride += letter_height * glyph.height_ratio;
			if (stride > sizeX) sizeX = stride;
			v_index++;
		}
		box_size = { sizeX, letter_height + sizeY * letter_height };
	}
	void render_text() {
		text_vbo->bind_VBO();
		glBufferSubData(GL_ARRAY_BUFFER, 0, text_size * 4 * sizeof(TextData), text_buffer);

		text_SP->activate_shader();
		text_vao->bind_VAO();
		texture.bind(0);
		glDrawElements(GL_TRIANGLES, text_size * 6, GL_UNSIGNED_INT, 0);
		//glBindTexture(GL_TEXTURE_2D, 0);
		text_size = 0;
	}
private:
	struct Glyph {
		float Xmin, Xmax, Ymin, Ymax, height_ratio;
	} glyph_coords[95];
	struct TextData {
		glm::vec2 vertices;
		glm::vec2 tex_coords;
		glm::vec4 color;
	};
	Smart_ptr<ShaderProgram> text_SP;
	Smart_ptr<VAO> text_vao;
	Smart_ptr<VBO> text_vbo;
	Smart_ptr<EBO> text_ebo;
	Texture texture{ 1 };
	float character_height = 0.0625;
	int text_size = 0; //amount if indeces needed
	static const int MAX_CHARACTERS_PER_DRAW = 500;
	TextData text_buffer[4 * MAX_CHARACTERS_PER_DRAW];
	GLuint* text_index_buffer = nullptr;
};

class Game {
public:
	Game();
	~Game() {
		uninit();
	}

	void input();
	bool update(GLFWwindow* window, float deltaTime);
	void render();
	void toggle_Fullscreen(GLFWwindow* window);

	void init();
	void uninit();
	//init
	void init_inventory_buffer();
	void init_main_buttons();
	void init_world_buttons();
	void init_audio();
	//inventory
	void recolor_active_bar_slot();
	void reset_active_bar_slot();
	void update_item_info_box(ObjectType type, int id);
	void remove_inventory_item(int slot_index);
	void update_main_crafting_slot(int colorVertex_id, int craft_index, int& text_info_index, Vertex2f*& buffer_ptr);
	void craft_item(int craft_id);
	std::vector<int> get_available_crafts();
	//effects
	void update_effect_info_box(Effect& effect);
	void use_item_with_effect(int inventory_slot);
	void apply_player_effect(Effect effect);
	void apply_entity_effect(Effect effect, EntityStats& stats);
	//weapons
	void activate_weapon(WeaponType type);
	bool shoot_arrow(int weapon_id);
	bool shoot_bullet(int weapon_id);
	bool shoot_magic(int weapon_id);
	bool throw_projectile(int weapon_id);
	void update_active_weapon(float deltaTime);
	//items
	void drop_item(int id, float X, float Y, int amount, float Xinc, bool pick_cd, float cd_time);
	void drop_enemy_items(int enemy_id, float xPos, float yPos);
	bool try_to_pick_item(DroppedItem& item);
	bool item_is_stackable(int id, ObjectType type);
	//world objects
	void update_wall_visibility(int x, int y);
	void update_lighting(glm::vec2 light_pos, glm::vec3 light_color, float radius, bool add);
	void update_block_light_impact(int column, int line, bool use_recursive_method, int recursive_depth);
	bool is_solid_block(int column, int line);
	void set_block(int x, int y, int id);
	void destroy_object(int x, int y);
	void destroy_complex_object(int x, int y);
	void set_wall(int x, int y, unsigned short id);
	void destroy_wall(int x, int y);
	//world
	void generate_world();
	void save_world_in_file(const char* fileName);
	void load_world_from_file(const char* fileName);
	void create_the_world_thread();
	void load_the_world_thread();
	void exit_and_save_the_world_thread();
	void load_available_saves();

	glm::vec4 getRainbowColor(float time) {
		float r = std::sin(time * 0.1f + 0.0f) * 0.5f + 0.5f;
		float g = std::sin(time * 0.1f + 2.0f) * 0.5f + 0.5f;
		float b = std::sin(time * 0.1f + 4.0f) * 0.5f + 0.5f;
		return glm::vec4(r, g, b, 1.f);
	}

	//controls
	bool keyStates[350];
	Mouse mouse;
	//screen size
	float ScreenWidth = 1920;
	float ScreenHeight = 1080;
private:
	//CONSTRAINTS
	const int MAX_SQUARES = 15000; //max amount of squares for sprites to draw per 1 call
	const int MAX_VERTEX_AMOUNT = MAX_SQUARES * 4;
	const int MAX_INDEX_AMOUNT = MAX_SQUARES * 6;
	const int MAX_CRAFTS_AVAILABLE = 12;
	const int MAX_ENTITIES = 100;
	float BLOCK_VISIBLE_SIZE = 40.f;  //the standard visible size of one block for Full HD resolution is 40.f
	bool collisionIsOn = true;
	//camera(Player) values
	struct Camera {
		float dX = 0.f, dY = 0.f;
		float scaling = 1.f, scalingDx = 0.f, scalingDy = 0.f;
		float rightBorderDx = 0.f, topBorderDy = 0.f;
	} camera;
	float playerXinc = 0.f, playerYinc = 0.f;
	Player player;
	//inventory
	static const int INVENTORY_SIZE = 191; //max is 191 (85 for crafting, 40 for chest, 66 for player's inventory)
	static const int CRAFTING_SLOTS = 85;
	InventoryPair inventory_array[INVENTORY_SIZE - CRAFTING_SLOTS]; //stores all objects that are in inventory, 50 main, 8 for coins and ammo, 8 for armor and accessories, 40 for chest
	InventoryPair currently_moving_object; //stores the object that is currently moving with mouse in inventory
	int index_of_last_slot_picked = 0; //stores the index of the last slot where the object was taken with mouse (puts it back if leaving inventory with currently taken object)
	bool inventoryIsOpen = false;
	ActiveWeapon active_weapon;
	ActiveBreakableObject active_breakable_object;
	ActiveChest active_chest;
	ObjectInfoBox object_info_box[2]; //one for object name from active slot and one for object that mouse cursor is pointing on
	EntityInfoText entity_info_text;
	short active_bar_slot = 0; //from 0 to 9

	ColorVertex2f inventory_vert_buf[INVENTORY_SIZE * 4];
	ColorVertex2f inv_chest_slots_buf[40 * 4];
	int additional_slots = 0; //to calculate how many slots should be drawn in inventory
	int additional_sprites_slots = 0;

	Vertex2f inventory_obj_vert_buf[(INVENTORY_SIZE + 1) * 4]; //array that contains vertices for objects in inventory
	GLfloat inventory_text_info[(INVENTORY_SIZE + 1 - 8 - 70) * 3]; //array that contains info about all text in inventory (value and starting position of each value)
	//(max size + 1 because of object that is holded by mouse)
	int inventory_index_size = 0;
	int inventory_text_size = 0;
	//game objects information (database)
	//ambient objects
	AmbientController ambientController;
	//objects info: 32 blocks, 6 complex objects, 10 weapons, 1 wall, 3 ammo, 3 coins, 4 materials
	Smart_ptr<ObjectInfo> objectInfo[101];
	//entities info: 7 projectiles, 1 zombie, 1 slime, 3 eyes
	Smart_ptr<EntityInfo> entityInfo[15];
	//effects info:
	Smart_ptr<GameEffects::EffectBase> effects[11];
	//particles info
	ParticleInfo particlesInfo[2];
	std::vector<Particle> particles_v;
	//recipes info
	CraftableItem craftable_items[50];
	CraftingSystem crafting_system;

	//MAIN ARRAYS AND VECTORS
	WorldSlot** sprites_Array = nullptr; //pointer to array of Slots in the world for objects and walls
	ShaderLightingInfo* lightMap_data = nullptr;
	std::vector<Smart_ptr<EntityBase>> entities;
	std::vector<Smart_ptr<EntityBase>> projectiles;
	std::vector<DroppedItem> dropped_items;
	std::vector<DamageText> damage_text;
	//world size (only one for now)
	unsigned short world_width = 4200;
	unsigned short world_height = 1200;
	float skyColor[3]{}; //current color of the sky
	float dayColor[3] = { 0.53, 0.81, 0.92 };
	float nightColor[3] = { 0.0, 0.0, 0.05 };
	float day_ratio = 1.0; //1 is day, 0 is night 
	const float dayTime = 600.f; //10 minutes is the duration of day/night
	float cycle_time = 0.0; //time since the day/night has started
	bool isDay = true; //true - day, false - night
	float day_time_speed = 1.0f; //made in heaven

	//textures
	Texture a_textures{ 5 };

	//Graphics main objects
	Smart_ptr<ShaderProgram> sprite_SP_ptr;
	Smart_ptr<ShaderProgram> entity_sprite_SP_ptr;
	Smart_ptr<ShaderProgram> ambient_sprite_SP_ptr;
	Smart_ptr<ShaderProgram> UI_sprite_SP_ptr;

	Smart_ptr<VAO> sprite_vao_p;
	Smart_ptr<VBO> sprite_vbo_p;
	Smart_ptr<EBO> sprite_ebo_p;
	Smart_ptr<SSBO> sprite_ssbo_p;
	Smart_ptr<SSBO> sprite_ambient_ssbo_p;
	Smart_ptr<SSBO> spriteLightMapSSBO;

	Smart_ptr<VAO> instance_vao_p;
	Smart_ptr<VBO> instance_vbo_p;
	Smart_ptr<EBO> instance_ebo_p;

	Smart_ptr<ShaderProgram> color_ui_SP;
	Smart_ptr<VAO> inventory_vao;
	Smart_ptr<VBO> inventory_vbo;
	Smart_ptr<EBO> inventory_ebo;
	Smart_ptr<VAO> inventory_objects_vao;
	Smart_ptr<VBO> inventory_objects_vbo;
	Smart_ptr<EBO> inventory_objects_ebo;
	Smart_ptr<VAO> ui_elements_vao;
	Smart_ptr<VBO> ui_elements_vbo;
	Smart_ptr<EBO> ui_elements_ebo;

	Smart_ptr<ShaderProgram> entity_SP;
	Smart_ptr<VAO> entity_vao;
	Smart_ptr<VBO> entity_vbo;
	Smart_ptr<EBO> entity_ebo;

	std::vector<Button> main_buttons;
	std::vector<Button> world_buttons;
	TextField text_field;
	const static int MAX_UI_COLOR_ELEMENTS = 20;
	ColorVertex2f buttons_buffer[MAX_UI_COLOR_ELEMENTS * 4];
	int buttons_amount = 0;

	Vertex2f* sprite_vertices_buffer = nullptr;
	GLfloat player_hitbox_buffer[24];

	EntityData entity_sprite_buf[1000]; //for 200 entities
	SpriteData ambient_sprite_buf[20]; //20 ambient sprites

	int entities_count = 0;
	GLuint* sprite_index_buffer = nullptr;
	int index_size = 0;
	int buffer_size = 0;

	//text manager
	Smart_ptr<Text> text_manager;
	//audio manager
	Smart_ptr<AudioManager> audio_manager;
	int current_music_id = 0;

	//Matrices
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	//save the world info in file in specific interval, for the safety :)
	int auto_save_interval = 120; //120 seconds
	float game_delta_Time = 0.f;
	int FPS_counter = 0;
	int current_FPS = 0;
	float rainbow_color_time = 0;

	//save files vector
	std::vector<std::string> save_Files;
	std::string active_world = "";
	bool loading_the_world = false;
	bool saving_the_world = false;
	bool creating_the_world = false;
	//options
	bool isFullscreen = false;
	//game state
	enum Game_State : uint8_t { inMainMenu, inOptions, inControlsOptions, inAudioOptions, inRenderingOptions, inWorldExplorer, WorldIsLoading, WorldIsSaving, WorldIsCreating, inWorldCreator, inGame, inGamePause }
	game_update_state = inMainMenu, game_render_state = inMainMenu;
};