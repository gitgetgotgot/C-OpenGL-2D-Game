#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <memory>

enum EntityType { isEnemy, isNPC, isMob, isPet, isProjectile };
enum EnemyType { isSlime, isZombie, isFlyingEye };
enum ProjectileType { Arrow, Bullet, Throwable, MagicBall };
enum EntityMovementType { isWalking, isSwimming, isFlying };

//structure that contains information about drop from entity
struct DropInfo {
	int id = 0;
	float chance = 0.f;
	//amount of this item in range [amount1, amount2]
	int amount1 = 0;
	int amount2 = 0;
	DropInfo(int id, float chance, int amount) : id{ id }, chance{ chance }, amount1{ amount }, amount2{ amount } {}
	DropInfo(int id, float chance, int amount1, int amount2) : id{ id }, chance{ chance }, amount1{ amount1 }, amount2{ amount2 } {}
};

//structure that has info about appliable effect
struct AppliableEffect {
	float duration = 0.f;
	int id = -1;
	AppliableEffect() {}
	AppliableEffect(int id, float duration) : id{ id }, duration{ duration } {}
};

//structure that has info about appliable lighting
struct AppliableLight {
	glm::vec3 light_color{ 0.f, 0.f, 0.f };
	float light_radius = 0.f; //in blocks
	AppliableLight() {}
	AppliableLight(float radius, glm::vec3 color) : light_radius{ radius }, light_color{ color } {}
};

//particles
struct ParticleInfo {
	glm::vec2 tex_coords[4];
	AppliableLight light;
	bool emitsLight = false;
};

class Particle {
public:
	Particle(int id, float lifeTime, glm::vec2 sprite_center, glm::vec2 sprite_size, glm::vec2 velocity) :
		id{ id }, lifeTime{ lifeTime }, sprite_center{ sprite_center }, sprite_size{ sprite_size }, velocity{ velocity } {}
	bool update(float deltaTime) {
		currentTime += deltaTime;
		if (currentTime >= lifeTime) {
			return true; //return true if particle should be destroyed
		}
		sprite_center.x += velocity.x;
		sprite_center.y += velocity.y;
		sprite_size *= 0.99;
		angle += 180.f * deltaTime;
		glm::mat4 matModel(1.f);
		matModel = glm::translate(matModel, glm::vec3(sprite_center.x, sprite_center.y, 0.f));
		matModel = glm::rotate(matModel, angle, glm::vec3(0.f, 0.f, 1.f));
		matModel = glm::translate(matModel, glm::vec3(-sprite_size.x * 0.5, -sprite_size.y * 0.5, 0.f));
		matModel = glm::scale(matModel, glm::vec3(sprite_size.x, sprite_size.y, 0.f));
		modelMatrix = matModel;
		return false;
	}
	int id;
	float lifeTime, currentTime = 0.f, angle = 0.f;
	glm::vec2 sprite_center;
	glm::vec2 sprite_size;
	glm::vec2 velocity;
	glm::mat4 modelMatrix;
};
//class ParticleSystem {
//public:
//	ParticleSystem() {}
//	ParticleSystem(float spawnInterval) : spawnInterval{ spawnInterval } {}
//	virtual void updateParticles(glm::vec2 emitter_ld_corner, glm::vec2 emitter_size, float deltaTime, float block_size) {}
//	std::vector<Particle> particles;
//	float spawnInterval;
//	float currentTime = 0.f;
//};
//class FireParticleSystem : public ParticleSystem {
//public:
//	FireParticleSystem() : ParticleSystem(0.2f) {}
//	void updateParticles(glm::vec2 emitter_ld_corner, glm::vec2 emitter_size, float deltaTime, float block_size) override {
//		currentTime += deltaTime;
//		for (int i = 0; i < particles.size(); i++) {
//			particles[i].currentTime += deltaTime;
//			if (particles[i].currentTime >= particles[i].lifeTime) {
//				particles.erase(particles.begin() + i);
//				i--;
//				continue;
//			}
//			particles[i].sprite_center.x += particles[i].velocity.x;
//			particles[i].sprite_center.y += particles[i].velocity.y;
//			particles[i].sprite_size *= 0.99;
//			glm::mat4 matModel(1.f);
//			matModel = glm::translate(matModel, glm::vec3(particles[i].sprite_center.x, particles[i].sprite_center.y, 0.f));
//			//matModel = glm::rotate(matModel, 0.f, glm::vec3(0.f, 0.f, 1.f));
//			matModel = glm::translate(matModel, glm::vec3(-particles[i].sprite_size.x * 0.5, -particles[i].sprite_size.y * 0.5, 0.f));
//			matModel = glm::scale(matModel, glm::vec3(particles[i].sprite_size.x, particles[i].sprite_size.y, 0.f));
//			particles[i].modelMatrix = matModel;
//		}
//		if (currentTime >= spawnInterval) {
//			currentTime = 0.f;
//			glm::vec2 center;
//			center.x = emitter_ld_corner.x;
//			center.y = emitter_ld_corner.y;
//			particles.emplace_back(Particle(0, 2.f, center, glm::vec2(block_size * 0.5, block_size * 0.5), glm::vec2(block_size * 0.05, block_size * 0.05)));
//		}
//	}
//};

