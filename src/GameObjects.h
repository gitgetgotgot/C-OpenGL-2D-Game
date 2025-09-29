#pragma once

#include "Colliders.h"

template<class T> class Smart_ptr {
public:
	Smart_ptr() {
		ptr = nullptr;
	}
	Smart_ptr(T* object) {
		ptr = object;
	}
	Smart_ptr(Smart_ptr&& other) noexcept {
		ptr = other.ptr;
		other.ptr = nullptr;
	}
	Smart_ptr& operator=(Smart_ptr&& other) noexcept {
		if (this != &other) {
			delete ptr;
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}
	//Smart_ptr(const Smart_ptr&) = delete;
	//Smart_ptr& operator=(const Smart_ptr&) = delete;
	~Smart_ptr() {
		if (ptr)
			delete ptr;
	}
	void clear() noexcept {
		delete ptr;
		ptr = nullptr;
	}
	T& operator*() {
		return *ptr;
	}
	T* operator->() {
		return ptr;
	}
	Smart_ptr<T>& operator=(T* object) {
		this->ptr = object;
		return *this;
	}
private:
	T* ptr;
};

enum ObjectType : uint8_t { None, isBlock, isWall, isComplexObject, isCompObjPart, isWeapon, isAmmo, isArmor, isMaterial, isCoin, isAccessory, isConsumable, isPotion };
enum BlockType { isSolidBlock, isWood, isTreeTop, isPlatform, isTorch, isLiquid, isGrass, isFlower, isGlass, isBottle };
enum ComplexObjectType { isWorkbench, isFurnace, isChest, isDoor, isTable, isChair, isAnvil, isLamp, isChandelier, isLyfeCrystal };
enum WeaponType { isPickaxe, isSword, isDagger, isAxe, isHammer, isGun, isShotgun, isBow, isThrowable, isSpear, isMagical };
enum AmmoType { isArrow, isBullet, isMagicBall };

struct Mouse {
	float mouseX = 0, mouseY = 0;
	float wheelOffset = 0;
	bool left_button = false, right_button = false;
};

struct Vertex2f {
	glm::vec2 vertices;
	glm::vec2 tex_vertices;
	float tex_id;
};
struct EntityData {
	glm::mat4 modelMatrix;
	glm::vec2 tex_coords[4];
	float tex_id;
	float color_value = 0.f; //if entity should always be lightened with it's own color, then color_value is 1.f (not used for now, just for padding)
	float padding[2] = { 0.f, 0.f };
};
struct SpriteData {
	glm::mat4 modelMatrix;
	glm::vec2 tex_coords[4];
	float tex_id = 0.f, light_level = 1.f, opacity = 1.f; //light level and opacity are from 0.0 to 1.0 (can be used for weather system)
	float padding = 0.f;
};
struct ColorVertex2f {
	glm::vec2 vertices;
	glm::vec4 color;
};
struct ShaderLightingInfo {
	float r, g, b;
	float global_light_impact;
	float source_light_impact;
};

constexpr float TEXT_DAMAGE_LIFETIME = 1.0f;
struct DamageText {
	DamageText(glm::vec2 start_pos, glm::vec4 color, std::string damage, float text_max_height) :
		start_pos{ start_pos }, color{ color }, text{ damage }, max_height{ text_max_height } {
		text_height = max_height * 0.2f;
	}
	//return true if should be deleted
	bool updateText(float deltaTime, float block_size) {
		lifeTime += deltaTime;
		start_pos.y += block_size * deltaTime;
		if (lifeTime < TEXT_DAMAGE_LIFETIME * 0.5)
			text_height = lifeTime / (TEXT_DAMAGE_LIFETIME * 0.5) * (0.8 * max_height);
		if (lifeTime >= TEXT_DAMAGE_LIFETIME) {
			return true;
		}
		else return false;
	}
	glm::vec4 color;
	glm::vec2 start_pos;
	float text_height;
	float max_height;
	float lifeTime = 0.f;
	std::string text;
};

struct Button {
	Button() {}
	Button(std::string text, glm::vec2 center_pos, glm::vec2 size, glm::vec4 color, glm::vec4 active_color) : center_pos{ center_pos }, text{ text }, size{ size }, color{ color }, active_color{ active_color } {
		position = { this->center_pos.x - size.x * 0.5, this->center_pos.y - size.y * 0.5 };
	}
	bool cursor_is_inside(float mouseX, float mouseY) {
		if (mouseX >= position.x && mouseX <= position.x + size.x && mouseY >= position.y && mouseY <= position.y + size.y) {
			isActive = true;
			return true;
		}
		else {
			isActive = false;
			return false;
		}
	}
	void update_pressed_state(Mouse& mouse) {
		if (mouse.left_button) {
			if (mouse.mouseX >= position.x && mouse.mouseX <= position.x + size.x && mouse.mouseY >= position.y && mouse.mouseY <= position.y + size.y) {
				isActive = true;
			}
			else {
				isActive = false;
			}
		}
	}
	void update_buffer(ColorVertex2f*& buffer) const {
		if (isActive) {
			buffer->color = active_color;
			buffer->vertices = { position.x, position.y };
			buffer++;
			buffer->color = active_color;
			buffer->vertices = { position.x, position.y + size.y };
			buffer++;
			buffer->color = active_color;
			buffer->vertices = { position.x + size.x, position.y + size.y };
			buffer++;
			buffer->color = active_color;
			buffer->vertices = { position.x + size.x, position.y };
			buffer++;
		}
		else {
			buffer->color = color;
			buffer->vertices = { position.x, position.y };
			buffer++;
			buffer->color = color;
			buffer->vertices = { position.x, position.y + size.y };
			buffer++;
			buffer->color = color;
			buffer->vertices = { position.x + size.x, position.y + size.y };
			buffer++;
			buffer->color = color;
			buffer->vertices = { position.x + size.x, position.y };
			buffer++;
		}
	}
	bool isActive = false;
	std::string text;
	glm::vec2 size;
	glm::vec2 position;
	glm::vec2 center_pos;
	glm::vec4 color, active_color;
};
struct TextField {
	TextField() {}
	TextField(std::string text, glm::vec2 center_pos, glm::vec2 size, glm::vec4 color, glm::vec4 active_color) : center_pos{ center_pos }, text{ text }, size{ size }, color{ color }, active_color{ active_color } {
		position = { this->center_pos.x - size.x * 0.5, this->center_pos.y - size.y * 0.5 };
	}
	void update_for_fileName_input(Mouse& mouse, bool* keyStates) {
		if (mouse.left_button) {
			//mouse.left_button = false;
			if (mouse.mouseX >= position.x && mouse.mouseX <= position.x + size.x && mouse.mouseY >= position.y && mouse.mouseY <= position.y + size.y) {
				isActive = true;
			}
			else {
				isActive = false;
			}
		}
		if (isActive) {
			if (text.size() < 20) {
				for (int i = 65; i < 90; i++)  //key codes for GLFW from A to Z
					if (keyStates[i]) {
						if (keyStates[GLFW_KEY_LEFT_SHIFT] || keyStates[GLFW_KEY_RIGHT_SHIFT]) text += (char)i; //capital letter
						else text += (char)(i + 32); //not capital letter
						keyStates[i] = false;
						return;
					}
				for (int i = 48; i < 58; i++) //numbers from 0 to 9
					if (keyStates[i]) {
						text += (char)i;
						keyStates[i] = false;
						return;
					}
				if (keyStates[32]) { //space
					text += (char)32;
					keyStates[32] = false;
				}
				if (keyStates[45]) { //-
					text += (char)45;
					keyStates[45] = false;
				}
				if (keyStates[95]) { //_
					text += (char)95;
					keyStates[95] = false;
				}
			}
			if (keyStates[GLFW_KEY_BACKSPACE] && text.size() > 0) {
				keyStates[GLFW_KEY_BACKSPACE] = false;
				text.resize(text.size() - 1);
			}
		}

	}
	void update_for_input(Mouse& mouse, bool* keyStates) {
		if (mouse.left_button) {
			mouse.left_button = false;
			if (mouse.mouseX >= position.x && mouse.mouseX <= position.x + size.x && mouse.mouseY >= position.y && mouse.mouseY <= position.y + size.y) {
				isActive = true;
			}
			else {
				isActive = false;
			}
		}
		if (isActive) {
			if (text.size() < 20) {
				for (int i = 65; i < 90; i++)  //key codes for GLFW from A to Z
					if (keyStates[i]) {
						if (keyStates[GLFW_KEY_LEFT_SHIFT] || keyStates[GLFW_KEY_RIGHT_SHIFT]) text += (char)i; //capital letter
						else text += (char)(i + 32); //not capital letter
						keyStates[i] = false;
						return;
					}
				for (int i = 32; i < 65; i++) //from space to @
					if (keyStates[i]) {
						text += (char)i;
						keyStates[i] = false;
						return;
					}
				for (int i = 91; i < 97; i++) //from [ to `
					if (keyStates[i]) {
						text += (char)i;
						keyStates[i] = false;
						return;
					}
				for (int i = 123; i < 127; i++) //from { to ~
					if (keyStates[i]) {
						text += (char)i;
						keyStates[i] = false;
						return;
					}
			}
			if (keyStates[GLFW_KEY_BACKSPACE] && text.size() > 0) {
				keyStates[GLFW_KEY_BACKSPACE] = false;
				text.resize(text.size() - 1);
			}
		}
	}
	void update_buffer(ColorVertex2f*& buffer) const {
		if (isActive) {
			buffer->color = active_color;
			buffer->vertices = { position.x, position.y };
			buffer++;
			buffer->color = active_color;
			buffer->vertices = { position.x, position.y + size.y };
			buffer++;
			buffer->color = active_color;
			buffer->vertices = { position.x + size.x, position.y + size.y };
			buffer++;
			buffer->color = active_color;
			buffer->vertices = { position.x + size.x, position.y };
			buffer++;
		}
		else {
			buffer->color = color;
			buffer->vertices = { position.x, position.y };
			buffer++;
			buffer->color = color;
			buffer->vertices = { position.x, position.y + size.y };
			buffer++;
			buffer->color = color;
			buffer->vertices = { position.x + size.x, position.y + size.y };
			buffer++;
			buffer->color = color;
			buffer->vertices = { position.x + size.x, position.y };
			buffer++;
		}
	}
	bool isActive = false;
	std::string text;
	glm::vec2 size;
	glm::vec2 position;
	glm::vec2 center_pos;
	glm::vec4 color, active_color;
};

//classes that contain info about all objects
class ObjectInfo {
public:
	ObjectInfo(std::string name) : name{ name } {}
	ObjectInfo(ObjectType type, std::string name) : objectType{ type }, name{ name } {}
	ObjectInfo(std::string name, float tex_slot_id, bool emitsLight, glm::vec3 light_color, float light_radius) :
		name{ name }, tex_slot_id{ tex_slot_id }, emitsLight{ emitsLight }, light_color{ light_color }, light_radius{ light_radius } {}
	virtual ~ObjectInfo() {}

	virtual BlockType get_block_type() const { return isSolidBlock; }
	virtual float get_toughness() const { return 0.f; }
	virtual int get_drop_object_id() const { return 0; }
	virtual bool allow_collision() const { return 0; }
	virtual bool allow_bottom_collision() const { return 0; }
	//for complex objects and weapons
	virtual float get_sizeX() const { return 0.f; }
	virtual float get_sizeY() const { return 0.f; }

	virtual ComplexObjectType get_comp_obj_type() const { return isWorkbench; }

	virtual WeaponType get_weapon_type() const { return isPickaxe; }
	virtual int get_weapon_usable_audio_id() const { return 3; }
	virtual int get_damage() const { return 0; }
	virtual float get_speed_factor() const { return 0.f; }
	virtual float get_hit_cd() const { return 0.f; }
	virtual float get_crit_chance() const { return 0.f; }
	virtual bool is_stackable() const { return 0; }
	virtual float get_instrument_power() const { return 0.f; }
	virtual int get_weapon_proj_id() const { return 0; }
	virtual float get_weapon_proj_cd() const { return 0.f; }
	virtual int get_weapon_mana_cost() const { return 0; }

	virtual AmmoType get_ammo_type() const { return isArrow; }

	virtual int get_entity_id() const { return 0; }
	//main info
	ObjectType objectType = None;
	std::string name;
	glm::vec2 texture_coords[4];
	float tex_slot_id = 0.f;
	//light info
	bool emitsLight = false;
	glm::vec3 light_color{ 0.f, 0.f, 0.f };
	float light_radius = 0.f; //in blocks
	//effect info
	int effectId = -1; //id of effect for "usable" items, -1 means no effect
	float effectDuration = 0.f;
};

struct AmbientObject {
	glm::vec2 sprite_size;
	glm::vec2 position;
	float tex_slot_id = 0.f;
};
class AmbientController {
public:
	AmbientController() {}
	AmbientController(float dayTime) : dayTime{ dayTime } {}
	AmbientObject sun, moon[2], clouds[2], far_clouds[3], sky_stars[6];
	glm::vec2 texture_coords[5][4];
	void updateAmbientLayer(bool isDay, float cycleTime, float dayRatio, SpriteData*& ptr, float ScreenWidth, float ScreenHeight, float blockSize, float deltaTime) {
		//sky stars
		glm::vec2* tex = texture_coords[4];
		glm::mat4 matModel;
		float stars_opacity = 0.f;
		if (!isDay) {
			if (cycleTime <= dayTime * 0.1) stars_opacity = cycleTime / (dayTime * 0.1);
			else if (cycleTime >= dayTime * 0.9) stars_opacity = (dayTime - cycleTime) / (dayTime * 0.1);
			else stars_opacity = 1.f;
		}
		for (int i = 0; i < 6; i++) {
			sky_stars[i].position.x += deltaTime * blockSize * sky_stars_speed;
		}
		if (sky_stars[0].position.x >= ScreenWidth) {
			sky_stars[0].position.x -= ScreenWidth * 0.5;
			sky_stars[1].position.x -= ScreenWidth * 0.5;
			sky_stars[2].position.x -= ScreenWidth * 0.5;
			sky_stars[3].position.x -= ScreenWidth * 0.5;
			sky_stars[4].position.x -= ScreenWidth * 0.5;
			sky_stars[5].position.x -= ScreenWidth * 0.5;
		}
		for (int i = 0; i < 6; i++) {
			ptr->tex_coords[0] = tex[0];
			ptr->tex_coords[1] = tex[1];
			ptr->tex_coords[2] = tex[2];
			ptr->tex_coords[3] = tex[3];
			ptr->opacity = stars_opacity;
			matModel = glm::mat4(1.f);
			matModel = glm::translate(matModel, glm::vec3(sky_stars[i].position.x, sky_stars[i].position.y, 0.f));
			matModel = glm::scale(matModel, glm::vec3(ScreenWidth * 0.5, ScreenHeight * 0.5, 0.f));
			ptr->modelMatrix = matModel;
			ptr++;
		}
		//far clouds
		far_clouds[0].position.x -= deltaTime * blockSize * far_clouds_speed;
		far_clouds[1].position.x -= deltaTime * blockSize * far_clouds_speed;
		far_clouds[2].position.x -= deltaTime * blockSize * far_clouds_speed;
		if (far_clouds[0].position.x + ScreenWidth * 0.5 <= 0.f) {
			far_clouds[0].position.x += ScreenWidth * 0.5;
			far_clouds[1].position.x += ScreenWidth * 0.5;
			far_clouds[2].position.x += ScreenWidth * 0.5;
		}
		if (dayRatio < 0.02) dayRatio = 0.02;
		tex = texture_coords[3];
		for (int i = 0; i < 3; i++) {
			ptr->tex_coords[0] = tex[0];
			ptr->tex_coords[1] = tex[1];
			ptr->tex_coords[2] = tex[2];
			ptr->tex_coords[3] = tex[3];
			ptr->light_level = dayRatio;
			matModel = glm::mat4(1.f);
			matModel = glm::translate(matModel, glm::vec3(far_clouds[i].position.x, ScreenHeight - ScreenWidth * 0.13f, 0.f));
			matModel = glm::scale(matModel, glm::vec3(ScreenWidth * 0.5, ScreenWidth * 0.0625f, 0.f));
			ptr->modelMatrix = matModel;
			ptr++;
		}
		// Sun / Moon
		if (isDay) {
			chooseMoon = true;
			tex = texture_coords[0];
			ptr->tex_coords[0] = tex[0];
			ptr->tex_coords[1] = tex[1];
			ptr->tex_coords[2] = tex[2];
			ptr->tex_coords[3] = tex[3];
			glm::mat4 matModel(1.f);
			matModel = glm::translate(matModel, glm::vec3(ScreenWidth / 2, -ScreenHeight * 0.55, 0.f));
			matModel = glm::rotate(matModel, glm::radians(40.f - cycleTime * (80.f / dayTime)), glm::vec3(0.f, 0.f, 1.f));
			matModel = glm::translate(matModel, glm::vec3(-blockSize * sun.sprite_size.x * 0.5, ScreenHeight * 1.5f - blockSize * sun.sprite_size.y * 0.5, 0.f));
			matModel = glm::scale(matModel, glm::vec3(blockSize * sun.sprite_size.x, blockSize * sun.sprite_size.y, 0.f));
			ptr->modelMatrix = matModel;
		}
		else {
			if (chooseMoon) {
				chooseMoon = false;
				moon_index = rand() % 2;
			}
			tex = texture_coords[1 + moon_index];
			ptr->tex_coords[0] = tex[0];
			ptr->tex_coords[1] = tex[1];
			ptr->tex_coords[2] = tex[2];
			ptr->tex_coords[3] = tex[3];
			glm::mat4 matModel(1.f);
			matModel = glm::translate(matModel, glm::vec3(ScreenWidth / 2, -ScreenHeight * 0.55, 0.f));
			matModel = glm::rotate(matModel, glm::radians(40.f - cycleTime * (80.f / dayTime)), glm::vec3(0.f, 0.f, 1.f));
			matModel = glm::translate(matModel, glm::vec3(-blockSize * moon[moon_index].sprite_size.x * 0.5, ScreenHeight * 1.5f - blockSize * moon[moon_index].sprite_size.y * 0.5, 0.f));
			matModel = glm::scale(matModel, glm::vec3(blockSize * moon[moon_index].sprite_size.x, blockSize * moon[moon_index].sprite_size.y, 0.f));
			ptr->modelMatrix = matModel;
		}
		ptr++;
		//clouds
		tex = texture_coords[3];
		clouds[0].position.x -= deltaTime * blockSize * clouds_speed;
		clouds[1].position.x -= deltaTime * blockSize * clouds_speed;
		if (clouds[0].position.x + ScreenWidth <= 0.f) {
			clouds[0].position.x += ScreenWidth;
			clouds[1].position.x += ScreenWidth;
		}
		ptr->tex_coords[0] = tex[0];
		ptr->tex_coords[1] = tex[1];
		ptr->tex_coords[2] = tex[2];
		ptr->tex_coords[3] = tex[3];
		ptr->light_level = dayRatio;
		matModel = glm::mat4(1.f);
		matModel = glm::translate(matModel, glm::vec3(clouds[0].position.x, ScreenHeight - ScreenWidth * 0.125f, 0.f));
		matModel = glm::scale(matModel, glm::vec3(ScreenWidth, ScreenWidth * 0.125f, 0.f));
		ptr->modelMatrix = matModel;
		ptr++;
		ptr->tex_coords[0] = tex[0];
		ptr->tex_coords[1] = tex[1];
		ptr->tex_coords[2] = tex[2];
		ptr->tex_coords[3] = tex[3];
		ptr->light_level = dayRatio;
		matModel = glm::mat4(1.f);
		matModel = glm::translate(matModel, glm::vec3(clouds[1].position.x, ScreenHeight - ScreenWidth * 0.125f, 0.f));
		matModel = glm::scale(matModel, glm::vec3(ScreenWidth, ScreenWidth * 0.125f, 0.f));
		ptr->modelMatrix = matModel;
		ptr++;
		
	}
private:
	bool chooseMoon = false;
	int moon_index = 0;
	float dayTime;
	float clouds_speed = 1.f; //1 block per second
	float far_clouds_speed = 0.5f; //half block per second
	float sky_stars_speed = 0.25f; //quarter block per second
};

class BlockInfo : public ObjectInfo {
public:
	BlockInfo(std::string name, BlockType type, float toughness, int drop_obj_id, bool collision, bool bottomCollision) : ObjectInfo(name), type{ type }, toughness{ toughness },
		drop_object_id{ drop_obj_id }, allowCollision{ collision }, allowBottomCollision{ bottomCollision } {
		objectType = isBlock;
	}
	BlockInfo(std::string name, BlockType type, float tex_slot_id, bool emitsLight, glm::vec3 light_color, float light_radius, float toughness, int drop_obj_id, bool collision, bool bottomCollision) :
		ObjectInfo(name, tex_slot_id, emitsLight, light_color, light_radius), type{ type }, toughness{ toughness }, drop_object_id{ drop_obj_id }, allowCollision{ collision }, allowBottomCollision{ bottomCollision } {
		objectType = isBlock;
	}
	float get_toughness() const override { return toughness; }
	int get_drop_object_id() const override { return drop_object_id; }
	bool allow_collision() const override { return allowCollision; }
	bool allow_bottom_collision() const override { return allowBottomCollision; }
	BlockType get_block_type() const override { return type; }
private:
	BlockType type;
	float toughness = 1;
	int drop_object_id;
	bool allowCollision;
	bool allowBottomCollision;
};

class ComplexObjectInfo : public BlockInfo {
public:
	//it doesn't matter what type of block is used for the BlockInfo constructor here, so it's just isSolidBlock
	ComplexObjectInfo(std::string name, ComplexObjectType type, float toughness, int drop_obj_id, float blWidth, float blHeight, bool collision, bool bottomCollision) :
		BlockInfo(name, isSolidBlock, toughness, drop_obj_id, collision, bottomCollision), blocksWidth{ blWidth }, type{ type }, blocksHeight{ blHeight } {
		objectType = isComplexObject;
	}
	ComplexObjectInfo(std::string name, ComplexObjectType type, float tex_slot_id, bool emitsLight, glm::vec3 light_color, float light_radius, float toughness, int drop_obj_id, float blWidth,
		float blHeight, bool collision, bool bottomCollision) : BlockInfo(name, BlockType::isSolidBlock, tex_slot_id, emitsLight, light_color, light_radius, toughness, drop_obj_id, collision, bottomCollision), type{ type },
		blocksWidth{ blWidth }, blocksHeight{ blHeight } {
		objectType = isComplexObject;
	}
	float get_sizeX() const override { return blocksWidth; }
	float get_sizeY() const override { return blocksHeight; }
	ComplexObjectType get_comp_obj_type() const override { return type; }
private:
	ComplexObjectType type;
	float blocksWidth, blocksHeight;
};

class WeaponInfo : public ObjectInfo {
public:
	WeaponInfo(std::string name, WeaponType type, int dmg, float hit_cd, float spd_factor, float proj_cd, int proj_id, float sizeX, float sizeY, float crit_chance, bool isStackable, int use_sound_id) :
		ObjectInfo(name), type{ type }, damage{ dmg }, hit_cd{ hit_cd }, speed_factor{ spd_factor }, projectile_cd{ proj_cd }, projectile_id{ proj_id },
		sizeX{sizeX}, sizeY{sizeY}, crit_chance{crit_chance}, isStackable{isStackable}, use_sound_id{use_sound_id} {
		objectType = isWeapon;
	}
	WeaponType get_weapon_type() const { return type; }
	int get_damage() const override { return damage; }
	float get_speed_factor() const override { return speed_factor; }
	float get_hit_cd() const override { return hit_cd; }
	float get_sizeX() const override { return sizeX; }
	float get_sizeY() const override { return sizeY; }
	float get_crit_chance() const override { return crit_chance; }
	bool is_stackable() const override { return isStackable; }
	int get_weapon_usable_audio_id() const override { return use_sound_id; }
	int get_weapon_proj_id() const override { return projectile_id; }
	float get_weapon_proj_cd() const override { return projectile_cd; }
private:
	WeaponType type;
	int damage;
	float hit_cd = 0.5f; //cd for each enemy that was hit by this weapon
	float speed_factor = 1.f; //speed of weapon (speed of swinging, starting speed of arrow or bullet, speed of throwable weapons, etc.)
	float projectile_cd = 1.f; //cd for shooting projectiles (from any kind of weapon if it has this ability)
	int projectile_id = -1; //if it's a sword with projectiles, magic weapon or throwable weapon, etc. (-1 means no projectile)
	float sizeX;
	float sizeY;
	float crit_chance;
	bool isStackable = false;
	int use_sound_id = 0;
};
class InstrumentalWeaponInfo : public WeaponInfo {
public:
	InstrumentalWeaponInfo(std::string name, WeaponType type, float power, int dmg, float hit_cd, float spd_factor, float proj_cd, int proj_id, float sizeX, float sizeY, float crit_chance, bool isStackable, int use_sound_id) :
		WeaponInfo(name, type, dmg, hit_cd, spd_factor, proj_cd, proj_id, sizeX, sizeY, crit_chance, isStackable, use_sound_id) {
		this->power = power;
	}
	float get_instrument_power() const override { return power; }
private:
	float power;
};
class MagicalWeaponInfo : public WeaponInfo {
public:
	MagicalWeaponInfo(std::string name, WeaponType type, int dmg, int manaCost, float hit_cd, float spd_factor, float proj_cd, int proj_id, float sizeX, float sizeY, float crit_chance, bool isStackable, int use_sound_id) :
		WeaponInfo(name, type, dmg, hit_cd, spd_factor, proj_cd, proj_id, sizeX, sizeY, crit_chance, isStackable, use_sound_id) {
		this->manaCost = manaCost;
	}
	int get_weapon_mana_cost() const { return manaCost; }
private:
	int manaCost;
};

class AmmoInfo : public ObjectInfo {
public:
	AmmoInfo(std::string name, AmmoType type, int dmg, float sizeX, float sizeY, int entity_id) : ObjectInfo(name), type{ type }, damage{ dmg }, sizeX{ sizeX }, sizeY{ sizeY }, entity_id{ entity_id } {
		objectType = isAmmo;
	}
	AmmoInfo(std::string name, AmmoType type, float tex_slot_id, bool emitsLight, glm::vec3 light_color, float light_radius, int dmg, float sizeX, float sizeY, int entity_id) :
		ObjectInfo(name, tex_slot_id, emitsLight, light_color, light_radius), type{ type }, damage{ dmg }, sizeX{ sizeX }, sizeY{ sizeY }, entity_id{ entity_id } {
		objectType = isAmmo;
	}
	AmmoType get_ammo_type() const override { return type; }
	int get_damage() const override { return damage; }
	float get_sizeX() const override { return sizeX; }
	float get_sizeY() const override { return sizeY; }
	int get_entity_id() const override { return entity_id; }
private:
	AmmoType type;
	int entity_id;
	int damage;
	float sizeX;
	float sizeY;
};

//OBJECT CLASSES WITH COMPONENT STRUCTURE
class GameObject;
struct ItemObject {
	ItemObject(ObjectType type, unsigned short id) : type{ type }, id{ id } {}
	ObjectType type;
	unsigned short id;
};
struct InventoryPair {
	ItemObject object{ None, 0 };
	unsigned short amount = 0;
};
class ObjectComponent {
public:
	virtual ~ObjectComponent() {}
	virtual unsigned short get_column() const { return -1; }
	virtual unsigned short get_line() const { return -1; }
	virtual InventoryPair* get_chest_slots() { return nullptr; }
	virtual uint8_t get_door_state() const { return -1; }
};
class ComplexObjectPartComponent : public ObjectComponent {
public:
	ComplexObjectPartComponent(unsigned short column, unsigned short line) :column{ column }, line{ line } {}
	unsigned short get_column() const override { return column; }
	unsigned short get_line() const override { return line; }
private:
	unsigned short column, line;
};
class ChestComponent : public ObjectComponent {
public:
	InventoryPair* get_chest_slots() override { return chest_slots; }
private:
	InventoryPair chest_slots[40];
};
class DoorComponent : public ObjectComponent {
public:
	DoorComponent(uint8_t state) : door_state{ state } {}
	uint8_t get_door_state() const override { return door_state; }
private:
	uint8_t door_state = 0; //0 - closed, 1 - opened to left, 2 - opened to right
};

class GameObject {
public:
	GameObject() {}
	GameObject(ObjectType type, unsigned short object_ID) : object_type{ type }, object_id{ object_ID }, component{ nullptr } {}
	GameObject(ObjectType type, unsigned short object_ID, ObjectComponent* component) : object_type{ type }, object_id{ object_ID }, component{component} {}
	GameObject& operator=(GameObject&& other) noexcept {
		if (this == &other) return *this;
		object_type = other.object_type;
		object_id = other.object_id;
		delete component;
		component = other.component;
		other.component = nullptr;
		return *this;
	}
	~GameObject() {
		delete component;
	}
	unsigned short object_id = 0;
	ObjectType object_type = None;
	ObjectComponent* component = nullptr;
};
struct WorldSlot {
	GameObject object;
	unsigned short wall_id = 0; //0 means there is no wall in this slot
	bool wall_is_visible = true; //used to decide if wall should be rendered or not, changes based on nearby blocks when they are destroyed/placed
};

struct ActiveChest { //structure for the active chest that the player has opened/closed
	bool isOpen = false;
	InventoryPair* slot_pointer = nullptr;
};

struct ObjectInfoBox {
	bool show_box = false;
	glm::vec2 box_size{ 0.f, 0.f };
	std::string box_string;
	glm::vec2 starting_pos; //left upper corner
	glm::vec4 text_color = { 1.f, 1.f, 1.f, 1.f };
	float box_vertices[24] = { 0.f,0.f, 0.f,0.f,1.f,0.8f,    //0,1
								0.f,0.f, 0.f,0.f,1.f,0.8f,    //6,7
								0.f,0.f, 0.f,0.f,1.f,0.8f,    //12,13
								0.f,0.f, 0.f,0.f,1.f,0.8f };  //18,19
};

struct EntityInfoText {
	bool isActive = false;
	std::string info;
	glm::vec2 start_pos = {0.f, 0.f};
};

struct ActiveWeapon { //draw sprite for active weapon (also should be used for tools, not only weapons, but it's not important for now)
	bool isActive = false;
	bool hitboxIsActive = false;
	bool points_to_left;
	bool render_upside_down = false;
	int weapon_id;
	float angle = 0.f;
	float sizeX, sizeY;
	float time_spent = 0.f;
	float speed_factor = 1.f;
	float power = 1.f; //for pickaxe, axe, etc.
	float projectile_attack_cd = 0.f; //cooldown of attack(shooting arrow from bow, etc.)
	float projectile_attack_current_cd = 0.f;
	float hit_cd = 0.f; //cd for each enemy that was hit by swinging weapon
	float time_to_finish_swing = 0.f;
	glm::mat4 modelMatrix;
	glm::vec2 tex_coords[4];
	float tex_id = 0.f;
	WeaponType weapon_type;
	Hitbox_2D_OBB hitbox;
};

//struct that is used for breaking different objects
struct ActiveBreakableObject {
	void start_breaking_object(int column, int line, float time_to_break) {
		isBreaking = true;
		this->column = column;
		this->line = line;
		time_breaking = 0.f;
		this->time_to_break = time_to_break;
	}
	bool update_breaking_object(int column, int line, float deltaTime) {
		if (column != this->column || line != this->line) {
			start_breaking_object(column, line, this->time_to_break);
			return false;
		}
		time_breaking += deltaTime;
		if (time_breaking >= time_to_break) {
			isBreaking = false;
			return true;
		}
		return false;
	}
	bool isBreaking = false;
	int column, line;
	float time_to_break = 1.0f; //default
	float time_breaking = 0.5f;
};

enum CraftingCondition { Nothing, Workbench, Furnace, Anvil, Table };
struct CraftingPair {
	int id = 0, amount = 0;
};
struct CraftableItem {
	CraftableItem() {}
	CraftableItem(int item_id, int amount, CraftingCondition condition, std::vector<CraftingPair> items_needed) :
		item_id{ item_id }, craftable_amount{ amount }, condition{ condition }, items_needed{ items_needed } {}
	int item_id = 0; //id of item that is craftable
	int craftable_amount = 0; //amount of this item per one craft
	std::vector<CraftingPair> items_needed;
	CraftingCondition condition = Nothing; //specific condition (be near the workbench, etc.)
};

struct CraftingSystem {
	bool is_near_workbench = false;
	bool is_near_furnace = false;
	bool is_near_anvil = false;
	std::vector<int> available_crafts; //contains all crafts available at the moment
	bool show_all_crafts = false; //show all crafts available in slots (more comfortable)
	ColorVertex2f helper_slots[70 * 4]; //maximum 70 slots
	ColorVertex2f main_slots[5 * 4]; //main array where the first is crafting slot, where item can be taken(crafted), other just for other available crafts(near the item in first slot in available_crafts)
	ColorVertex2f needed_items_slots[10 * 4]; //max is currently 10
	int index_of_current_craft = 0; //index of craft that is currently in the first main slot
};