class EntityInfo {
public:
	EntityInfo(EntityType type, std::string name, glm::vec2 hitboxSize, glm::vec2 spriteSize) :
		type{ type }, name{ name }, hitboxSize{ hitboxSize }, spriteSize{ spriteSize } {}
	~EntityInfo() {}
	//enemies
	virtual int get_HP() const { return 0; }
	virtual int get_dmg() const { return 0; }
	virtual int get_defense() const { return 0; }
	virtual float get_jump_V0() const { return 0; }
	virtual float get_walk_V() const { return 0; }
	virtual float get_moving_X_V() const { return 0; }
	virtual float get_moving_Y_V() const { return 0; }
	virtual EnemyType get_enemy_type() { return isSlime; }
	virtual std::vector<DropInfo>& get_drop_v() { std::vector<DropInfo> d; return d; }
	//projectiles
	virtual int get_ammo_item_id() const { return -1; }
	virtual glm::vec2* get_tex_coords_ptr() { return nullptr; }
	virtual int get_proj_max_enemy_hits() const { return 0; }
	virtual bool proj_uses_gravity() const { return false; }
	EntityType type;
	std::string name;
	glm::vec2 hitboxSize;
	glm::vec2 spriteSize;
	float tex_slot_id = 0.f;
	//light
	bool emitsLight = false;
	AppliableLight light;
	//sounds
	bool does_sounds = false;
	//effects
	bool hasEffect = false;
	AppliableEffect effect;
};
///ENEMIES
class EnemyInfo : public EntityInfo {
public:
	EnemyInfo(EnemyType type, std::string name, int HP, int dmg, int defense, glm::vec2 hitboxSize, glm::vec2 spriteSize) :
		EntityInfo(isEnemy, name, hitboxSize, spriteSize), type{ type }, HP{ HP }, damage{ dmg }, defense{ defense } {}
	int get_HP() const override { return HP; }
	int get_dmg() const override { return damage; }
	int get_defense() const override { return defense; }
	EnemyType get_enemy_type() override { return type; }
	std::vector<DropInfo>& get_drop_v() override { return drop_items; }
private:
	int HP, damage, defense;
	EnemyType type;
	std::vector<DropInfo> drop_items;
};
///3 main types of enemies
class WalkingEnemyInfo : public EnemyInfo {
public:
	WalkingEnemyInfo(EnemyType type, std::string name, int HP, int dmg, int defense, float jump_V0, float walk_V, glm::vec2 hitboxSize, glm::vec2 spriteSize) :
		EnemyInfo(type, name, HP, dmg, defense, hitboxSize, spriteSize), jump_V0{ jump_V0 }, walk_V{ walk_V } {}
	float get_jump_V0() const { return jump_V0; }
	float get_walk_V() const { return walk_V; }
private:
	float jump_V0, walk_V;
};

class FlyingEnemyInfo : public EnemyInfo {
public:
	FlyingEnemyInfo(EnemyType type, std::string name, int HP, int dmg, int defense, float flyingX_V, float flyingY_V, glm::vec2 hitboxSize, glm::vec2 spriteSize) :
		EnemyInfo(type, name, HP, dmg, defense, hitboxSize, spriteSize), flyingX_V{ flyingX_V }, flyingY_V{ flyingY_V } {}
	float get_moving_X_V() const override { return flyingX_V; }
	float get_moving_Y_V() const override { return flyingY_V; }
private:
	float flyingX_V, flyingY_V;
};

class WaterEnemyInfo : public EnemyInfo {

};
///walking enemies info
class SlimeInfo : public WalkingEnemyInfo {
public:
	SlimeInfo(std::string name, int HP, int dmg, int defense, float jump_V0, float walk_V, glm::vec2 hitboxSize, glm::vec2 spriteSize, bool does_sounds) :
		WalkingEnemyInfo(isSlime, name, HP, dmg, defense, jump_V0, walk_V, hitboxSize, spriteSize) {
		this->does_sounds = does_sounds;
	}
	glm::vec2* get_tex_coords_ptr() override { return tex_coords; }
private:
	glm::vec2 tex_coords[8]{}; //2 sprites for all slimes probably
};

class ZombieInfo : public WalkingEnemyInfo {
public:
	ZombieInfo(std::string name, int HP, int dmg, int defense, float jump_V0, float walk_V, glm::vec2 hitboxSize, glm::vec2 spriteSize, bool does_sounds) :
		WalkingEnemyInfo(isZombie, name, HP, dmg, defense, jump_V0, walk_V, hitboxSize, spriteSize) {
		this->does_sounds = does_sounds;
	}
	glm::vec2* get_tex_coords_ptr() override { return tex_coords; }
private:
	glm::vec2 tex_coords[24]{}; //6 sprites for all zombies probably
};
///
///flying enemies info
class FlyingEyeInfo : public FlyingEnemyInfo {
public:
	FlyingEyeInfo(std::string name, int HP, int dmg, int defense, float flyingX_V, float flyingY_V, glm::vec2 hitboxSize, glm::vec2 spriteSize, bool does_sounds) :
		FlyingEnemyInfo(isFlyingEye, name, HP, dmg, defense, flyingX_V, flyingY_V, hitboxSize, spriteSize) {
		this->does_sounds = does_sounds;
	}
	glm::vec2* get_tex_coords_ptr() override { return tex_coords; }
private:
	glm::vec2 tex_coords[8]{}; //2 sprites for all zombies probably
};
///
class NPC_Info : public EntityInfo {
public:

private:
	int damage;
};

class MobInfo : public EntityInfo {
public:
	int HP;
private:
	int damage;
};

class PetInfo : public EntityInfo {
public:

private:

};

class ProjectileInfo : public EntityInfo {
public:
	ProjectileInfo(ProjectileType type, int ammo_item_id, int damage, int max_enemy_hits, bool usesGravity, glm::vec2 spriteSize, bool hasEffect, bool hasLight) :
		EntityInfo(isProjectile, "", glm::vec2(0.f, 0.f), spriteSize), type{ type }, ammo_item_id{ ammo_item_id }, DMG{ damage }, max_enemy_hits{ max_enemy_hits }, usesGravity{ usesGravity } {
		this->emitsLight = hasLight; this->hasEffect = hasEffect; this->spriteSize = spriteSize;
	}
	ProjectileInfo(ProjectileType type, int ammo_item_id, int damage, int max_enemy_hits, bool usesGravity, glm::vec2 spriteSize, bool hasEffect, bool hasLight, AppliableEffect effect) :
		EntityInfo(isProjectile, "", glm::vec2(0.f, 0.f), spriteSize), type{ type }, ammo_item_id{ ammo_item_id }, DMG{ damage }, max_enemy_hits{ max_enemy_hits }, usesGravity{ usesGravity } {
		this->emitsLight = hasLight; this->hasEffect = hasEffect; this->spriteSize = spriteSize;
		this->effect = effect;
	}
	ProjectileInfo(ProjectileType type, int ammo_item_id, int damage, int max_enemy_hits, bool usesGravity, glm::vec2 spriteSize, bool hasEffect, bool hasLight, AppliableLight light) :
		EntityInfo(isProjectile, "", glm::vec2(0.f, 0.f), spriteSize), type{ type }, ammo_item_id{ ammo_item_id }, DMG{ damage }, max_enemy_hits{ max_enemy_hits }, usesGravity{ usesGravity } {
		this->emitsLight = hasLight; this->hasEffect = hasEffect; this->spriteSize = spriteSize;
		this->light = light;
	}
	ProjectileInfo(ProjectileType type, int ammo_item_id, int damage, int max_enemy_hits, bool usesGravity, glm::vec2 spriteSize, bool hasEffect, bool hasLight, AppliableEffect effect, AppliableLight light) :
		EntityInfo(isProjectile, "", glm::vec2(0.f, 0.f), spriteSize), type{ type }, ammo_item_id{ ammo_item_id }, DMG{ damage }, max_enemy_hits{ max_enemy_hits }, usesGravity{ usesGravity } {
		this->emitsLight = hasLight; this->hasEffect = hasEffect; this->spriteSize = spriteSize;
		this->effect = effect; this->light = light;
	}
	int get_ammo_item_id() const override { return ammo_item_id; }
	glm::vec2* get_tex_coords_ptr() override { return tex_coords; }
	int get_dmg() const override { return DMG; }
	int get_proj_max_enemy_hits() const override { return max_enemy_hits; }
	bool proj_uses_gravity() const override { return usesGravity; }
private:
	ProjectileType type;
	int ammo_item_id; //if projectile can be an item
	int DMG;
	float enemy_hit_cd = 0.5f;
	int max_enemy_hits;
	bool usesGravity;
	glm::vec2 tex_coords[4];
};

struct WalkingEnemyPhysics {
	bool has_bottom_collision = false;
	bool has_side_collision = false;
	bool has_top_collision = false;
	bool moving_down = false;
	bool has_bottom_collision_only_with_objects = false; //check if entity has bottom collision with objects that have "bottom collision only"
	bool should_jump = false;
	float fallingDistance = 0.f; //to calculate the increment for moving on Y axis
	float time_in_free_falling = 0.f;
	float jump_V0, current_jump_V = 0.f;
	float walk_V;
	float Xinc = 0.f;
	int current_Y_max_level; //Y level of current block layer, where the entity is standing or stopped going up from jump
};
struct FlyingEnemyPhysics {
	bool has_bottom_collision = false;
	bool has_side_collision = false;
	bool has_top_collision = false;
	bool moving_down = false;
	float flyingX_V, flyingY_V;
	float Xinc = 0.f, Yinc = 0.f;
};

enum EffectType { isBuff, isDebuff, isDamagingDebuff, isImmediateBuff, isHealing };
struct Effect {
	float duration;
	float current_duration = 0.f;
	float delta_dmg_time = 0.f; //delta time that is used for debuffs with damage
	int id;
	Effect(float duration, int id) : duration{ duration }, id{ id } {}
	bool updateEffect(float deltaTime) {
		current_duration += deltaTime;
		delta_dmg_time += deltaTime;
		//return true if effect should be removed
		if (current_duration >= duration) return true;
		else return false;
	}
};

struct EntityStats {
	std::vector<Effect> effects;
	int HP = 100;
	int DEF = 0;
	float speedFactor = 1.f;
	float hit_cd = 0.f; //cd after receiving hit from melee weapon
};

class EntityBase {
public:
	EntityBase(int entity_id) : entity_id{ entity_id } {}
	~EntityBase() {}
	virtual void update_entity(float deltaTime, float block_size, float playerX, float playerY) = 0;
	virtual void update_model(float block_size) = 0;
	virtual void do_entity_sounds(Smart_ptr<AudioManager>& audioManager) {}
	virtual void do_entity_hit_sound(Smart_ptr<AudioManager>& audioManager) {}
	virtual void do_entity_death_sound(Smart_ptr<AudioManager>& audioManager) {}

	virtual bool update_proj_hits_counter() { return true; }

	virtual WalkingEnemyPhysics& get_walking_physics() { WalkingEnemyPhysics p; return p; }
	virtual FlyingEnemyPhysics& get_flying_physics() { FlyingEnemyPhysics p; return p; }
	virtual int get_HP() const { return 0; }
	virtual void decrease_HP(int value) {};
	virtual int get_tex_index() const { return 0; }
	virtual int get_proj_dmg() const { return 0; }
	virtual bool dmg_is_crit() const { return false; }
	virtual EntityMovementType get_movement_type() const { return isWalking; }
	virtual EntityStats& get_entity_stats() { EntityStats s; return s; }
	int entity_id;
	bool looks_at_left = false;
	glm::mat4 matModel;
	glm::vec2 sprite_center_point;
	glm::vec2 sprite_size;
	Hitbox_2D_AABB hitbox;
};

class LivingEntity : public EntityBase {
public:
	LivingEntity(int entity_id, float spawn_center_x, float spawn_center_y, int HP, int DEF, glm::vec2 hitboxSize, glm::vec2 spriteSize) :
		EntityBase(entity_id) {
		stats.HP = HP; stats.DEF = DEF;
		sprite_size = spriteSize;
		hitbox.center = { spawn_center_x, spawn_center_y };
		hitbox.size = hitboxSize;
	}
	int get_HP() const override { return stats.HP; }
	void decrease_HP(int value) override { stats.HP -= value; }
	EntityStats& get_entity_stats() override { return stats; }
	void update_hit_cd(float deltaTime) {
		if (stats.hit_cd > 0.f) {
			stats.hit_cd -= deltaTime;
			if (stats.hit_cd <= 0.f)
				stats.hit_cd = 0.f;
		}
	}
protected:
	EntityStats stats;
};
class WalkingEntity : public LivingEntity {
public:
	WalkingEntity(int entity_id, float spawn_center_x, float spawn_center_y, int HP, int DEF, glm::vec2 hitboxSize, glm::vec2 spriteSize) :
		LivingEntity(entity_id, spawn_center_x, spawn_center_y, HP, DEF, hitboxSize, spriteSize) {}
	WalkingEnemyPhysics& get_walking_physics() override { return physics; }
	EntityMovementType get_movement_type() const override { return EntityMovementType::isWalking; }
protected:
	WalkingEnemyPhysics physics;
};
class FlyingEntity : public LivingEntity {
public:
	FlyingEntity(int entity_id, float spawn_center_x, float spawn_center_y, int HP, int DEF, glm::vec2 hitboxSize, glm::vec2 spriteSize) :
		LivingEntity(entity_id, spawn_center_x, spawn_center_y, HP, DEF, hitboxSize, spriteSize) {}
	FlyingEnemyPhysics& get_flying_physics() override { return physics; }
	EntityMovementType get_movement_type() const override { return EntityMovementType::isFlying; }
protected:
	FlyingEnemyPhysics physics;
};

class Slime : public WalkingEntity {
public:
	Slime(int entity_id, float spawn_center_x, float spawn_center_y, EntityInfo& slimeInfo, float block_size) :
		WalkingEntity(entity_id, spawn_center_x, spawn_center_y, slimeInfo.get_HP(), slimeInfo.get_defense(), slimeInfo.hitboxSize * block_size, slimeInfo.spriteSize * block_size) {
		physics.jump_V0 = slimeInfo.get_jump_V0();
		physics.walk_V = slimeInfo.get_walk_V();
		physics.current_Y_max_level = hitbox.center.y - hitbox.size.y * 0.5f; //mmm
	}
	void update_entity(float deltaTime, float block_size, float playerX, float playerY) override {
		if (physics.has_bottom_collision || physics.has_bottom_collision_only_with_objects) {
			time_standing += deltaTime;
			physics.Xinc = 0.f;
		}
		update_hit_cd(deltaTime);
		sprite_time += deltaTime;
		//change sprite every half second
		if (sprite_time >= 0.25f) {
			sprite_time = 0.f;
			current_sprite++;
			if (current_sprite > 1)
				current_sprite = 0;
		}
		if (physics.time_in_free_falling > 0.f)
			current_sprite = 1;
		//make jump after standing for 5 seconds
		if (time_standing >= 5.f) {
			time_standing = 0.f;
			physics.should_jump = true;
			if (playerX - hitbox.center.x > 0.f)
				physics.Xinc = physics.walk_V * block_size * deltaTime;
			else
				physics.Xinc = -physics.walk_V * block_size * deltaTime;
		}
	}
	void do_entity_hit_sound(Smart_ptr<AudioManager>& audioManager) {
		audioManager->play_Sound(6);
	}
	void do_entity_death_sound(Smart_ptr<AudioManager>& audioManager) {
		audioManager->play_Sound(7);
	}
	void update_model(float block_size) override {
		sprite_center_point.x = hitbox.center.x;
		sprite_center_point.y = hitbox.center.y - hitbox.size.y * 0.5f + sprite_size.y * 0.5f;
		matModel = glm::translate(glm::mat4(1.f), glm::vec3(sprite_center_point.x - sprite_size.x * 0.5f, sprite_center_point.y - sprite_size.y * 0.5f, 0.f));
		matModel = glm::scale(matModel, glm::vec3(sprite_size.x, sprite_size.y, 0.f));
	}
	int get_tex_index() const override { 
		return current_sprite;
	}
private:
	float sprite_time = 0.f;
	float time_standing = 0.f; //time when slime is not moving
	int current_sprite = 0; //current sprite is first or second
};
class Zombie : public WalkingEntity {
public:
	Zombie(int entity_id, float spawn_center_x, float spawn_center_y, EntityInfo& zombieInfo, float block_size) :
		WalkingEntity(entity_id, spawn_center_x, spawn_center_y, zombieInfo.get_HP(), zombieInfo.get_defense(), zombieInfo.hitboxSize * block_size, zombieInfo.spriteSize * block_size) {
		physics.jump_V0 = zombieInfo.get_jump_V0();
		physics.walk_V = zombieInfo.get_walk_V();
		physics.current_Y_max_level = hitbox.center.y - hitbox.size.y * 0.5f; //mmm
		physics.Xinc = physics.walk_V;
	}
	void update_entity(float deltaTime, float block_size, float playerX, float playerY) override {
		//manage zombie sprites
		sprite_time += deltaTime;
		sounds_cd_time += deltaTime;
		update_hit_cd(deltaTime);
		if (physics.has_bottom_collision || physics.has_bottom_collision_only_with_objects) {
			if (physics.Xinc != 0.f) {
				if (current_sprite == 0)
					current_sprite = 2;
				else if (sprite_time >= 0.2f) {
					sprite_time = 0.f;
					current_sprite++;
					if (current_sprite > 5)
						current_sprite = 2;
				}
			}
			else
				current_sprite = 0;
		}
		else {
			current_sprite = 1;
		}
		//try to jump if current Xinc is 0, meaning that there is no movement
		if (physics.has_side_collision && !physics.has_top_collision && (physics.has_bottom_collision || physics.has_bottom_collision_only_with_objects)) {
			time_standing = 0.f;
			physics.should_jump = true;
		}
		//Xinc
		if (playerX + sign * block_size * 0.75 - hitbox.center.x > 0.f) {
			if (!player_right_side) {
				player_right_side = !player_right_side;
				sign = 1;
			}
			physics.Xinc = physics.walk_V * block_size * deltaTime;
			looks_at_left = false;
		}
		else {
			if (player_right_side) {
				player_right_side = !player_right_side;
				sign = -1;
			}
			physics.Xinc = -physics.walk_V * block_size * deltaTime;
			looks_at_left = true;
		}
	}
	void do_entity_hit_sound(Smart_ptr<AudioManager>& audioManager) {
		audioManager->play_Sound(6);
	}
	void do_entity_death_sound(Smart_ptr<AudioManager>& audioManager) {
		audioManager->play_Sound(8);
	}
	void do_entity_sounds(Smart_ptr<AudioManager>& audioManager) {
		if (sounds_cd_time >= 5.f) {
			sounds_cd_time = 0.f;
			audioManager->play_Sound(9 + rand() % 2);
		}
	}
	void update_model(float block_size) override {
		sprite_center_point.x = hitbox.center.x;
		sprite_center_point.y = hitbox.center.y - hitbox.size.y * 0.5f + sprite_size.y * 0.5f;
		matModel = glm::translate(glm::mat4(1.f), glm::vec3(sprite_center_point.x - sprite_size.x * 0.5f, sprite_center_point.y - sprite_size.y * 0.5f, 0.f));
		matModel = glm::scale(matModel, glm::vec3(sprite_size.x, sprite_size.y, 0.f));
	}
	int get_tex_index() const override {
		return current_sprite;
	}
private:
	float sprite_time = 0.f;
	float sounds_cd_time = 0.f; //cooldown for making sounds
	float time_standing = 0.f; //time when zombie is not moving right or left
	int current_sprite = 0; //current sprite of zombie
	bool player_right_side = false; //goes to the right or left side of player (used to prevent zombie from moving right and left on one place when at the same X position as player)
	int sign = -1;
};

class FlyingEye : public FlyingEntity {
public:
	FlyingEye(int entity_id, float spawn_center_x, float spawn_center_y, EntityInfo& eyeInfo, float block_size) :
		FlyingEntity(entity_id, spawn_center_x, spawn_center_y, eyeInfo.get_HP(), eyeInfo.get_defense(), eyeInfo.hitboxSize * block_size, eyeInfo.spriteSize * block_size) {
		physics.flyingX_V = eyeInfo.get_moving_X_V();
		physics.flyingY_V = eyeInfo.get_moving_Y_V();
	}
	void update_entity(float deltaTime, float block_size, float playerX, float playerY) override {
		sprite_time += deltaTime;
		update_hit_cd(deltaTime);
		if (sprite_time >= 0.25f) {
			sprite_time = 0.f;
			current_sprite++;
			if (current_sprite > 1)
				current_sprite = 0;
		}
		//Xinc
		if (playerX - hitbox.center.x > 0.f) {
			physics.Xinc += physics.flyingX_V * block_size * deltaTime * 0.01;
			if (physics.Xinc > physics.flyingX_V * block_size * deltaTime) physics.Xinc = physics.flyingX_V * block_size * deltaTime;
		}
		else {
			physics.Xinc -= physics.flyingX_V * block_size * deltaTime * 0.01;
			if (physics.Xinc < -physics.flyingX_V * block_size * deltaTime) physics.Xinc = -physics.flyingX_V * block_size * deltaTime;
		}
		//Yinc
		if (playerY - hitbox.center.y > 0.f) {
			physics.Yinc += physics.flyingY_V * block_size * deltaTime * 0.01;
			if (physics.Yinc > physics.flyingY_V * block_size * deltaTime) physics.Yinc = physics.flyingY_V * block_size * deltaTime;
		}
		else {
			physics.Yinc -= physics.flyingY_V * block_size * deltaTime * 0.01;
			if (physics.Yinc < -physics.flyingY_V * block_size * deltaTime) physics.Yinc = -physics.flyingY_V * block_size * deltaTime;
		}
	}
	void do_entity_hit_sound(Smart_ptr<AudioManager>& audioManager) {
		audioManager->play_Sound(6);
	}
	void do_entity_death_sound(Smart_ptr<AudioManager>& audioManager) {
		audioManager->play_Sound(7);
	}
	void update_model(float block_size) override {
		hitbox.size = { block_size, block_size };
		//here sprite_center_point is used as center point for matModel
		if (physics.Xinc == 0)
			flying_angle = 0;
		else
			flying_angle = std::atan(physics.Yinc / physics.Xinc);
		if (physics.Xinc > 0.f)
			flying_angle += glm::radians(180.f);

		matModel = glm::mat4(1.f);
		matModel = glm::translate(matModel, glm::vec3(hitbox.center.x, hitbox.center.y, 0.f));
		matModel = glm::rotate(matModel, flying_angle, glm::vec3(0.f, 0.f, 1.f));
		matModel = glm::translate(matModel, glm::vec3(-sprite_size.y * 0.75f, -sprite_size.y * 0.5f, 0.f));
		matModel = glm::scale(matModel, glm::vec3(sprite_size.x, sprite_size.y, 0.f));
	}
	int get_tex_index() const override {
		return current_sprite;
	}
private:
	float sprite_time = 0.f;
	int current_sprite = 0; //current sprite of eye
	float flying_angle;
};

class GravityProjectile : public EntityBase {
public:
	GravityProjectile(int entity_id, float speedX, float speedY, float flying_angle, float center_x, float center_y, glm::vec2 spriteSize, int DMG, int max_enemy_hits, bool isCrit, float spriteAdjustmentAngle) :
		EntityBase(entity_id), speedX{ speedX }, speedY{ speedY }, flying_angle{ flying_angle }, DMG{ DMG }, isCrit{ isCrit }, max_enemy_hits_available{ max_enemy_hits }, sprite_adjustment_angle{ spriteAdjustmentAngle } {
		hitbox.center = { center_x, center_y };
		sprite_center_point = hitbox.center;
		this->sprite_size = spriteSize;
		if (spriteAdjustmentAngle > 0.f) Yadjustment = 0.2f;
		else Yadjustment = 0.8f;
	}
	void update_entity(float deltaTime, float block_size, float playerX, float playerY) override {
		float dx, dy;
		//calculate dX
		dx = speedX * deltaTime;
		sprite_center_point.x += dx;
		//calculate current y and dY
		float currentY = sprite_center_point.y + speedY * deltaTime - 9.8f * (block_size / 40.f) * time_of_flight * time_of_flight / 2.f;
		dy = currentY - sprite_center_point.y;
		sprite_center_point.y = currentY;
		time_of_flight += deltaTime;
		//calculate new angle of arrow
		flying_angle = std::atan(dy / dx) + glm::radians(sprite_adjustment_angle);
		if (dx < 0.f)
			flying_angle += glm::radians(180.f);
		//sprite model matrix
		matModel = glm::mat4(1.f);
		matModel = glm::translate(matModel, glm::vec3(sprite_center_point.x, sprite_center_point.y, 0.f));
		matModel = glm::rotate(matModel, flying_angle, glm::vec3(0.f, 0.f, 1.f));
		matModel = glm::translate(matModel, glm::vec3(-block_size * 0.5f, -block_size * Yadjustment, 0.f));
		matModel = glm::scale(matModel, glm::vec3(block_size, block_size, 0.f));
		//hitbox
		hitbox.size = { block_size * sprite_size.x * 0.3f, block_size * sprite_size.x * 0.3f };
		hitbox.center = sprite_center_point;
	}
	void update_model(float block_size) override {}
	int get_proj_dmg() const override { return DMG; }
	bool dmg_is_crit() const override { return isCrit; }
	bool update_proj_hits_counter() override {
		max_enemy_hits_available--;
		if (max_enemy_hits_available == 0) return false;
		return true;
	}
private:
	float speedX, speedY;
	float time_of_flight = 0.f;
	float flying_angle;
	int DMG;
	bool isCrit;
	int max_enemy_hits_available;
	float sprite_adjustment_angle;
	float Yadjustment;
};

class LinearProjectile : public EntityBase {
public:
	LinearProjectile(int entity_id, float speedX, float speedY, float flying_angle, float center_x, float center_y, glm::vec2 spriteSize, int DMG, int max_enemy_hits, bool isCrit) :
		EntityBase(entity_id), speedX{ speedX }, speedY{ speedY }, flying_angle{ flying_angle }, DMG{ DMG }, isCrit{ isCrit }, max_enemy_hits_available{ max_enemy_hits } {
		hitbox.center = { center_x, center_y };
		sprite_center_point = hitbox.center;
		this->sprite_size = spriteSize;
	}
	void update_entity(float deltaTime, float block_size, float playerX, float playerY) override {
		//sprite
		hitbox.center.x += speedX * deltaTime;
		hitbox.center.y += speedY * deltaTime;
		sprite_center_point = hitbox.center;
		matModel = glm::mat4(1.f);
		matModel = glm::translate(matModel, glm::vec3(hitbox.center.x, hitbox.center.y, 0.f));
		matModel = glm::rotate(matModel, flying_angle, glm::vec3(0.f, 0.f, 1.f));
		matModel = glm::translate(matModel, glm::vec3(-block_size * sprite_size.x / 2, -block_size * sprite_size.y / 2, 0.f));
		matModel = glm::scale(matModel, glm::vec3(block_size * sprite_size.x, block_size * sprite_size.y, 0.f));
		//hitbox
		hitbox.size = { block_size * sprite_size.x * 0.3f, block_size * sprite_size.x * 0.3f };
	}
	void update_model(float block_size) override {}
	int get_proj_dmg() const override { return DMG; }
	bool dmg_is_crit() const override { return isCrit; }
	bool update_proj_hits_counter() override {
		max_enemy_hits_available--;
		if (max_enemy_hits_available == 0) return false;
		return true;
	}
private:
	float speedX, speedY;
	float flying_angle;
	int DMG;
	bool isCrit;
	int max_enemy_hits_available;
};

struct PlayerStats {
	int HP = 100;
	int MANA = 20;
	int DEF = 0;
	int currentHP = 0;
	int currentMANA = 20;
	float speedFactor = 1.f;
	float damageMultiplier = 1.f;
	float critChanceBuff = 0.f;
	float regeneration = 1.f;
	float damageReduction = 0.f; //in percentage
	bool hasPotionSickness = false;
};
struct Player {
	//effects
	std::vector<Effect> effects;
	//stats
	PlayerStats stats;
	//hitbox and physics
	Hitbox_2D_AABB hitbox;
	glm::vec2 sprite_left_down_corner;
	glm::vec2 sprite_size;
	bool looks_at_left = true;
	bool has_bottom_collision = false;
	bool has_side_collision = false;
	bool has_top_collision = false;
	bool moving_down = false;
	bool has_bottom_collision_only_with_objects = false; //check if the player has bottom collision with objects that have "bottom collision only"
	float fallingDistance = 0.f; //to calculate the increment for moving on Y axis
	float time_in_free_falling = 0.f; //time spent while moving after jump
	float jump_V0 = 0.f; //the starting speed of jump
	float speed = 200.f; //standard speed - 5 blocks per second
	float jump_speed = 600.f; //standard jump speed
	int current_Y_max_level; //Y of current block layer, where the player is standing or stopped going up from jump
	//sprite
	glm::vec2 tex_coords[24]; //6 sprites test
	int current_sprite = 0;
	float sprite_time = 0.f;
};

namespace GameEffects {
	class EffectBase {
	public:
		bool emitsParticles = false;
		EffectType type;
		std::string name;
		glm::vec2 sprite_coords[4]; //sprite that is used in player's UI
		EffectBase(EffectType type, std::string name, bool emitsParticles) : type{ type }, name{ name }, emitsParticles{ emitsParticles } {}
		~EffectBase() {}
		virtual bool applyEffect(PlayerStats& stats) { return false; }
		virtual bool inflictDamage(PlayerStats& stats, float& deltaDMGTime) { return false; }
		virtual void removeEffect(PlayerStats& stats) {}
		virtual bool applyEntityEffect(EntityStats& stats) { return false; }
		virtual bool inflictEntityDamage(EntityStats& stats, float& deltaDMGTime) { return false; }
		virtual void removeEntityEffect(EntityStats& stats) {}
		virtual bool emit_particle(std::vector<Particle>& particles, glm::vec2 emitter_center, glm::vec2 emitter_size, float deltaTime, float block_size) { return false; }
	};

	class IncHP_LifeCrystal : public EffectBase {
	public:
		IncHP_LifeCrystal() : EffectBase(EffectType::isImmediateBuff, "", false) {}
		bool applyEffect(PlayerStats& stats) override {
			if (stats.HP < 400) {
				stats.HP += 20;
				stats.currentHP += 20;
				return true;
			}
			return false;
		}
	};
	class IncMANA_ManaCrystal : public EffectBase {
	public:
		IncMANA_ManaCrystal() : EffectBase(EffectType::isImmediateBuff, "", false) {}
		bool applyEffect(PlayerStats& stats) override {
			if (stats.MANA < 200) {
				stats.MANA += 20;
				stats.currentMANA += 20;
				return true;
			}
			return false;
		}
	};
	class IncreaseDefense : public EffectBase {
	public:
		IncreaseDefense() : EffectBase(EffectType::isBuff, "DEF increased", false) {}
		bool applyEffect(PlayerStats& stats) override {
			stats.DEF += 10;
			return true;
		}
		void removeEffect(PlayerStats& stats) {
			stats.DEF -= 10;
		}
	};
	class IncreaseSpeed : public EffectBase {
	public:
		IncreaseSpeed() : EffectBase(EffectType::isBuff, "Speed increased", false) {}
		bool applyEffect(PlayerStats& stats) override {
			stats.speedFactor = 1.25f;
			return true;
		}
		void removeEffect(PlayerStats& stats) {
			stats.speedFactor = 1.f;
		}
	};
	class IncreaseRegeneration : public EffectBase {
	public:
		IncreaseRegeneration() : EffectBase(EffectType::isBuff, "Regeneration", false) {}
		bool applyEffect(PlayerStats& stats) override {
			stats.regeneration = 1.5f;
			return true;
		}
		void removeEffect(PlayerStats& stats) {
			stats.regeneration = 1.f;
		}
	};
	class Burning : public EffectBase {
	public:
		Burning() : EffectBase(EffectType::isDamagingDebuff, "Burning!", true) {}
		bool inflictDamage(PlayerStats& stats, float& deltaDMGTime) override {
			if (deltaDMGTime >= 0.25) {
				deltaDMGTime = 0.f;
				stats.currentHP -= 2;
				return true;
			}
			return false;
		}
		bool inflictEntityDamage(EntityStats& stats, float& deltaDMGTime) {
			if (deltaDMGTime >= 0.25) {
				deltaDMGTime = 0.f;
				stats.HP -= 2;
				return true;
			}
			return false;
		}
		bool emit_particle(std::vector<Particle>& particles, glm::vec2 emitter_center, glm::vec2 emitter_size, float deltaTime, float block_size) override {
			currentTime += deltaTime;
			if (currentTime >= particleSpawnInterval) {
				currentTime = 0.f;
				glm::vec2 center(emitter_center.x - emitter_size.x * 0.5f + float(rand() % int(emitter_size.x)), emitter_center.y - emitter_size.y * 0.5f + float(rand() % int(emitter_size.y)));
				particles.emplace_back(Particle(0, 2.f, center, glm::vec2(block_size * 0.5, block_size * 0.5), glm::vec2(0.f, block_size * 0.03)));
				return true;
			}
			return false;
		}
	private:
		float particleSpawnInterval = 0.1f;
		float currentTime = 0.f;
	};
	class FrostBurning : public EffectBase {
	public:
		FrostBurning() : EffectBase(EffectType::isDamagingDebuff, "Frostburning!", true) {}
		bool inflictDamage(PlayerStats& stats, float& deltaDMGTime) override {
			if (deltaDMGTime >= 0.25) {
				deltaDMGTime = 0.f;
				stats.currentHP -= 2;
				return true;
			}
			return false;
		}
		bool inflictEntityDamage(EntityStats& stats, float& deltaDMGTime) {
			if (deltaDMGTime >= 0.25) {
				deltaDMGTime = 0.f;
				stats.HP -= 2;
				return true;
			}
			return false;
		}
		bool emit_particle(std::vector<Particle>& particles, glm::vec2 emitter_center, glm::vec2 emitter_size, float deltaTime, float block_size) override {
			currentTime += deltaTime;
			if (currentTime >= particleSpawnInterval) {
				currentTime = 0.f;
				glm::vec2 center(emitter_center.x - emitter_size.x * 0.5f + float(rand() % int(emitter_size.x)), emitter_center.y - emitter_size.y * 0.5f + float(rand() % int(emitter_size.y)));
				particles.emplace_back(Particle(1, 2.f, center, glm::vec2(block_size * 0.5, block_size * 0.5), glm::vec2(0.f, block_size * 0.03)));
				return true;
			}
			return false;
		}
	private:
		float particleSpawnInterval = 0.1f;
		float currentTime = 0.f;
	};
	class Poisoning : public EffectBase {
	public:
		Poisoning() : EffectBase(EffectType::isDamagingDebuff, "Poisoning!", false) {}
		bool inflictDamage(PlayerStats& stats, float& deltaDMGTime) override {
			if (deltaDMGTime >= 0.25) {
				deltaDMGTime = 0.f;
				stats.currentHP -= 1;
				return true;
			}
			return false;
		}
	};
	class MushroomHealing : public EffectBase {
	public:
		MushroomHealing() : EffectBase(EffectType::isHealing, "", false) {}
		bool applyEffect(PlayerStats& stats) override {
			if (stats.currentHP != stats.HP) {
				stats.currentHP += 25;
				return true;
			}
			return false;
		}
	};
	class PotionHealing : public EffectBase {
	public:
		PotionHealing() : EffectBase(EffectType::isHealing, "", false) {}
		bool applyEffect(PlayerStats& stats) override {
			if (stats.currentHP != stats.HP) {
				stats.currentHP += 50;
				return true;
			}
			return false;
		}
	};
}

struct DroppedItem {
	bool has_pick_cd = false; //if player is throwing something, then it has cd for 1 second for picking it up
	float cd_time = 0.f;
	int id = 0;
	int amount = 0;
	Hitbox_2D_AABB hitbox;
	//physics
	bool has_bottom_collision = false;
	float fallingDistance = 0.f;
	float time_in_free_falling = 0.f;
	float Xinc = 0.f;
};