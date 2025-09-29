#define _CRT_SECURE_NO_WARNINGS

#include "Game.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <glm/gtx/string_cast.hpp>
namespace fs = std::filesystem;

Game::Game() {

}

void Game::input() {
	mouse.wheelOffset = 0.f;
}

bool Game::update(GLFWwindow* window, float deltaTime) {
	game_delta_Time += deltaTime;
	FPS_counter++;
	if (game_delta_Time >= 1.0) {
		game_delta_Time = 0.f;
		current_FPS = FPS_counter;
		FPS_counter = 0;
	}

	switch (game_update_state) {
	case inMainMenu: {
		game_render_state = Game_State::inMainMenu;
		ColorVertex2f* ptr = buttons_buffer;
		buttons_amount = 3;
		for (int i = 0; i < 3; i++) {
			if (main_buttons[i].cursor_is_inside(mouse.mouseX, mouse.mouseY) && mouse.left_button) {
				mouse.left_button = false;
				if (i == 0) {
					game_update_state = Game_State::inWorldExplorer;
					break;
				}
				else if (i == 1) {
					game_update_state = Game_State::inOptions;
					break;
				}
				else if (i == 2) {
					return 0;
					break;
				}
			}
			main_buttons[i].update_buffer(ptr);
		}
		return 1;
	}
	case inWorldExplorer: {
		game_render_state = Game_State::inWorldExplorer;
		if (keyStates[GLFW_KEY_ESCAPE]) {
			keyStates[GLFW_KEY_ESCAPE] = false;
			game_update_state = Game_State::inMainMenu;
			return 1;
		}
		ColorVertex2f* ptr = buttons_buffer;
		buttons_amount = 2;
		for (int i = 3; i < 5; i++) {
			if (main_buttons[i].cursor_is_inside(mouse.mouseX, mouse.mouseY) && mouse.left_button) {
				mouse.left_button = false;
				if (i == 3 && !active_world.empty()) {
					loading_the_world = true;
					game_update_state = Game_State::WorldIsLoading;
					std::thread(&Game::load_the_world_thread, this).detach();
					break;
				}
				else if (i == 4) {
					game_update_state = Game_State::inWorldCreator;
					break;
				}
			}
			main_buttons[i].update_buffer(ptr);
		}
		for (auto& button : world_buttons) {
			button.update_pressed_state(mouse);
			if (button.isActive) {
				active_world = button.text;
			}
			button.update_buffer(ptr);
			buttons_amount++;
		}
		return 1;
	}
	case inWorldCreator: {
		game_render_state = Game_State::inWorldCreator;
		if (keyStates[GLFW_KEY_ESCAPE]) {
			keyStates[GLFW_KEY_ESCAPE] = false;
			game_update_state = Game_State::inWorldExplorer;
			text_field.text = "";
			return 1;
		}
		ColorVertex2f* ptr = buttons_buffer;
		buttons_amount = 2;
		if (main_buttons[5].cursor_is_inside(mouse.mouseX, mouse.mouseY) && mouse.left_button && !text_field.text.empty()) {
			mouse.left_button = false;

			std::string fileName("Saves/");
			fileName += text_field.text;
			fileName += ".txt";
			active_world = fileName;
			save_Files.emplace_back(fileName);
			init_world_buttons();
			text_field.text = "";

			creating_the_world = true;
			game_update_state = Game_State::WorldIsCreating;
			std::thread(&Game::create_the_world_thread, this).detach();
			return 1;
		}
		main_buttons[5].update_buffer(ptr);
		text_field.update_for_fileName_input(mouse, keyStates);
		text_field.update_buffer(ptr);
		return 1;
	}
	case WorldIsCreating: {
		game_render_state = Game_State::WorldIsCreating;
		if (!creating_the_world) game_update_state = Game_State::inGame;
		return 1;
	}
	case WorldIsLoading: {
		game_render_state = Game_State::WorldIsLoading;
		if (!loading_the_world) game_update_state = Game_State::inGame;
		return 1;
	}
	case WorldIsSaving: {
		game_render_state = Game_State::WorldIsSaving;
		if (!saving_the_world) game_update_state = Game_State::inMainMenu;
		return 1;
	}
	case inGame: {
		game_render_state = Game_State::inGame;
		if (cycle_time >= dayTime) {
			cycle_time = 0.0;
			isDay = !isDay;
		}
		if (cycle_time <= dayTime * 0.1) {
			if (isDay) {
				day_ratio = 0.5 * (1 + cycle_time / (dayTime * 0.1));
			}
			else {
				day_ratio = 0.5 * (1 - cycle_time / (dayTime * 0.1));
			}
			for (int i = 0; i < 3; ++i) {
				skyColor[i] = nightColor[i] * (1.0f - day_ratio) + dayColor[i] * day_ratio;
			}
		}
		else if (cycle_time >= dayTime * 0.9) {
			if (isDay) {
				day_ratio = 0.5 * (1 + (dayTime - cycle_time) / (dayTime * 0.1));
			}
			else {
				day_ratio = 0.5 * (1 - (dayTime - cycle_time) / (dayTime * 0.1));
			}
			for (int i = 0; i < 3; ++i) {
				skyColor[i] = nightColor[i] * (1.0f - day_ratio) + dayColor[i] * day_ratio;
			}
		}
		else {
			if (isDay) {
				day_ratio = 1.0;
				skyColor[0] = dayColor[0];
				skyColor[1] = dayColor[1];
				skyColor[2] = dayColor[2];
			}
			else {
				day_ratio = 0.0;
				skyColor[0] = nightColor[0];
				skyColor[1] = nightColor[1];
				skyColor[2] = nightColor[2];
			}
		}
		cycle_time += deltaTime;

		rainbow_color_time += 30.f * deltaTime;
		if (rainbow_color_time >= 62.83f)
			rainbow_color_time = 0.f;

		if (keyStates[GLFW_KEY_ESCAPE]) {
			game_update_state = Game_State::WorldIsSaving;
			saving_the_world = true;
			std::thread(&Game::exit_and_save_the_world_thread, this).detach();
			return 1;
		}
		//move
		playerXinc = 0.f;
		playerYinc = 0.f;
		if (keyStates[GLFW_KEY_W]) {
			if (keyStates[GLFW_KEY_LEFT_SHIFT])
				playerYinc = player.speed * 5;
			else
				playerYinc = player.speed;
		}
		if (keyStates[GLFW_KEY_A]) {
			player.looks_at_left = true;
			if (keyStates[GLFW_KEY_LEFT_SHIFT])
				playerXinc = -player.speed * 5;
			else
				playerXinc = -player.speed;
		}
		/*if (keyStates[GLFW_KEY_S])
			if (keyStates[GLFW_KEY_LEFT_SHIFT]) {
				playerYinc = -player.speed * 5;
				if (player.hitbox_left_down_corner.y <= 0.f) {
					player.hitbox_left_down_corner.y = 0.f;
				}
			}
			else {
				playerYinc = -player.speed;
				if (player.hitbox_left_down_corner.y <= 0.f) {
					player.hitbox_left_down_corner.y = 0.f;
				}
			}*/
		if (keyStates[GLFW_KEY_D]) {
			player.looks_at_left = false;
			if (keyStates[GLFW_KEY_LEFT_SHIFT])
				playerXinc = player.speed * 5;
			else
				playerXinc = player.speed;
		}

		//[[TEST]] spawning entities
		if (keyStates[GLFW_KEY_LEFT_SHIFT]) {
			if (keyStates[GLFW_KEY_Z]) {
				keyStates[GLFW_KEY_Z] = false;
				entities.emplace_back(Smart_ptr<EntityBase>(new Zombie(7, player.hitbox.center.x, player.hitbox.center.y + BLOCK_VISIBLE_SIZE * 4, *entityInfo[7], BLOCK_VISIBLE_SIZE)));
			}
			if (keyStates[GLFW_KEY_S]) {
				keyStates[GLFW_KEY_S] = false;
				entities.emplace_back(Smart_ptr<EntityBase>(new Slime(6, player.hitbox.center.x, player.hitbox.center.y + BLOCK_VISIBLE_SIZE * 4, *entityInfo[6], BLOCK_VISIBLE_SIZE)));
			}
			if (keyStates[GLFW_KEY_P]) {
				keyStates[GLFW_KEY_P] = false;
				entities.emplace_back(Smart_ptr<EntityBase>(new FlyingEye(8, player.hitbox.center.x - BLOCK_VISIBLE_SIZE * 4, player.hitbox.center.y + BLOCK_VISIBLE_SIZE * 4, *entityInfo[8], BLOCK_VISIBLE_SIZE)));
				entities.emplace_back(Smart_ptr<EntityBase>(new FlyingEye(9, player.hitbox.center.x + BLOCK_VISIBLE_SIZE * 10, player.hitbox.center.y + BLOCK_VISIBLE_SIZE * 13, *entityInfo[9], BLOCK_VISIBLE_SIZE)));
				entities.emplace_back(Smart_ptr<EntityBase>(new FlyingEye(10, player.hitbox.center.x - BLOCK_VISIBLE_SIZE * 20, player.hitbox.center.y + BLOCK_VISIBLE_SIZE * 9, *entityInfo[10], BLOCK_VISIBLE_SIZE)));
			}
		}

		//inventory
		if (keyStates[GLFW_KEY_E]) {
			keyStates[GLFW_KEY_E] = false;
			if (inventoryIsOpen || active_chest.isOpen) {
				inventoryIsOpen = false;
				active_chest.isOpen = false;
				if (currently_moving_object.object.type) {
					if (index_of_last_slot_picked < INVENTORY_SIZE - 40) {
						//add object from curerntly moving object to inventory slot
						inventory_array[index_of_last_slot_picked].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
						inventory_array[index_of_last_slot_picked].amount = currently_moving_object.amount;
						//delete it from the currently moving object
						currently_moving_object.object = ItemObject(None, 0);
						currently_moving_object.amount = 0;
					}
					else {
						//add object from curerntly moving object to inventory slot
						active_chest.slot_pointer[index_of_last_slot_picked - (INVENTORY_SIZE - 40)].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
						active_chest.slot_pointer[index_of_last_slot_picked - (INVENTORY_SIZE - 40)].amount = currently_moving_object.amount;
						//delete it from the currently moving object
						currently_moving_object.object = ItemObject(None, 0);
						currently_moving_object.amount = 0;
					}
				}
				active_chest.slot_pointer = nullptr;
			}
			else {
				inventoryIsOpen = true;
				crafting_system.index_of_current_craft = 0;
			}
		}
		//zoom [50% - 200%]
		if (keyStates[GLFW_KEY_PAGE_UP]) {
			keyStates[GLFW_KEY_PAGE_UP] = false;
			if (camera.scaling < 2.0) {
				camera.scaling += 0.25;
				camera.scalingDx = ScreenWidth / 2 * (1 / camera.scaling) - ScreenWidth / 2;
				camera.scalingDy = ScreenHeight / 2 * (1 / camera.scaling) - ScreenHeight / 2;
				camera.rightBorderDx = -world_width * BLOCK_VISIBLE_SIZE + ScreenWidth + camera.scalingDx;
				camera.topBorderDy = -world_height * BLOCK_VISIBLE_SIZE + ScreenHeight + camera.scalingDy;
			}
		}
		if (keyStates[GLFW_KEY_PAGE_DOWN]) {
			keyStates[GLFW_KEY_PAGE_DOWN] = false;
			if (camera.scaling > 0.5) {
				camera.scaling -= 0.25;
				camera.scalingDx = ScreenWidth / 2 * (1 / camera.scaling) - ScreenWidth / 2;
				camera.scalingDy = ScreenHeight / 2 * (1 / camera.scaling) - ScreenHeight / 2;
				camera.rightBorderDx = -world_width * BLOCK_VISIBLE_SIZE + ScreenWidth + camera.scalingDx;
				camera.topBorderDy = -world_height * BLOCK_VISIBLE_SIZE + ScreenHeight + camera.scalingDy;
			}
		}
		if (!active_weapon.isActive) { //maybe should change this if
			//scroll active slot in ui bar
			if (mouse.wheelOffset != 0 && !inventoryIsOpen) {
				if (mouse.wheelOffset > 0) {
					active_bar_slot--;
					if (active_bar_slot == -1) active_bar_slot = 9;
				}
				else {
					active_bar_slot++;
					if (active_bar_slot == 10) active_bar_slot = 0;
				}
			}
			//or use numbers
			if (keyStates[GLFW_KEY_0]) active_bar_slot = 9;
			if (keyStates[GLFW_KEY_1]) active_bar_slot = 0;
			if (keyStates[GLFW_KEY_2]) active_bar_slot = 1;
			if (keyStates[GLFW_KEY_3]) active_bar_slot = 2;
			if (keyStates[GLFW_KEY_4]) active_bar_slot = 3;
			if (keyStates[GLFW_KEY_5]) active_bar_slot = 4;
			if (keyStates[GLFW_KEY_6]) active_bar_slot = 5;
			if (keyStates[GLFW_KEY_7]) active_bar_slot = 6;
			if (keyStates[GLFW_KEY_8]) active_bar_slot = 7;
			if (keyStates[GLFW_KEY_9]) active_bar_slot = 8;
		}

		if (keyStates[GLFW_KEY_T]) {
			keyStates[GLFW_KEY_T] = false;
			isDay = !isDay;
		}

		if (keyStates[GLFW_KEY_F]) {
			keyStates[GLFW_KEY_F] = false;
			toggle_Fullscreen(window);
		}

		//[[TEST]] crafting system
		if (keyStates[GLFW_KEY_C]) {
			keyStates[GLFW_KEY_C] = false;
			if (crafting_system.show_all_crafts)
				crafting_system.show_all_crafts = false;
			else
				crafting_system.show_all_crafts = true;
		}

		//check collision for player
		if (collisionIsOn) {
			player.moving_down = false;
			player.has_bottom_collision_only_with_objects = false;
			bool has_side = false;
			bool has_bottom = false;
			bool has_top = false;
			int leftX = (player.hitbox.center.x - player.hitbox.size.x * 0.5f) / BLOCK_VISIBLE_SIZE - 1;
			if (leftX < 0) leftX = 0;
			int rightX = leftX + 3;
			if (rightX >= world_width) rightX = world_width - 1;
			int bottomY = (player.hitbox.center.y - player.hitbox.size.y * 0.5f) / BLOCK_VISIBLE_SIZE - 1;
			if (bottomY < 0) bottomY = 0;
			int topY = bottomY + 4;
			if (topY >= world_height) topY = world_height - 1;

			if (!player.has_bottom_collision) { //if the player doesn't have bottom collision, then he is falling
				//add delta time to time in free falling
				player.time_in_free_falling += deltaTime;
				//calculate distance with this time_in_free_falling between last and current distances and add to current player Y
				float distance = player.jump_V0 * player.time_in_free_falling - BLOCK_VISIBLE_SIZE * 2 * 9.8f * player.time_in_free_falling * player.time_in_free_falling / 2;
				float dY = distance - player.fallingDistance;
				player.hitbox.center.y += dY;
				//if the player has jumped, then calculate the current Y level to later get the level where it stops and starts going down
				if (player.jump_V0 && dY >= 0.f)
					player.current_Y_max_level = (player.hitbox.center.y - player.hitbox.size.y * 0.5f) / BLOCK_VISIBLE_SIZE;
				//check whether the player is moving down or up based on positive/negative delta distance
				if (dY < 0.f)
					player.moving_down = true;
				//adjust camera
				camera.dY -= dY;
				//remember current distance while moving
				player.fallingDistance = distance;
			}
			//check player collision with blocks
			int object_id;
			for (int i = leftX; i <= rightX; i++) {
				for (int j = bottomY; j <= topY; j++) {
					if (sprites_Array[i][j].object.object_type) {
						if (sprites_Array[i][j].object.object_type == isCompObjPart) {  //if part of complex object, then use its column and line
							object_id = sprites_Array[sprites_Array[i][j].object.component->get_column()][sprites_Array[i][j].object.component->get_line()].object.object_id;
						}
						else {  //if it's simple or complex object
							object_id = sprites_Array[i][j].object.object_id;
						}
						if (objectInfo[object_id]->allow_bottom_collision()) { //if only bottom collision is allowed
							if (CollisionManager::getTypeCollisionAABBwithBlock(player.hitbox, i, j, BLOCK_VISIBLE_SIZE) == BOTTOM && player.current_Y_max_level > j) {
								if (player.jump_V0 && !player.moving_down)
									continue;
								if (playerYinc < 0.f) playerYinc = 0.f;
								player.time_in_free_falling = 0.f;
								player.hitbox.center.y = j * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + player.hitbox.size.y * 0.5f;
								player.fallingDistance = 0.f;
								player.jump_V0 = 0.f;
								player.current_Y_max_level = j + 1;
								player.has_bottom_collision_only_with_objects = true;
							}
						}
						else if (objectInfo[object_id]->allow_collision()) //if all types of collision are allowed
							switch (CollisionManager::getTypeCollisionAABBwithBlock(player.hitbox, i, j, BLOCK_VISIBLE_SIZE)) {
							case LEFT:
								if (!(is_solid_block(i + 1, j))) {
									if (playerXinc < 0.f) playerXinc = 0.f;
									player.hitbox.center.x = i * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + player.hitbox.size.x * 0.5f;
									has_side = true;
								}
								break;
							case RIGHT:
								if (playerXinc > 0.f) playerXinc = 0.f;
								player.hitbox.center.x = i * BLOCK_VISIBLE_SIZE - player.hitbox.size.x * 0.5f;
								has_side = true;
								break;
							case TOP:
								if (playerYinc > 0.f) playerYinc = 0.f;
								has_top = true;
								player.time_in_free_falling = 0.f;
								player.hitbox.center.y = j * BLOCK_VISIBLE_SIZE - player.hitbox.size.y * 0.5f;
								player.fallingDistance = 0.f;
								player.jump_V0 = 0.f;
								break;
							case BOTTOM:
								if (playerYinc < 0.f) playerYinc = 0.f;
								has_bottom = true;
								player.time_in_free_falling = 0.f;
								player.hitbox.center.y = j * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + player.hitbox.size.y * 0.5f;
								player.fallingDistance = 0.f;
								player.jump_V0 = 0.f;
								player.current_Y_max_level = j + 1;
								break;
							case CORNER:
								if (playerXinc != 0.f && playerYinc != 0.f) playerYinc = 0.f;
								break;
							default:
								break;
							}
					}
				}
			}
			if (has_bottom) {
				player.has_bottom_collision = true;
				player.has_bottom_collision_only_with_objects = false;
			}
			else player.has_bottom_collision = false;
			if (has_side) {
				player.has_side_collision = true;
			}
			else player.has_side_collision = false;
			if (has_top) player.has_top_collision = true;
			else player.has_top_collision = false;
			if (player.has_bottom_collision_only_with_objects)
				player.has_bottom_collision = true;

			if (player.has_side_collision && keyStates[GLFW_KEY_D] && player.has_bottom_collision) { //auto "jump" on one block, if going right and it is possible
				if (sprites_Array[rightX][bottomY + 1].object.object_type &&
					!sprites_Array[rightX][bottomY + 2].object.object_type &&
					!sprites_Array[rightX][bottomY + 3].object.object_type &&
					!sprites_Array[rightX][bottomY + 4].object.object_type &&
					!sprites_Array[rightX - 1][bottomY + 4].object.object_type &&
					!sprites_Array[rightX - 2][bottomY + 4].object.object_type) {
					player.hitbox.center.y = (bottomY + 1) * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + player.hitbox.size.y * 0.5f;
					camera.dY -= BLOCK_VISIBLE_SIZE;
					playerXinc = 5.f;
				}
			}
			if (player.has_side_collision && keyStates[GLFW_KEY_A] && player.has_bottom_collision) { //auto "jump" on one block, if going left and it is possible
				if (sprites_Array[leftX][bottomY + 1].object.object_type &&
					!sprites_Array[leftX][bottomY + 2].object.object_type &&
					!sprites_Array[leftX][bottomY + 3].object.object_type &&
					!sprites_Array[leftX][bottomY + 4].object.object_type &&
					!sprites_Array[leftX + 1][bottomY + 4].object.object_type &&
					!sprites_Array[leftX + 2][bottomY + 4].object.object_type) {
					player.hitbox.center.y = (bottomY + 1) * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + player.hitbox.size.y * 0.5f;
					camera.dY -= BLOCK_VISIBLE_SIZE;
					playerXinc = -5.f;
				}
			}
			if (keyStates[GLFW_KEY_SPACE] && player.has_bottom_collision && !player.has_top_collision) { //if has bottom collision, then can jump
				player.has_bottom_collision = false;
				player.jump_V0 = player.jump_speed;
			}
			if (keyStates[GLFW_KEY_S] && player.has_bottom_collision_only_with_objects) {
				player.has_bottom_collision = false;
				player.has_bottom_collision_only_with_objects = false;
				player.current_Y_max_level--; //make it lower by one so that when going throught the object it won't count bottom collision
			}
			player.sprite_time += deltaTime;
			if (player.has_bottom_collision || player.has_bottom_collision_only_with_objects) {
				if (playerXinc != 0.f) {
					if (player.current_sprite == 0)
						player.current_sprite = 2;
					else if (player.sprite_time >= 0.2f) {
						player.sprite_time = 0.f;
						player.current_sprite++;
						if (player.current_sprite > 5)
							player.current_sprite = 2;
					}
				}
				else
					player.current_sprite = 0;
			}
			else {
				player.current_sprite = 1;
			}
		}
		player.hitbox.center.x += playerXinc * deltaTime * player.stats.speedFactor;
		player.hitbox.center.y += playerYinc * deltaTime * player.stats.speedFactor;
		if (player.hitbox.center.x - player.hitbox.size.x * 0.5f < 0) player.hitbox.center.x = player.hitbox.size.x * 0.5f;
		if (player.hitbox.center.y - player.hitbox.size.y * 0.5f < 0) player.hitbox.center.y = player.hitbox.size.y * 0.5f;
		if (player.hitbox.center.x + player.hitbox.size.x * 0.5f > world_width * BLOCK_VISIBLE_SIZE) player.hitbox.center.x = world_width * BLOCK_VISIBLE_SIZE - player.hitbox.size.x * 0.5f;
		if (player.hitbox.center.y + player.hitbox.size.y * 0.5f > world_height * BLOCK_VISIBLE_SIZE) {
			player.time_in_free_falling = 0.f;
			player.fallingDistance = 0.f;
			player.jump_V0 = 0.f;
			player.hitbox.center.y = world_height * BLOCK_VISIBLE_SIZE - player.hitbox.size.y * 0.5f;
		}
		camera.dX = -player.hitbox.center.x + ScreenWidth / 2;
		camera.dY = -player.hitbox.center.y + ScreenHeight / 2;
		if (camera.dX > -camera.scalingDx) camera.dX = -camera.scalingDx;
		if (camera.dX < camera.rightBorderDx) camera.dX = camera.rightBorderDx;
		if (camera.dY > -camera.scalingDy) camera.dY = -camera.scalingDy;
		if (camera.dY < camera.topBorderDy) camera.dY = camera.topBorderDy;

		//update player effects
		for (int i = 0; i < player.effects.size(); i++) {
			Effect& effect = player.effects[i];
			//update effect and remove if needed
			if (effect.updateEffect(deltaTime)) {
				effects[effect.id]->removeEffect(player.stats);
				//remove potion seakness
				if (effect.id == 8)
					player.stats.hasPotionSickness = false;
				player.effects.erase(player.effects.begin() + i);
				i--;
				continue;
			}
			if (effects[effect.id]->type == EffectType::isDamagingDebuff)
				effects[effect.id]->inflictDamage(player.stats, effect.delta_dmg_time);
		}

		//All entities
		entities_count = 0;
		EntityData* entity_buf = entity_sprite_buf;
		//player
		{
			player.sprite_left_down_corner.x = player.hitbox.center.x - player.sprite_size.x * 0.5f;
			player.sprite_left_down_corner.y = player.hitbox.center.y - player.hitbox.size.y * 0.5f;
			glm::mat4 matModel(1.f);
			matModel = glm::translate(matModel, glm::vec3(player.sprite_left_down_corner.x, player.sprite_left_down_corner.y, 0.f));
			matModel = glm::scale(matModel, glm::vec3(player.sprite_size.x, player.sprite_size.y, 0.f));
			entity_buf->modelMatrix = matModel;
			int offset = player.current_sprite * 4;
			if (player.looks_at_left) {
				entity_buf->tex_coords[0] = player.tex_coords[offset + 3];
				entity_buf->tex_coords[1] = player.tex_coords[offset + 2];
				entity_buf->tex_coords[2] = player.tex_coords[offset + 1];
				entity_buf->tex_coords[3] = player.tex_coords[offset];
				entity_buf->tex_id = 0.f;
			}
			else {
				entity_buf->tex_coords[0] = player.tex_coords[offset];
				entity_buf->tex_coords[1] = player.tex_coords[offset + 1];
				entity_buf->tex_coords[2] = player.tex_coords[offset + 2];
				entity_buf->tex_coords[3] = player.tex_coords[offset + 3];
				entity_buf->tex_id = 0.f;
			}
			entity_buf++;
			entities_count++;
		}

		//ambient sprites
		{
			SpriteData* ptr = ambient_sprite_buf;
			ambientController.updateAmbientLayer(isDay, cycle_time, day_ratio, ptr, ScreenWidth, ScreenHeight, BLOCK_VISIBLE_SIZE, deltaTime);
		}
		sprite_ambient_ssbo_p->bind_SSBO();
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SpriteData) * 12, ambient_sprite_buf);

		//view matrix
		viewMatrix = glm::mat4(1.f);
		//scale the camera
		viewMatrix = glm::scale(viewMatrix, glm::vec3(camera.scaling, camera.scaling, 0.f));
		//adjust camera based on the scaling (scales the camera in the middle, not from the corner) and change camera position based on camera dx and dy
		viewMatrix = glm::translate(viewMatrix, glm::vec3(camera.scalingDx + camera.dX, camera.scalingDy + camera.dY, 0.f));

		sprite_SP_ptr->activate_shader();
		sprite_SP_ptr->set_Uniform_Mat4("viewMatrix", viewMatrix);
		entity_sprite_SP_ptr->activate_shader();
		entity_sprite_SP_ptr->set_Uniform_Mat4("viewMatrix", viewMatrix);

		//update damage text
		int dmg_text_size = damage_text.size();
		for (int i = 0; i < dmg_text_size; i++) {
			if (damage_text[i].updateText(deltaTime, BLOCK_VISIBLE_SIZE)) {
				damage_text.erase(damage_text.begin() + i);
				dmg_text_size--;
				i--;
			}
		}

		//entities
		glm::vec2* tex_coords_ptr;
		entity_info_text.isActive = false;
		for (int e = 0; e < entities.size(); e++) {
			Smart_ptr<EntityBase>& entity = entities[e];
			float x = entity->hitbox.center.x - entity->hitbox.size.x * 0.5f;
			float y = entity->hitbox.center.y - entity->hitbox.size.y * 0.5f;
			float width = entity->hitbox.size.x;
			float height = entity->hitbox.size.y;
			//despawn any entity if it's more than 150 blocks away on X or Y axis from the player
			if (std::abs(x - player.hitbox.center.x) >= 150.f * BLOCK_VISIBLE_SIZE ||
				std::abs(y - player.hitbox.center.y) >= 150.f * BLOCK_VISIBLE_SIZE)
			{
				entities.erase(entities.begin() + e);
				e--;
				continue;
			}
			if (entity->get_HP() <= 0) {
				entity->do_entity_death_sound(audio_manager);
				drop_enemy_items(entity->entity_id, x, y + height / 2);
				entities.erase(entities.begin() + e);
				e--;
				continue;
			}
			//show info if mouse points on this entity's hitbox
			if (mouse.mouseX * (1 / camera.scaling) - camera.dX - camera.scalingDx >= x && mouse.mouseX * (1 / camera.scaling) - camera.dX - camera.scalingDx <= x + width &&
				mouse.mouseY * (1 / camera.scaling) - camera.dY - camera.scalingDy >= y && mouse.mouseY * (1 / camera.scaling) - camera.dY - camera.scalingDy <= y + height)
			{
				entity_info_text.isActive = true;
				int id = entity->entity_id;
				entity_info_text.info = entityInfo[id]->name + "(" + std::to_string(entity->get_HP()) + "/" + std::to_string(entityInfo[id]->get_HP()) + ")";
				entity_info_text.start_pos.x = mouse.mouseX - BLOCK_VISIBLE_SIZE * 1.5;
				entity_info_text.start_pos.y = mouse.mouseY;
			}

			int leftX = x / BLOCK_VISIBLE_SIZE - 1;
			int rightX = leftX + (int)(width - 0.1) + 2; //hmm
			int bottomY = y / BLOCK_VISIBLE_SIZE - 1;
			int topY = bottomY + (int)(height - 0.1) + 2; //hmm
			if (leftX < 0) leftX = 0;
			if (rightX >= world_width) rightX = world_width - 1;
			if (bottomY < 0) bottomY = 0;
			if (topY >= world_height) topY = world_height - 1;

			//update entity logic
			entity->update_entity(deltaTime, BLOCK_VISIBLE_SIZE, player.hitbox.center.x, player.hitbox.center.y);
			//entity sounds
			entity->do_entity_sounds(audio_manager);
			//physics
			EntityMovementType m_type = entity->get_movement_type();
			if (m_type == EntityMovementType::isWalking) {
				WalkingEnemyPhysics& physx = entity->get_walking_physics();
				physx.moving_down = false;
				physx.has_bottom_collision_only_with_objects = false;
				physx.has_side_collision = false;
				physx.has_top_collision = false;
				bool has_bottom = false;
				float eXinc = physx.Xinc;

				if (!physx.has_bottom_collision) {
					//add delta time to time in free falling
					physx.time_in_free_falling += deltaTime;
					//calculate distance with this time_in_free_falling between last and current distances and add to current player Y
					float distance = physx.current_jump_V * physx.time_in_free_falling - BLOCK_VISIBLE_SIZE * 2 * 9.8f * physx.time_in_free_falling * physx.time_in_free_falling / 2;
					float dY = distance - physx.fallingDistance;
					entity->hitbox.center.y += dY;
					//if the entity has jumped, then calculate the current Y level to later get the level where it stops and starts going down
					if (physx.current_jump_V && dY >= 0.f)
						physx.current_Y_max_level = entity->hitbox.center.y / BLOCK_VISIBLE_SIZE;
					//check whether the entity is moving down or up based on positive/negative delta distance
					if (dY < 0.f)
						physx.moving_down = true;
					//remember current distance while moving
					physx.fallingDistance = distance;
				}
				int object_id;
				for (int i = leftX; i <= rightX; i++) {
					for (int j = bottomY; j <= topY; j++) {
						if (sprites_Array[i][j].object.object_type) {
							if (sprites_Array[i][j].object.object_type == isCompObjPart) {  //if part of complex object, then use its column and line
								object_id = sprites_Array[sprites_Array[i][j].object.component->get_column()][sprites_Array[i][j].object.component->get_line()].object.object_id;
							}
							else {  //if it's simple or complex object
								object_id = sprites_Array[i][j].object.object_id;
							}
							if (objectInfo[object_id]->allow_bottom_collision()) { //if only bottom collision is allowed
								if (CollisionManager::getTypeCollisionAABBwithBlock(entity->hitbox, i, j, BLOCK_VISIBLE_SIZE) == BOTTOM && physx.current_Y_max_level > j) {
									if (physx.current_jump_V && !physx.moving_down)
										continue;
									physx.time_in_free_falling = 0.f;
									entity->hitbox.center.y = j * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + height * 0.5f;
									physx.fallingDistance = 0.f;
									physx.current_jump_V = 0.f;
									physx.current_Y_max_level = j + 1;
									physx.has_bottom_collision_only_with_objects = true;
								}
							}
							else if (objectInfo[object_id]->allow_collision()) //if all types of collision are allowed
								switch (CollisionManager::getTypeCollisionAABBwithBlock(entity->hitbox, i, j, BLOCK_VISIBLE_SIZE)) {
								case LEFT:
									if (!(is_solid_block(i + 1, j))) {
										entity->hitbox.center.x = i * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + width * 0.5f;
										physx.has_side_collision = true;
										if (eXinc < 0.f) eXinc = 0.f;
									}
									break;
								case RIGHT:
									entity->hitbox.center.x = i * BLOCK_VISIBLE_SIZE - width * 0.5f;
									physx.has_side_collision = true;
									if (eXinc > 0.f) eXinc = 0.f;
									break;
								case TOP:
									physx.has_top_collision = true;
									physx.time_in_free_falling = 0.f;
									entity->hitbox.center.y = j * BLOCK_VISIBLE_SIZE - height * 0.5f;
									physx.fallingDistance = 0.f;
									physx.current_jump_V = 0.f;
									break;
								case BOTTOM:
									has_bottom = true;
									physx.time_in_free_falling = 0.f;
									entity->hitbox.center.y = j * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + height * 0.5f;
									physx.fallingDistance = 0.f;
									physx.current_jump_V = 0.f;
									physx.current_Y_max_level = j + 1;
									break;
								case CORNER:
									break;
								default:
									break;
								}
						}
					}
				}
				if (has_bottom) {
					physx.has_bottom_collision = true;
					physx.has_bottom_collision_only_with_objects = false;
				}
				else physx.has_bottom_collision = false;
				if (physx.has_bottom_collision_only_with_objects)
					physx.has_bottom_collision = true;

				if (physx.should_jump) { //start jump
					physx.should_jump = false;
					physx.has_bottom_collision = false;
					physx.has_bottom_collision_only_with_objects = false;
					physx.current_jump_V = physx.jump_V0 * BLOCK_VISIBLE_SIZE;
				}
				entity->hitbox.center.x += eXinc;
				physx.Xinc = eXinc;
			}
			else if (m_type == EntityMovementType::isFlying) {
				FlyingEnemyPhysics& physx = entity->get_flying_physics();
				float eXinc = physx.Xinc, eYinc = physx.Yinc;
				int object_id;
				for (int i = leftX; i <= rightX; i++) {
					for (int j = bottomY; j <= topY; j++) {
						if (sprites_Array[i][j].object.object_type) {
							if (sprites_Array[i][j].object.object_type == isCompObjPart) {  //if part of complex object, then use its column and line
								object_id = sprites_Array[sprites_Array[i][j].object.component->get_column()][sprites_Array[i][j].object.component->get_line()].object.object_id;
							}
							else {  //if it's simple or complex object
								object_id = sprites_Array[i][j].object.object_id;
							}
							if (objectInfo[object_id]->allow_collision()) //if all types of collision are allowed
								switch (CollisionManager::getTypeCollisionAABBwithBlock(entity->hitbox, i, j, BLOCK_VISIBLE_SIZE)) {
								case LEFT:
									entity->hitbox.center.x = i * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + width * 0.5f;
									if (eXinc < 0.f) eXinc = BLOCK_VISIBLE_SIZE * deltaTime * 4;
									break;
								case RIGHT:
									entity->hitbox.center.x = i * BLOCK_VISIBLE_SIZE - width * 0.5f;
									if (eXinc > 0.f) eXinc = -BLOCK_VISIBLE_SIZE * deltaTime * 4;
									break;
								case TOP:
									entity->hitbox.center.y = j * BLOCK_VISIBLE_SIZE - height * 0.5f;
									if (eYinc > 0.f) eYinc = 0.f;
									break;
								case BOTTOM:
									entity->hitbox.center.y = j * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + height * 0.5f;
									if (eYinc < 0.f) eYinc = 0.f;
									break;
								case CORNER:
									break;
								default:
									break;
								}
						}
					}
				}
				entity->hitbox.center.x += physx.Xinc;
				entity->hitbox.center.y += physx.Yinc;
				physx.Xinc = eXinc; physx.Yinc = eYinc;
			}
			else {

			}
			//update entity model
			entity->update_model(BLOCK_VISIBLE_SIZE);

			entity_buf->modelMatrix = entity->matModel;
			tex_coords_ptr = entityInfo[entity->entity_id]->get_tex_coords_ptr();
			int offset = 4 * entity->get_tex_index();
			if (entity->looks_at_left) {
				entity_buf->tex_coords[0] = tex_coords_ptr[offset + 3];
				entity_buf->tex_coords[1] = tex_coords_ptr[offset + 2];
				entity_buf->tex_coords[2] = tex_coords_ptr[offset + 1];
				entity_buf->tex_coords[3] = tex_coords_ptr[offset];
				entity_buf->tex_id = 0.f;
			}
			else {
				entity_buf->tex_coords[0] = tex_coords_ptr[offset];
				entity_buf->tex_coords[1] = tex_coords_ptr[offset + 1];
				entity_buf->tex_coords[2] = tex_coords_ptr[offset + 2];
				entity_buf->tex_coords[3] = tex_coords_ptr[offset + 3];
				entity_buf->tex_id = 0.f;
			}
			entity_buf++;
			entities_count++;

			//entity effects
			EntityStats& stats = entity->get_entity_stats();
			for (int i = 0; i < stats.effects.size(); i++) {
				Effect& effect = stats.effects[i];
				//update effect and remove if needed
				if (effect.updateEffect(deltaTime)) {
					stats.effects.erase(stats.effects.begin() + i);
					i--;
					continue;
				}
				if (effects[effect.id]->emitsParticles) {
					if (effects[effect.id]->emit_particle(particles_v, entity->hitbox.center, glm::vec2(width, height), deltaTime, BLOCK_VISIBLE_SIZE)) {
						Particle& particle = particles_v.back();
						ParticleInfo& info = particlesInfo[particle.id];
						if (info.emitsLight) {
							update_lighting(particle.sprite_center, info.light.light_color, info.light.light_radius* BLOCK_VISIBLE_SIZE, true);
						}
					}
				}
				if (effects[effect.id]->type == EffectType::isDamagingDebuff)
					effects[effect.id]->inflictEntityDamage(stats, effect.delta_dmg_time);
			}
		}
		//projectiles and active weapon
		
		//[[PROBABLY CAN ADD THIS RIGHT IN ENTITIES CYCLE
		if (active_weapon.isActive && active_weapon.hitboxIsActive) {
			for (int e = 0; e < entities.size(); e++) {
				Smart_ptr<EntityBase>& entity = entities[e];
				if (entity->get_entity_stats().hit_cd == 0.f && CollisionManager::checkCollision_AABB_with_OBB(entity->hitbox, active_weapon.hitbox)) {
					int damage = objectInfo[active_weapon.weapon_id]->get_damage();
					//crit damage
					if (1 + rand() % 100 <= objectInfo[active_weapon.weapon_id]->get_crit_chance()) {
						damage *= 2;
						damage_text.emplace_back(DamageText(glm::vec2(entity->hitbox.center.x, entity->hitbox.center.y),
							glm::vec4(1.f, 0.475f, 0.204f, 1.f), std::to_string(damage), BLOCK_VISIBLE_SIZE * 1.5f));
					}
					else {
						damage_text.emplace_back(DamageText(glm::vec2(entity->hitbox.center.x, entity->hitbox.center.y),
							glm::vec4(1.f, 0.906f, 0.78f, 1.f), std::to_string(damage), BLOCK_VISIBLE_SIZE));
					}
					entity->decrease_HP(damage);
					entity->do_entity_hit_sound(audio_manager);

					if (active_weapon.time_to_finish_swing < active_weapon.hit_cd)
						entity->get_entity_stats().hit_cd = active_weapon.time_to_finish_swing;
					else
						entity->get_entity_stats().hit_cd = active_weapon.hit_cd;
				}
			}
		}
		//]]

		for (int i = 0; i < projectiles.size(); i++) {
			Smart_ptr<EntityBase>& projectile = projectiles[i];
			int column = projectile->sprite_center_point.x / BLOCK_VISIBLE_SIZE;
			int line = projectile->sprite_center_point.y / BLOCK_VISIBLE_SIZE;
			Smart_ptr<EntityInfo>& projectileInfo = entityInfo[projectile->entity_id];
			//remove previous light
			if (projectileInfo->emitsLight) {
				AppliableLight light = projectileInfo->light;
				update_lighting(projectile->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, false);
			}
			//if out of world
			if (column < 0 || column >= world_width || line < 0 || line >= world_height) {
				projectiles.erase(projectiles.begin() + i);
				i--;
				continue;
			}
			//if far away from player
			if (std::abs(projectile->hitbox.center.x - player.hitbox.center.x) >= 150.f * BLOCK_VISIBLE_SIZE ||  ///HHMMMMMMMMMMM
				std::abs(projectile->hitbox.center.y - player.hitbox.center.y) >= 150.f * BLOCK_VISIBLE_SIZE)
			{
				projectiles.erase(projectiles.begin() + i);
				i--;
				continue;
			}
			//check sollision with objects
			if (is_solid_block(column, line))
				if (CollisionManager::getTypeCollisionAABBwithBlock(projectile->hitbox, column, line, BLOCK_VISIBLE_SIZE)) {
					projectiles.erase(projectiles.begin() + i);
					i--;
					continue;
				}
			//check collision with entities
			bool next = false;
			for (int e = 0; e < entities.size(); e++) {
				Smart_ptr<EntityBase>& entity = entities[e];
				if (entity->get_entity_stats().hit_cd == 0.f && CollisionManager::checkCollisionAABB(projectile->hitbox, entity->hitbox)) {
					if (entity->get_HP() <= 0) {
						continue;
					}
					//entity takes damage
					int damage = projectile->get_proj_dmg();
					bool isCrit = projectile->dmg_is_crit();
					entity->decrease_HP(damage);
					entity->do_entity_hit_sound(audio_manager);
					//crit damage
					if (isCrit) {
						damage_text.emplace_back(DamageText(glm::vec2(entity->hitbox.center.x, entity->hitbox.center.y),
							glm::vec4(1.f, 0.475f, 0.204f, 1.f), std::to_string(damage), BLOCK_VISIBLE_SIZE * 1.5f));
					}
					else {
						damage_text.emplace_back(DamageText(glm::vec2(entity->hitbox.center.x, entity->hitbox.center.y),
							glm::vec4(1.f, 0.906f, 0.78f, 1.f), std::to_string(damage), BLOCK_VISIBLE_SIZE));
					}
					if (entityInfo[projectile->entity_id]->hasEffect) {
						AppliableEffect effect = entityInfo[projectile->entity_id]->effect;
						EntityStats& stats = entity->get_entity_stats();
						apply_entity_effect(Effect(effect.duration, effect.id), stats);
					}
					//remove this projectile if no available hits left
					if (!projectile->update_proj_hits_counter()) {
						projectiles.erase(projectiles.begin() + i);
						i--;
						next = true;
						break;
					}
					entity->get_entity_stats().hit_cd = 0.5f;
				}
			}
			if (next) continue;
			projectile->update_entity(deltaTime, BLOCK_VISIBLE_SIZE, 0.f, 0.f);
			entity_buf->modelMatrix = projectile->matModel;
			tex_coords_ptr = entityInfo[projectile->entity_id]->get_tex_coords_ptr();
			if (projectile->looks_at_left) {
				entity_buf->tex_coords[0] = tex_coords_ptr[3];
				entity_buf->tex_coords[1] = tex_coords_ptr[2];
				entity_buf->tex_coords[2] = tex_coords_ptr[1];
				entity_buf->tex_coords[3] = tex_coords_ptr[0];
				entity_buf->tex_id = 0.f;
			}
			else {
				entity_buf->tex_coords[0] = tex_coords_ptr[0];
				entity_buf->tex_coords[1] = tex_coords_ptr[1];
				entity_buf->tex_coords[2] = tex_coords_ptr[2];
				entity_buf->tex_coords[3] = tex_coords_ptr[3];
				entity_buf->tex_id = 0.f;
			}
			entity_buf++;
			entities_count++;
			if (projectileInfo->emitsLight) {
				AppliableLight light = projectileInfo->light;
				update_lighting(projectile->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
			}
		}
		if (active_weapon.isActive) {
			entity_buf->modelMatrix = active_weapon.modelMatrix;
			if (active_weapon.render_upside_down) {
				entity_buf->tex_coords[0] = active_weapon.tex_coords[1];
				entity_buf->tex_coords[1] = active_weapon.tex_coords[0];
				entity_buf->tex_coords[2] = active_weapon.tex_coords[3];
				entity_buf->tex_coords[3] = active_weapon.tex_coords[2];
			}
			else {
				entity_buf->tex_coords[0] = active_weapon.tex_coords[0];
				entity_buf->tex_coords[1] = active_weapon.tex_coords[1];
				entity_buf->tex_coords[2] = active_weapon.tex_coords[2];
				entity_buf->tex_coords[3] = active_weapon.tex_coords[3];
			}
			entity_buf->tex_id = 0.f;
			entity_buf++;
			entities_count++;
		}
		//update particles
		for (int i = 0; i < particles_v.size(); i++) {
			Particle& particle = particles_v[i];
			ParticleInfo& info = particlesInfo[particle.id];
			if (info.emitsLight) {
				update_lighting(particle.sprite_center, info.light.light_color, info.light.light_radius * BLOCK_VISIBLE_SIZE, false);
			}
			if (particle.update(deltaTime)) {
				particles_v.erase(particles_v.begin() + i);
				i--;
				continue;
			}
			if (info.emitsLight) {
				update_lighting(particle.sprite_center, info.light.light_color, info.light.light_radius * BLOCK_VISIBLE_SIZE, true);
			}
		}
		//add particles to entities buffer
		int size = particles_v.size();
		for (int i = 0; i < size; i++) {
			tex_coords_ptr = particlesInfo[particles_v[i].id].tex_coords;
			entity_buf->modelMatrix = particles_v[i].modelMatrix;
			entity_buf->tex_coords[0] = tex_coords_ptr[0];
			entity_buf->tex_coords[1] = tex_coords_ptr[1];
			entity_buf->tex_coords[2] = tex_coords_ptr[2];
			entity_buf->tex_coords[3] = tex_coords_ptr[3];
			entity_buf->tex_id = 0.f;
			entity_buf++;
			entities_count++;
		}

		sprite_ssbo_p->bind_SSBO();
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(EntityData)* entities_count, entity_sprite_buf);

		//physics update for dropped items
		for (int d = 0; d < dropped_items.size(); d++) {
			DroppedItem& item = dropped_items[d];
			float x = item.hitbox.center.x - item.hitbox.size.x * 0.5f;
			float y = item.hitbox.center.y - item.hitbox.size.y * 0.5f;
			//delete if out of world
			if (x > world_width * BLOCK_VISIBLE_SIZE || x < 0) {
				dropped_items.erase(dropped_items.begin());
				d--;
				continue;
			}
			//update falling for item if needed
			if (!item.has_bottom_collision) {
				item.time_in_free_falling += deltaTime;
				float distance = - BLOCK_VISIBLE_SIZE * 2 * 9.8f * item.time_in_free_falling * item.time_in_free_falling / 2;
				item.hitbox.center.y += distance - item.fallingDistance;
				item.fallingDistance = distance;
			}
			item.hitbox.center.x += item.Xinc * deltaTime;
			if (item.has_pick_cd)
				item.cd_time += deltaTime;
			if (item.has_pick_cd && item.cd_time >= 1.f)
				item.has_pick_cd = false;
			//get area of blocks to check
			int leftX = x / BLOCK_VISIBLE_SIZE - 1;
			int rightX = leftX + (int)(item.hitbox.size.x - 0.1) + 2; //hmm
			int bottomY = y / BLOCK_VISIBLE_SIZE - 1;
			int topY = bottomY + (int)(item.hitbox.size.y - 0.1) + 2; //hmm
			if (leftX < 0) leftX = 0;
			if (rightX >= world_width) rightX = world_width - 1;
			if (bottomY < 0) bottomY = 0;
			if (topY >= world_height) topY = world_height - 1;
			//check collisions
			int object_id;
			for (int i = leftX; i <= rightX; i++) {
				for (int j = bottomY; j <= topY; j++) {
					if (sprites_Array[i][j].object.object_type) {
						if (sprites_Array[i][j].object.object_type == isCompObjPart) {  //if part of complex object, then use its column and line
							object_id = sprites_Array[sprites_Array[i][j].object.component->get_column()][sprites_Array[i][j].object.component->get_line()].object.object_id;
						}
						else {  //if it's simple or complex object
							object_id = sprites_Array[i][j].object.object_id;
						}
						if (objectInfo[object_id]->allow_bottom_collision()) { //if only bottom collision is allowed
							if (CollisionManager::getTypeCollisionAABBwithBlock(item.hitbox, i, j, BLOCK_VISIBLE_SIZE) == BOTTOM) {
								item.time_in_free_falling = 0.f;
								item.hitbox.center.y = j * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + item.hitbox.size.y * 0.5f;
								item.fallingDistance = 0.f;
							}
						}
						else if (objectInfo[object_id]->allow_collision()) //if all types of collision are allowed
							switch (CollisionManager::getTypeCollisionAABBwithBlock(item.hitbox, i, j, BLOCK_VISIBLE_SIZE)) {
							case LEFT:
								if (!(is_solid_block(i + 1, j))) {
									item.hitbox.center.x = i * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + item.hitbox.size.x * 0.5f;
									item.Xinc = 0.f; //hmm
								}
								break;
							case RIGHT:
								item.hitbox.center.x = i * BLOCK_VISIBLE_SIZE - item.hitbox.size.x * 0.5f;
								item.Xinc = 0.f; //hmm
								break;
							case TOP:
								item.time_in_free_falling = 0.f;
								item.hitbox.center.y = j * BLOCK_VISIBLE_SIZE - item.hitbox.size.y * 0.5f;
								item.fallingDistance = 0.f;
								break;
							case BOTTOM:
								item.time_in_free_falling = 0.f;
								item.hitbox.center.y = j * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE + item.hitbox.size.y * 0.5f;
								item.fallingDistance = 0.f;
								item.Xinc = 0.f;
								break;
							case CORNER:
								break;
							default:
								break;
							}
					}
				}
			}
			//check if item collides with the player and try to pick it
			if (CollisionManager::checkCollisionAABB(player.hitbox, item.hitbox) && !item.has_pick_cd) {
				//if player picked the item and amount of entity item is 0, then delete it
				if (try_to_pick_item(item) && item.amount == 0) {
					dropped_items.erase(dropped_items.begin() + d);
					d--;
				}
			}
		}

		//draw hitbox
		if (active_weapon.isActive && active_weapon.hitboxIsActive) {
			glm::vec4 point(-BLOCK_VISIBLE_SIZE * 0.4f, -BLOCK_VISIBLE_SIZE, 0.f, 1.f);
			point = active_weapon.hitbox.transformMatrix * point;

			player_hitbox_buffer[0] = point.x;
			player_hitbox_buffer[1] = point.y;
			player_hitbox_buffer[2] = 1.f;
			player_hitbox_buffer[3] = 0.f;
			player_hitbox_buffer[4] = 0.f;
			player_hitbox_buffer[5] = 1.f;

			point = glm::vec4(-BLOCK_VISIBLE_SIZE * 0.4f, BLOCK_VISIBLE_SIZE, 0.f, 1.f);
			point = active_weapon.hitbox.transformMatrix * point;
			player_hitbox_buffer[6] = point.x;
			player_hitbox_buffer[7] = point.y;
			player_hitbox_buffer[8] = 1.f;
			player_hitbox_buffer[9] = 0.f;
			player_hitbox_buffer[10] = 0.f;
			player_hitbox_buffer[11] = 1.f;

			point = glm::vec4(BLOCK_VISIBLE_SIZE * 0.4f, BLOCK_VISIBLE_SIZE, 0.f, 1.f);
			point = active_weapon.hitbox.transformMatrix * point;
			player_hitbox_buffer[12] = point.x;
			player_hitbox_buffer[13] = point.y;
			player_hitbox_buffer[14] = 1.f;
			player_hitbox_buffer[15] = 0.f;
			player_hitbox_buffer[16] = 0.f;
			player_hitbox_buffer[17] = 1.f;

			point = glm::vec4(BLOCK_VISIBLE_SIZE * 0.4f, -BLOCK_VISIBLE_SIZE, 0.f, 1.f);
			point = active_weapon.hitbox.transformMatrix * point;
			player_hitbox_buffer[18] = point.x;
			player_hitbox_buffer[19] = point.y;
			player_hitbox_buffer[20] = 1.f;
			player_hitbox_buffer[21] = 0.f;
			player_hitbox_buffer[22] = 0.f;
			player_hitbox_buffer[23] = 1.f;
		}
		else if (entities.size()) {
			float w = entities[0]->hitbox.size.x;
			float h = entities[0]->hitbox.size.y;
			float lX = entities[0]->hitbox.center.x - w * 0.5f;
			float bY = entities[0]->hitbox.center.y - h * 0.5f;
			player_hitbox_buffer[0] = lX;
			player_hitbox_buffer[1] = bY;
			player_hitbox_buffer[2] = 1.f;
			player_hitbox_buffer[3] = 0.f;
			player_hitbox_buffer[4] = 0.f;
			player_hitbox_buffer[5] = 1.f;
			player_hitbox_buffer[6] = lX;
			player_hitbox_buffer[7] = bY + h;
			player_hitbox_buffer[8] = 1.f;
			player_hitbox_buffer[9] = 0.f;
			player_hitbox_buffer[10] = 0.f;
			player_hitbox_buffer[11] = 1.f;
			player_hitbox_buffer[12] = lX + w;
			player_hitbox_buffer[13] = bY + h;
			player_hitbox_buffer[14] = 1.f;
			player_hitbox_buffer[15] = 0.f;
			player_hitbox_buffer[16] = 0.f;
			player_hitbox_buffer[17] = 1.f;
			player_hitbox_buffer[18] = lX + w;
			player_hitbox_buffer[19] = bY;
			player_hitbox_buffer[20] = 1.f;
			player_hitbox_buffer[21] = 0.f;
			player_hitbox_buffer[22] = 0.f;
			player_hitbox_buffer[23] = 1.f;
		}
		else if (projectiles.size()) {
			float w = projectiles[0]->hitbox.size.x;
			float h = projectiles[0]->hitbox.size.y;
			float lX = projectiles[0]->hitbox.center.x - w * 0.5f;
			float bY = projectiles[0]->hitbox.center.y - h * 0.5f;
			player_hitbox_buffer[0] = lX;
			player_hitbox_buffer[1] = bY;
			player_hitbox_buffer[2] = 1.f;
			player_hitbox_buffer[3] = 0.f;
			player_hitbox_buffer[4] = 0.f;
			player_hitbox_buffer[5] = 1.f;
			player_hitbox_buffer[6] = lX;
			player_hitbox_buffer[7] = bY + h;
			player_hitbox_buffer[8] = 1.f;
			player_hitbox_buffer[9] = 0.f;
			player_hitbox_buffer[10] = 0.f;
			player_hitbox_buffer[11] = 1.f;
			player_hitbox_buffer[12] = lX + w;
			player_hitbox_buffer[13] = bY + h;
			player_hitbox_buffer[14] = 1.f;
			player_hitbox_buffer[15] = 0.f;
			player_hitbox_buffer[16] = 0.f;
			player_hitbox_buffer[17] = 1.f;
			player_hitbox_buffer[18] = lX + w;
			player_hitbox_buffer[19] = bY;
			player_hitbox_buffer[20] = 1.f;
			player_hitbox_buffer[21] = 0.f;
			player_hitbox_buffer[22] = 0.f;
			player_hitbox_buffer[23] = 1.f;
		}
		entity_vbo->bind_VBO();
		glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * 4, player_hitbox_buffer);

		if (active_weapon.projectile_attack_current_cd >= 0)
			active_weapon.projectile_attack_current_cd -= deltaTime;
		if (active_weapon.isActive)
			update_active_weapon(deltaTime);
		if (!mouse.left_button && !active_weapon.isActive) {
			active_breakable_object.time_breaking = 0.f;
		}

		//change look of active slot in ui bar
		if (!inventoryIsOpen)
			recolor_active_bar_slot();

		additional_slots = 0;
		additional_sprites_slots = 0;
		if (inventoryIsOpen) {
			int offset = 264;
			if (active_chest.isOpen) {
				for (int i = 0; i < 160; i++) {
					inventory_vert_buf[i + offset] = inv_chest_slots_buf[i];
				}
				additional_slots += 40;
				offset += 160;
			}
			for (int i = 0; i < 20; i++) {
				inventory_vert_buf[i + offset] = crafting_system.main_slots[i];
			}
			offset += 20;
			additional_slots += 5;

			crafting_system.available_crafts = get_available_crafts();
			int crafts_available = crafting_system.available_crafts.size();
			int SIZE = crafts_available * 4;
			if (crafts_available > 0) {
				if (crafting_system.index_of_current_craft >= crafts_available)
					crafting_system.index_of_current_craft = crafts_available - 1;
				//sprites in main slots
				additional_sprites_slots++;
				if (crafting_system.index_of_current_craft - 1 >= 0) {
					additional_sprites_slots++;
					if (crafting_system.index_of_current_craft - 2 >= 0)
						additional_sprites_slots++;
				}
				if (crafting_system.index_of_current_craft + 1 < crafts_available) {
					additional_sprites_slots++;
					if (crafting_system.index_of_current_craft + 2 < crafts_available)
						additional_sprites_slots++;
				}
				//change current craftable item in main slot with mouse wheel
				if (mouse.mouseX >= crafting_system.main_slots[0].vertices.x && mouse.mouseX <= crafting_system.main_slots[2].vertices.x)
					if (mouse.mouseY >= crafting_system.main_slots[0].vertices.y && mouse.mouseY <= crafting_system.main_slots[1].vertices.y)
					{
						if (mouse.wheelOffset != 0)
							if (mouse.wheelOffset < 0) {
								crafting_system.index_of_current_craft++;
								if (crafting_system.index_of_current_craft == crafts_available)
									crafting_system.index_of_current_craft = 0;
							}
							else {
								crafting_system.index_of_current_craft--;
								if (crafting_system.index_of_current_craft < 0)
									crafting_system.index_of_current_craft = crafts_available - 1;
							}
					}
				//calculate amount of sprites and slots for item to craft
				int items_needed_size = craftable_items[crafting_system.available_crafts[crafting_system.index_of_current_craft]].items_needed.size() * 4;
				for (int i = 0; i < items_needed_size; i++) {
					inventory_vert_buf[i + offset] = crafting_system.needed_items_slots[i];
				}
				offset += items_needed_size;
				additional_slots += items_needed_size / 4;
				additional_sprites_slots += items_needed_size / 4;

				//show all crafts available with helper slots (count amount of slots and sprites)
				if (crafting_system.show_all_crafts) {
					for (int i = 0; i < SIZE; i++) {
						inventory_vert_buf[i + offset] = crafting_system.helper_slots[i];
					}
					additional_slots += crafts_available;
					additional_sprites_slots += crafts_available;
				}
			}
		}

		inventory_vbo->bind_VBO();
		if (active_chest.isOpen)
			glBufferSubData(GL_ARRAY_BUFFER, 0, INVENTORY_SIZE * sizeof(ColorVertex2f) * 4, inventory_vert_buf);
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, (INVENTORY_SIZE - 40) * sizeof(ColorVertex2f) * 4, inventory_vert_buf);
		//change back
		reset_active_bar_slot();
		object_info_box[0].show_box = false;
		object_info_box[1].show_box = false;
		//move objects in inventory
		if (inventoryIsOpen) {
			int size = INVENTORY_SIZE - 40 - CRAFTING_SLOTS;
			for (int i = 0; i < size; i++) {  //0,1 6,7 12,13 18,19
				if (mouse.mouseX >= inventory_vert_buf[i * 4].vertices.x && mouse.mouseX <= inventory_vert_buf[i * 4 + 2].vertices.x)
					if (mouse.mouseY >= inventory_vert_buf[i * 4].vertices.y && mouse.mouseY <= inventory_vert_buf[i * 4 + 1].vertices.y) {
						ObjectType type = inventory_array[i].object.type;
						int id = inventory_array[i].object.id;

						if (type && !currently_moving_object.object.type) {
							update_item_info_box(type, id);
						}
						if (mouse.left_button) {
							mouse.left_button = false;
							if (currently_moving_object.object.type) { //if mouse holds an object
								if (type) { //if there is an object in slot
									if (id == currently_moving_object.object.id && item_is_stackable(id, type)) { //if the same id
										int amount = inventory_array[i].amount + currently_moving_object.amount;
										if (amount > 999) {
											inventory_array[i].amount = 999;
											currently_moving_object.amount = amount - 999;
										}
										else {
											inventory_array[i].amount = amount;
											currently_moving_object.object = ItemObject(None, 0);
											currently_moving_object.amount = 0;
										}
										break;
									}
									else {  //if different id
										int amount = inventory_array[i].amount;
										//ObjectType type = inventory_array[i].object.get_type();
										inventory_array[i].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
										inventory_array[i].amount = currently_moving_object.amount;
										currently_moving_object.object = ItemObject(type, id);
										currently_moving_object.amount = amount;
										break;
									}
								}
								else { //if there is no object in slot
									//add object from currently moving object to inventory slot
									inventory_array[i].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
									inventory_array[i].amount = currently_moving_object.amount;
									//delete it from the currently moving object
									currently_moving_object.object = ItemObject(None, 0);
									currently_moving_object.amount = 0;
									break;
								}
							}
							else if (type) { //if mouse doesn't hold an object
								//add object to currently moving object
								currently_moving_object.object = ItemObject(type, id);
								currently_moving_object.amount = inventory_array[i].amount;
								//delete it from the inventory slot
								inventory_array[i].object = ItemObject(None, 0);
								inventory_array[i].amount = 0;
								index_of_last_slot_picked = i;
								break;
							}
						}
						//manage objects in inventory (add one to another (if they are the same and it is allowed) or subdivide amount)
						if (mouse.right_button) {
							mouse.right_button = false;
							if (type && id == currently_moving_object.object.id && item_is_stackable(id, type)) { //if there is an object
								if (inventory_array[i].amount < 999) {
									inventory_array[i].amount++;
									currently_moving_object.amount--;
									if (currently_moving_object.amount == 0) {
										currently_moving_object.object = ItemObject(None, 0);
									}
								}
								break;
							}
							else if (!type) { //if there is no object
								inventory_array[i].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
								inventory_array[i].amount++;
								currently_moving_object.amount--;
								if (currently_moving_object.amount == 0) {
									currently_moving_object.object = ItemObject(None, 0);
								}
								break;
							}
						}
					}
			}
			if (active_chest.isOpen) { //if any chest is open right now
				for (int i = size; i < INVENTORY_SIZE - CRAFTING_SLOTS; i++) //check each of 40 slots in chest, size here is exactly max size - 40
					if (mouse.mouseX >= inventory_vert_buf[i * 4].vertices.x && mouse.mouseX <= inventory_vert_buf[i * 4 + 2].vertices.x)
						if (mouse.mouseY >= inventory_vert_buf[i * 4].vertices.y && mouse.mouseY <= inventory_vert_buf[i * 4 + 1].vertices.y) {
							ObjectType type = active_chest.slot_pointer[i - size].object.type;
							int id = active_chest.slot_pointer[i - size].object.id;

							if (type && !currently_moving_object.object.type) {
								update_item_info_box(type, id);
							}
							if (mouse.left_button) {
								mouse.left_button = false;
								if (currently_moving_object.object.type) { //if mouse holds an object
									if (type) { //if there is an object in slot
										if (id == currently_moving_object.object.id && item_is_stackable(id, type)) { //if the same id
											int amount = active_chest.slot_pointer[i - size].amount + currently_moving_object.amount;
											if (amount > 999) {
												active_chest.slot_pointer[i - size].amount = 999;
												currently_moving_object.amount = amount - 999;
											}
											else {
												active_chest.slot_pointer[i - size].amount = amount;
												currently_moving_object.object = ItemObject(None, 0);
												currently_moving_object.amount = 0;
											}
											break;
										}
										else {  //if different id
											int amount = active_chest.slot_pointer[i - size].amount;
											active_chest.slot_pointer[i - size].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
											active_chest.slot_pointer[i - size].amount = currently_moving_object.amount;
											currently_moving_object.object = ItemObject(type, id);
											currently_moving_object.amount = amount;
											break;
										}
									}
									else { //if there is no object in slot
										//add object from currently moving object to inventory slot
										active_chest.slot_pointer[i - size].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
										active_chest.slot_pointer[i - size].amount = currently_moving_object.amount;
										//delete it from the currently moving object
										currently_moving_object.object = ItemObject(None, 0);
										currently_moving_object.amount = 0;
										break;
									}
								}
								else if (type) { //if mouse doesn't hold an object
									//add object to currently moving object
									currently_moving_object.object = ItemObject(type, id);
									currently_moving_object.amount = active_chest.slot_pointer[i - size].amount;
									//delete it from the inventory slot
									active_chest.slot_pointer[i - size].object = ItemObject(None, 0);
									active_chest.slot_pointer[i - size].amount = 0;
									index_of_last_slot_picked = i;
									break;
								}
							}
							//manage objects in inventory (add one to another (if they are the same and it is allowed) or subdivide amount)
							if (mouse.right_button) {
								mouse.right_button = false;
								if (type && id == currently_moving_object.object.id && item_is_stackable(id, type)) { //if there is an object
									if (active_chest.slot_pointer[i - size].amount < 999) {
										active_chest.slot_pointer[i - size].amount++;
										currently_moving_object.amount--;
										if (currently_moving_object.amount == 0) {
											currently_moving_object.object = ItemObject(None, 0);
										}
									}
									break;
								}
								else if (!type) { //if there is no object
									active_chest.slot_pointer[i - size].object = ItemObject(currently_moving_object.object.type, currently_moving_object.object.id);
									active_chest.slot_pointer[i - size].amount++;
									currently_moving_object.amount--;
									if (currently_moving_object.amount == 0) {
										currently_moving_object.object = ItemObject(None, 0);
									}
									break;
								}
							}
						}
			}
		}
		else if (inventory_array[active_bar_slot].object.type && !currently_moving_object.object.type) { //if there is an object in active slot, then show its name
			object_info_box[0].show_box = true;
			//show info box about this object
			object_info_box[0].box_string = "";
			object_info_box[0].box_string += objectInfo[inventory_array[active_bar_slot].object.id]->name;
			object_info_box[0].starting_pos = glm::vec2(ScreenWidth * 0.01, ScreenHeight * 0.985);
		}

		int array_size = 0;
		int object_id;
		ObjectType object_type;
		buffer_size = 0;
		Vertex2f* buffer_ptr;
		inventory_text_size = 0;
		if (inventoryIsOpen)
			size = INVENTORY_SIZE - 40 - CRAFTING_SLOTS;
		else
			size = 10;
		for (int i = 0; i < size; i++) {
			if (inventory_array[i].object.type) {
				array_size++;
			}
		}
		if (active_chest.isOpen)
			for (int i = size; i < INVENTORY_SIZE - CRAFTING_SLOTS; i++) {
				if (active_chest.slot_pointer[i - size].object.type) array_size++;
			}
		if (currently_moving_object.object.type)
			array_size++;
		array_size += additional_sprites_slots;
		inventory_index_size = array_size * 6;
		int index = 0;
		//fill buffer for inventory objects
		buffer_ptr = inventory_obj_vert_buf;
		float dX, dY; //adjust ratio of sprites
		float X, Y;
		for (int i = 0; i < size; i++) {
			dX = 1.f, dY = 1.f;
			if (!inventory_array[i].object.type) {
				continue;
			}
			if (inventory_array[i].amount > 1) {
				inventory_text_info[index * 3] = inventory_array[i].amount;
				inventory_text_info[index * 3 + 1] = inventory_vert_buf[i * 4].vertices.x;
				inventory_text_info[index * 3 + 2] = inventory_vert_buf[i * 4].vertices.y;
				index++;
				inventory_text_size++;
			}
			object_id = inventory_array[i].object.id;
			object_type = inventory_array[i].object.type;
			if (object_type == isComplexObject || object_type == isWeapon) {
				float sizeX = objectInfo[object_id]->get_sizeX();
				float sizeY = objectInfo[object_id]->get_sizeY();
				if (sizeX > sizeY)
					dY = sizeY / sizeX;
				else
					dX = sizeX / sizeY;
			}
			X = inventory_vert_buf[i * 4].vertices.x + ScreenHeight * (0.005 + 0.04 * ((1.f - dX) / 2.f));
			Y = inventory_vert_buf[i * 4].vertices.y + ScreenHeight * (0.005 + 0.04 * ((1.f - dY) / 2.f));
			tex_coords_ptr = objectInfo[object_id]->texture_coords;
			buffer_ptr->vertices = { X, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[0];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X, Y + ScreenHeight * 0.04 * dY };
			buffer_ptr->tex_vertices = tex_coords_ptr[1];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y + ScreenHeight * 0.04 * dY };
			buffer_ptr->tex_vertices = tex_coords_ptr[2];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[3];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_size += 4;
		}
		if (active_chest.isOpen)
			for (int i = size; i < INVENTORY_SIZE - CRAFTING_SLOTS; i++) {
				dX = 1.f, dY = 1.f;
				if (!active_chest.slot_pointer[i - size].object.type) {
					continue;
				}
				if (active_chest.slot_pointer[i - size].amount > 1) {
					inventory_text_info[index * 3] = active_chest.slot_pointer[i - size].amount;
					inventory_text_info[index * 3 + 1] = inventory_vert_buf[i * 4].vertices.x;
					inventory_text_info[index * 3 + 2] = inventory_vert_buf[i * 4].vertices.y;
					index++;
					inventory_text_size++;
				}
				object_id = active_chest.slot_pointer[i - size].object.id;
				object_type = active_chest.slot_pointer[i - size].object.type;
				if (object_type == isComplexObject || object_type == isWeapon) {
					float sizeX = objectInfo[object_id]->get_sizeX();
					float sizeY = objectInfo[object_id]->get_sizeY();
					if (sizeX > sizeY)
						dY = sizeY / sizeX;
					else
						dX = sizeX / sizeY;
				}
				X = inventory_vert_buf[i * 4].vertices.x + ScreenHeight * (0.005 + 0.04 * ((1.f - dX) / 2.f));
				Y = inventory_vert_buf[i * 4].vertices.y + ScreenHeight * (0.005 + 0.04 * ((1.f - dY) / 2.f));
				tex_coords_ptr = objectInfo[object_id]->texture_coords;
				buffer_ptr->vertices = { X, Y };
				buffer_ptr->tex_vertices = tex_coords_ptr[0];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X, Y + ScreenHeight * 0.04 * dY };
				buffer_ptr->tex_vertices = tex_coords_ptr[1];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y + ScreenHeight * 0.04 * dY };
				buffer_ptr->tex_vertices = tex_coords_ptr[2];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y };
				buffer_ptr->tex_vertices = tex_coords_ptr[3];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_size += 4;
			}
		//add effects icons if player has them and inventory is closed
		if (!inventoryIsOpen && player.effects.size()) {
			int size = player.effects.size();
			for (int i = 0; i < size; i++) {
				X = inventory_vert_buf[(i + 10) * 4].vertices.x;
				Y = inventory_vert_buf[(i + 10) * 4].vertices.y;
				if (mouse.mouseX >= inventory_vert_buf[(i + 10) * 4].vertices.x * 0.8 && mouse.mouseX <= inventory_vert_buf[(i + 10) * 4 + 2].vertices.x * 0.8)
					if (mouse.mouseY >= inventory_vert_buf[(i + 10) * 4].vertices.y * 0.8 + ScreenHeight * 0.2f && mouse.mouseY <= inventory_vert_buf[(i + 10) * 4 + 1].vertices.y * 0.8 + ScreenHeight * 0.2f) {
						update_effect_info_box(player.effects[i]);
					}
				int effect_id = player.effects[i].id;
				tex_coords_ptr = effects[effect_id]->sprite_coords;
				buffer_ptr->vertices = { X, Y };
				buffer_ptr->tex_vertices = tex_coords_ptr[0];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X, Y + ScreenHeight * 0.05 };
				buffer_ptr->tex_vertices = tex_coords_ptr[1];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X + ScreenHeight * 0.05, Y + ScreenHeight * 0.05 };
				buffer_ptr->tex_vertices = tex_coords_ptr[2];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X + ScreenHeight * 0.05, Y };
				buffer_ptr->tex_vertices = tex_coords_ptr[3];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_size += 4;
				inventory_index_size += 6;
			}
		}
		//add additional slots and sprites for crafting system and manage them
		if (inventoryIsOpen) {
			float X, Y;
			float dX = 1.f, dY = 1.f; //to adjust ratio for complex sprites
			int id = crafting_system.available_crafts[crafting_system.index_of_current_craft];
			int needed_sprites_size = craftable_items[id].items_needed.size();
			for (int i = 0; i < needed_sprites_size; i++) {
				dX = 1.f, dY = 1.f;
				if (craftable_items[id].items_needed[i].amount > 1) {
					inventory_text_info[index * 3] = craftable_items[id].items_needed[i].amount;
					inventory_text_info[index * 3 + 1] = crafting_system.needed_items_slots[i * 4].vertices.x;
					inventory_text_info[index * 3 + 2] = crafting_system.needed_items_slots[i * 4].vertices.y;
					index++;
					inventory_text_size++;
				}
				object_type = objectInfo[object_id]->objectType;
				if (object_type == isComplexObject || object_type == isWeapon) {
					float sizeX = objectInfo[object_id]->get_sizeX();
					float sizeY = objectInfo[object_id]->get_sizeY();
					if (sizeX > sizeY)
						dY = sizeY / sizeX;
					else
						dX = sizeX / sizeY;
				}
				object_id = craftable_items[id].items_needed[i].id;
				
				X = crafting_system.needed_items_slots[i * 4].vertices.x + ScreenHeight * (0.005 + 0.04 * ((1.f - dX) / 2.f));
				Y = crafting_system.needed_items_slots[i * 4].vertices.y + ScreenHeight * (0.005 + 0.04 * ((1.f - dY) / 2.f));
				if (mouse.mouseX >= X && mouse.mouseX <= X + ScreenHeight * 0.04 &&
					mouse.mouseY >= Y && mouse.mouseY <= Y + ScreenHeight * 0.04)
					update_item_info_box(objectInfo[object_id]->objectType, object_id);

				tex_coords_ptr = objectInfo[object_id]->texture_coords;
				buffer_ptr->vertices = { X, Y };
				buffer_ptr->tex_vertices = tex_coords_ptr[0];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X, Y + ScreenHeight * 0.04 * dY };
				buffer_ptr->tex_vertices = tex_coords_ptr[1];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y + ScreenHeight * 0.04 * dY };
				buffer_ptr->tex_vertices = tex_coords_ptr[2];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y };
				buffer_ptr->tex_vertices = tex_coords_ptr[3];
				buffer_ptr->tex_id = 0.f;
				buffer_ptr++;
				buffer_size += 4;
			}
			//add items in mains slots
			if (craftable_items[id].craftable_amount > 1) {
				inventory_text_info[index * 3] = craftable_items[id].craftable_amount;
				inventory_text_info[index * 3 + 1] = crafting_system.main_slots[0].vertices.x;
				inventory_text_info[index * 3 + 2] = crafting_system.main_slots[0].vertices.y;
				index++;
				inventory_text_size++;
			}
			object_id = craftable_items[id].item_id;
			dX = 1.f, dY = 1.f;
			object_type = objectInfo[object_id]->objectType;
			if (object_type == isComplexObject || object_type == isWeapon) {
				float sizeX = objectInfo[object_id]->get_sizeX();
				float sizeY = objectInfo[object_id]->get_sizeY();
				if (sizeX > sizeY)
					dY = sizeY / sizeX;
				else
					dX = sizeX / sizeY;
			}
			X = crafting_system.main_slots[0].vertices.x + ScreenHeight * (0.005 + 0.04 * ((1.f - dX) / 2.f));
			Y = crafting_system.main_slots[0].vertices.y + ScreenHeight * (0.005 + 0.04 * ((1.f - dY) / 2.f));
			if (mouse.mouseX >= X && mouse.mouseX <= X + ScreenHeight * 0.065 &&
				mouse.mouseY >= Y && mouse.mouseY <= Y + ScreenHeight * 0.065)
			{
				update_item_info_box(objectInfo[object_id]->objectType, object_id);
				if (mouse.left_button) {
					mouse.left_button = false;
					craft_item(id);
				}
			}
			tex_coords_ptr = objectInfo[object_id]->texture_coords;
			buffer_ptr->vertices = { X, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[0];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X, Y + ScreenHeight * 0.06 * dY };
			buffer_ptr->tex_vertices = tex_coords_ptr[1];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X + ScreenHeight * 0.06 * dX, Y + ScreenHeight * 0.06 * dY };
			buffer_ptr->tex_vertices = tex_coords_ptr[2];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X + ScreenHeight * 0.06 * dX, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[3];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_size += 4;

			needed_sprites_size = crafting_system.available_crafts.size();
			if (crafting_system.index_of_current_craft - 1 >= 0) {
				update_main_crafting_slot(4, crafting_system.index_of_current_craft - 1, index, buffer_ptr);
				if (crafting_system.index_of_current_craft - 2 >= 0) {
					update_main_crafting_slot(8, crafting_system.index_of_current_craft - 2, index, buffer_ptr);
				}
			}
			if (crafting_system.index_of_current_craft + 1 < needed_sprites_size) {
				update_main_crafting_slot(12, crafting_system.index_of_current_craft + 1, index, buffer_ptr);
				if (crafting_system.index_of_current_craft + 2 < needed_sprites_size) {
					update_main_crafting_slot(16, crafting_system.index_of_current_craft + 2, index, buffer_ptr);
				}
			}

			//add helper sprites if needed
			if (crafting_system.show_all_crafts) {
				for (int i = 0; i < needed_sprites_size; i++) {
					dX = 1.f, dY = 1.f;
					id = crafting_system.available_crafts[i];
					if (craftable_items[id].craftable_amount > 1) {
						inventory_text_info[index * 3] = craftable_items[id].craftable_amount;
						inventory_text_info[index * 3 + 1] = crafting_system.helper_slots[i * 4].vertices.x;
						inventory_text_info[index * 3 + 2] = crafting_system.helper_slots[i * 4].vertices.y;
						index++;
						inventory_text_size++;
					}
					object_id = craftable_items[id].item_id;
					object_type = objectInfo[object_id]->objectType;
					if (object_type == isComplexObject || object_type == isWeapon) {
						float sizeX = objectInfo[object_id]->get_sizeX();
						float sizeY = objectInfo[object_id]->get_sizeY();
						if (sizeX > sizeY)
							dY = sizeY / sizeX;
						else
							dX = sizeX / sizeY;
					}
					X = crafting_system.helper_slots[i * 4].vertices.x + ScreenHeight * (0.005 + 0.04 * ((1.f - dX) / 2.f));
					Y = crafting_system.helper_slots[i * 4].vertices.y + ScreenHeight * (0.005 + 0.04 * ((1.f - dY) / 2.f));
					if (mouse.mouseX >= X && mouse.mouseX <= X + ScreenHeight * 0.04 &&
						mouse.mouseY >= Y && mouse.mouseY <= Y + ScreenHeight * 0.04)
					{
						if (mouse.left_button) {
							mouse.left_button = false;
							crafting_system.index_of_current_craft = i;
						}
						update_item_info_box(objectInfo[object_id]->objectType, object_id);
					}
					tex_coords_ptr = objectInfo[object_id]->texture_coords;
					buffer_ptr->vertices = { X, Y };
					buffer_ptr->tex_vertices = tex_coords_ptr[0];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;
					buffer_ptr->vertices = { X, Y + ScreenHeight * 0.04 * dY };
					buffer_ptr->tex_vertices = tex_coords_ptr[1];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;
					buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y + ScreenHeight * 0.04 * dY };
					buffer_ptr->tex_vertices = tex_coords_ptr[2];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;
					buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y };
					buffer_ptr->tex_vertices = tex_coords_ptr[3];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;
					buffer_size += 4;
				}
			}
		}
		if (currently_moving_object.object.type) {
			dX = 1.f, dY = 1.f;
			if (currently_moving_object.amount > 1) {
				inventory_text_info[index * 3] = currently_moving_object.amount;
				inventory_text_info[index * 3 + 1] = mouse.mouseX - ScreenHeight * 0.0275;
				inventory_text_info[index * 3 + 2] = mouse.mouseY - ScreenHeight * 0.0275;
				inventory_text_size++;
			}

			float X = mouse.mouseX - ScreenHeight * 0.02;
			float Y = mouse.mouseY - ScreenHeight * 0.02;
			object_id = currently_moving_object.object.id;
			object_type = currently_moving_object.object.type;
			if (object_type == isComplexObject || object_type == isWeapon) {
				float sizeX = objectInfo[object_id]->get_sizeX();
				float sizeY = objectInfo[object_id]->get_sizeY();
				if (sizeX > sizeY)
					dY = sizeY / sizeX;
				else
					dX = sizeX / sizeY;
			}
			tex_coords_ptr = objectInfo[object_id]->texture_coords;
			buffer_ptr->vertices = { X, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[0];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X, Y + ScreenHeight * 0.04 * dY };
			buffer_ptr->tex_vertices = tex_coords_ptr[1];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y + ScreenHeight * 0.04 * dY };
			buffer_ptr->tex_vertices = tex_coords_ptr[2];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
			buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[3];
			buffer_ptr->tex_id = 0.f;
			buffer_size += 4;
		}
		inventory_objects_vbo->bind_VBO();
		glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size * sizeof(Vertex2f), inventory_obj_vert_buf);

		//calculate the borders of current area to be drawn based on the camera(player) position
		int player_X = (ScreenWidth / 2 - camera.dX) / BLOCK_VISIBLE_SIZE;
		int player_Y = (ScreenHeight / 2 - camera.dY) / BLOCK_VISIBLE_SIZE;
		int leftX = player_X - 25 * (1 / camera.scaling); //25 blocks to both sides on oX
		int rightX = player_X + 26 * (1 / camera.scaling);
		int bottomY = player_Y - 15 * (1 / camera.scaling); //15 blocks to both sides on oY
		int topY = player_Y + 16 * (1 / camera.scaling);
		if (leftX < 0) leftX = 0;
		if (rightX > world_width ) rightX = world_width;
		if (bottomY < 0) bottomY = 0;
		if (topY > world_height ) topY = world_height;

		//break or put objects and walls, use weapons, tools, etc. with mouse left button
		if (mouse.left_button && inventory_array[active_bar_slot].object.type) {
			ObjectType type = inventory_array[active_bar_slot].object.type;
			if (type == isWeapon) { //if the player is holding a weapon (pickaxe, axe, etc.)
				Smart_ptr<ObjectInfo>& weapon = objectInfo[inventory_array[active_bar_slot].object.id];
				WeaponType weapon_type = weapon->get_weapon_type();
				//start weapon animation if not active
				if (!active_weapon.isActive) {
					activate_weapon(weapon_type);
				}
				if (weapon_type == isPickaxe) {
					//the first step is to calculate how many blocks based on cameraDx/y and dx/y, then how many blocks on the camera based on resolution and scaling with current mouse coordinates
					int line = (-camera.dY - camera.scalingDy) / BLOCK_VISIBLE_SIZE + mouse.mouseY / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
					int column = (-camera.dX - camera.scalingDx) / BLOCK_VISIBLE_SIZE + mouse.mouseX / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
					if (sprites_Array[column][line].object.object_type) {
						if (sprites_Array[column][line].object.object_type == isCompObjPart) {  //if is a part of complex object, then get its column and line
							int new_column = sprites_Array[column][line].object.component->get_column();
							int new_line = sprites_Array[column][line].object.component->get_line();
							column = new_column;
							line = new_line;
						}
						object_id = sprites_Array[column][line].object.object_id;
						//update or start breaking object
						if (active_breakable_object.isBreaking) {
							//if object is breaked then remove and drop it
							if (active_breakable_object.update_breaking_object(column, line, deltaTime)) {
								//drop item
								drop_item(objectInfo[object_id]->get_drop_object_id(), column * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, line * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, 1, 0.f, false, 0);
								//remove object
								if (sprites_Array[column][line].object.object_type == isBlock) {
									destroy_object(column, line);
								}
								else {
									destroy_complex_object(column, line);
								}
							}
						}
						else {
							active_breakable_object.start_breaking_object(column, line, objectInfo[object_id]->get_toughness() / (weapon->get_speed_factor() * weapon->get_instrument_power()));
						}
					}
				}
				else if (weapon_type == isAxe) {
					//the first step is to calculate how many blocks based on cameraDx/y and dx/y, then how many blocks on the camera based on resolution and scaling with current mouse coordinates
					int line = (-camera.dY - camera.scalingDy) / BLOCK_VISIBLE_SIZE + mouse.mouseY / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
					int column = (-camera.dX - camera.scalingDx) / BLOCK_VISIBLE_SIZE + mouse.mouseX / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
					//check if it is any kind of wood
					if (sprites_Array[column][line].object.object_type) {
						object_id = sprites_Array[column][line].object.object_id;
						if (sprites_Array[column][line].object.object_type == isBlock &&
							objectInfo[object_id]->get_block_type() == isWood ||
							objectInfo[object_id]->get_block_type() == isTreeTop)
						{
							if (active_breakable_object.isBreaking) {
								//if object is breaked then remove and drop it
								if (active_breakable_object.update_breaking_object(column, line, deltaTime)) {
									//drop item
									drop_item(objectInfo[object_id]->get_drop_object_id(), column * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, line * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, 1, 0.f, false, 0);
									//remove object
									sprites_Array[column][line].object = GameObject(None, 0);
								}
							}
							else {
								active_breakable_object.start_breaking_object(column, line, objectInfo[object_id]->get_toughness() / (weapon->get_speed_factor() * weapon->get_instrument_power()));
							}
						}
					}
				}
				else if (weapon_type == isHammer) {
					//the first step is to calculate how many blocks based on cameraDx/y and dx/y, then how many blocks on the camera based on resolution and scaling with current mouse coordinates
					int line = (-camera.dY - camera.scalingDy) / BLOCK_VISIBLE_SIZE + mouse.mouseY / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
					int column = (-camera.dX - camera.scalingDx) / BLOCK_VISIBLE_SIZE + mouse.mouseX / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
					if (sprites_Array[column][line].wall_id) { //if there is a wall in this slot
						if (active_breakable_object.isBreaking) {
							//if object is breaked then remove and drop it
							if (active_breakable_object.update_breaking_object(column, line, deltaTime)) {
								//drop item
								drop_item(sprites_Array[column][line].wall_id, column * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, line * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, 1, 0.f, false, 0);
								//remowe wall
								destroy_wall(column, line);
							}
						}
						else {
							active_breakable_object.start_breaking_object(column, line, objectInfo[object_id]->get_toughness() / (weapon->get_speed_factor() * weapon->get_instrument_power()));
						}
					}
				}
			}
			else if (type == isWall) { //if the player is holding a wall, then he can put it if it is possible
				//the first step is to calculate how many blocks based on cameraDx/y and dx/y, then how many blocks on the camera based on resolution and scaling with current mouse coordinates
				int line = (-camera.dY - camera.scalingDy) / BLOCK_VISIBLE_SIZE + mouse.mouseY / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
				int column = (-camera.dX - camera.scalingDx) / BLOCK_VISIBLE_SIZE + mouse.mouseX / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
				if (!sprites_Array[column][line].wall_id) { //if there is no wall id (means that it is equal to 0 and there is no wall in this slot) 
					//if there is no wall then put new wall there
					set_wall(column, line, inventory_array[active_bar_slot].object.id);
					remove_inventory_item(active_bar_slot);
				}
			}
			else if (type == isConsumable || type == isPotion) {
				mouse.left_button = false;
				use_item_with_effect(active_bar_slot);
				audio_manager->play_Sound(1);
			}
			else { //if the player is holding a simple or complex object, then he can put it if it is possible
				int line = (-camera.dY - camera.scalingDy) / BLOCK_VISIBLE_SIZE + mouse.mouseY / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
				int column = (-camera.dX - camera.scalingDx) / BLOCK_VISIBLE_SIZE + mouse.mouseX / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
				if (!sprites_Array[column][line].object.object_type) {
					//if it's a simple object
					if (inventory_array[active_bar_slot].object.type == isBlock) {
						//check if there are any blocks near or a wall
						if (sprites_Array[column][line + 1].object.object_type || sprites_Array[column][line - 1].object.object_type ||
							sprites_Array[column + 1][line].object.object_type || sprites_Array[column - 1][line].object.object_type || sprites_Array[column][line].wall_id)
						{
							object_id = inventory_array[active_bar_slot].object.id;
							//check if it's a platform or a torch and use suitable sprite based on how it should be placed
							if (objectInfo[object_id]->get_block_type() == isPlatform || objectInfo[object_id]->get_block_type() == isTorch) {
								if (column - 1 >= 0 && is_solid_block(column - 1, line))
									object_id += 1;
								else if (column + 1 < world_width && is_solid_block(column + 1, line))
									object_id += 2;
							}
							set_block(column, line, object_id);
							remove_inventory_item(active_bar_slot);
						}
					}
					//if it's a complex object
					else if (inventory_array[active_bar_slot].object.type == isComplexObject) {
						bool enoughSpace = true;
						object_id = inventory_array[active_bar_slot].object.id;
						int width = column + objectInfo[object_id]->get_sizeX(), height = line + objectInfo[object_id]->get_sizeY();
						//check if enough space to put the object
						for (int i = column; i < width; i++)
							for (int j = line; j < height; j++) {
								if (sprites_Array[i][j].object.object_type) enoughSpace = false;
							}
						if (enoughSpace) {  //if enough space, then can put the object
							mouse.left_button = false;
							ComplexObjectType type = objectInfo[object_id]->get_comp_obj_type();
							if (type == isChest)
								sprites_Array[column][line].object = GameObject(isComplexObject, object_id, new ChestComponent);
							else if (type == isDoor)
								sprites_Array[column][line].object = GameObject(isComplexObject, object_id, new DoorComponent(0));
							else
								sprites_Array[column][line].object = GameObject(isComplexObject, object_id);

							//fill the rest space with "parts of the object"
							for (int i = column; i < width; i++)
								for (int j = line; j < height; j++) {
									if (!sprites_Array[i][j].object.object_type)
										sprites_Array[i][j].object = GameObject(isCompObjPart, 0, new ComplexObjectPartComponent(column, line));
								}
							if (objectInfo[object_id]->emitsLight) {
								update_lighting(glm::vec2((column + objectInfo[object_id]->get_sizeX() * 0.5f) * BLOCK_VISIBLE_SIZE, (line + objectInfo[object_id]->get_sizeY() * 0.5f) * BLOCK_VISIBLE_SIZE),
									objectInfo[object_id]->light_color, objectInfo[object_id]->light_radius * BLOCK_VISIBLE_SIZE, true);
							}

							remove_inventory_item(active_bar_slot);
						}
					}
				}
				else if (objectInfo[inventory_array[active_bar_slot].object.id]->effectId != -1) {
					mouse.left_button = false;
					use_item_with_effect(active_bar_slot);
				}
			}
		}
		//use specific objects (chests, doors, etc.) or throw items with mouse right button
		if (mouse.right_button) {
			mouse.right_button = false;
			if (currently_moving_object.object.type) { //SHOULD MAKE IT BETTER!!!!
				if (player.looks_at_left) {
					drop_item(currently_moving_object.object.id, player.hitbox.center.x,
						player.hitbox.center.y + player.hitbox.size.y * 0.25, currently_moving_object.amount, -3 * BLOCK_VISIBLE_SIZE, true, 1.f);
				}
				else {
					drop_item(currently_moving_object.object.id, player.hitbox.center.x,
						player.hitbox.center.y + player.hitbox.size.y * 0.25, currently_moving_object.amount, 3 * BLOCK_VISIBLE_SIZE, true, 1.f);
				}
				currently_moving_object.object = ItemObject(None, 0);
				currently_moving_object.amount = 0;
			}
			int line = (-camera.dY - camera.scalingDy) / BLOCK_VISIBLE_SIZE + mouse.mouseY / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
			int column = (-camera.dX - camera.scalingDx) / BLOCK_VISIBLE_SIZE + mouse.mouseX / BLOCK_VISIBLE_SIZE * (1 / camera.scaling);
			if (sprites_Array[column][line].object.object_type) {
				if (sprites_Array[column][line].object.object_type == isCompObjPart) {  //if is a part of complex object, then get its column and line
					int new_column = sprites_Array[column][line].object.component->get_column();
					int new_line = sprites_Array[column][line].object.component->get_line();
					column = new_column;
					line = new_line;
				}
				object_id = sprites_Array[column][line].object.object_id;
				if (sprites_Array[column][line].object.object_type == isComplexObject) {
					if (objectInfo[object_id]->get_comp_obj_type() == isChest) { //open/close chest
						if (active_chest.isOpen) {
							active_chest.isOpen = false;
							active_chest.slot_pointer = nullptr;
							inventoryIsOpen = false;
							//crafting_system.show_all_crafts = false;
						}
						else {
							active_chest.isOpen = true;
							active_chest.slot_pointer = sprites_Array[column][line].object.component->get_chest_slots();
							inventoryIsOpen = true;
							///crafting_system.show_all_crafts = false;
						}
					}
					else if (objectInfo[object_id]->get_comp_obj_type() == isDoor) { //open/close door
						int door_state = sprites_Array[column][line].object.component->get_door_state();
						if (!door_state) { //door is closed
							if (player.looks_at_left) {  //open to left
								if (!sprites_Array[column - 1][line].object.object_type && !sprites_Array[column - 1][line + 1].object.object_type &&
									!sprites_Array[column - 1][line + 2].object.object_type) {  //if enough space, then can open the door
									int width = column + objectInfo[object_id]->get_sizeX(), height = line + objectInfo[object_id]->get_sizeY();
									for (int i = column; i < width; i++)
										for (int j = line; j < height; j++) {
											sprites_Array[i][j].object = GameObject(None, 0);
										}
									column--;
									width = column + objectInfo[object_id + 1]->get_sizeX(), height = line + objectInfo[object_id + 1]->get_sizeY();
									sprites_Array[column][line].object = GameObject(isComplexObject, object_id + 1, new DoorComponent(1));
									for (int i = column; i < width; i++)
										for (int j = line; j < height; j++) {
											if (!sprites_Array[i][j].object.object_type)
												sprites_Array[i][j].object = GameObject(isCompObjPart, 0, new ComplexObjectPartComponent(column, line));
										}
								}
							}
							else {  //open to right
								if (!sprites_Array[column + 1][line].object.object_type && !sprites_Array[column + 1][line + 1].object.object_type &&
									!sprites_Array[column + 1][line + 2].object.object_type) {  //if enough space, then can open the door
									int width = column + objectInfo[object_id]->get_sizeX(), height = line + objectInfo[object_id]->get_sizeY();
									for (int i = column; i < width; i++)
										for (int j = line; j < height; j++) {
											sprites_Array[i][j].object = GameObject(None, 0);;
										}
									width = column + objectInfo[object_id + 2]->get_sizeX(), height = line + objectInfo[object_id + 2]->get_sizeY();
									sprites_Array[column][line].object = GameObject(isComplexObject, object_id + 2, new DoorComponent(2));
									for (int i = column; i < width; i++)
										for (int j = line; j < height; j++) {
											if (!sprites_Array[i][j].object.object_type)
												sprites_Array[i][j].object = GameObject(isCompObjPart, 0, new ComplexObjectPartComponent(column, line));
										}
								}
							}
							//check if enough space to open the door
							
						}
						else if (door_state == 1) {  //close to right
							int width = column + objectInfo[object_id]->get_sizeX(), height = line + objectInfo[object_id]->get_sizeY();
							for (int i = column; i < width; i++)
								for (int j = line; j < height; j++) {
									sprites_Array[i][j].object = GameObject(None, 0);
								}
							column++;
							width = column + objectInfo[object_id - 1]->get_sizeX(), height = line + objectInfo[object_id - 1]->get_sizeY();
							sprites_Array[column][line].object = GameObject(isComplexObject, object_id - 1, new DoorComponent(0));
							for (int i = column; i < width; i++)
								for (int j = line; j < height; j++) {
									if (!sprites_Array[i][j].object.object_type)
										sprites_Array[i][j].object = GameObject(isCompObjPart, 0, new ComplexObjectPartComponent(column, line));
								}
						}
						else {  //close to left
							int width = column + objectInfo[object_id]->get_sizeX(), height = line + objectInfo[object_id]->get_sizeY();
							for (int i = column; i < width; i++)
								for (int j = line; j < height; j++) {
									sprites_Array[i][j].object = GameObject(None, 0);
								}
							width = column + objectInfo[object_id - 2]->get_sizeX(), height = line + objectInfo[object_id - 2]->get_sizeY();
							sprites_Array[column][line].object = GameObject(isComplexObject, object_id - 2, new DoorComponent(0));
							for (int i = column; i < width; i++)
								for (int j = line; j < height; j++) {
									if (!sprites_Array[i][j].object.object_type)
										sprites_Array[i][j].object = GameObject(isCompObjPart, 0, new ComplexObjectPartComponent(column, line));
								}
						}
					}
				}
			}
		}

		//fill the buffer for drawing objects within the visible area
		index_size = 0;
		buffer_size = 0;
		buffer_ptr = sprite_vertices_buffer;
		X = leftX * BLOCK_VISIBLE_SIZE, Y;

		//firstly update sprite buffer for visible walls on the camera
		for (int i = leftX; i < rightX; i++) {
			Y = bottomY * BLOCK_VISIBLE_SIZE;
			for (int j = bottomY; j < topY; j++) {
				//render wall if it exists there and is visible
				if (sprites_Array[i][j].wall_id && sprites_Array[i][j].wall_is_visible) {
					object_id = sprites_Array[i][j].wall_id;
					tex_coords_ptr = objectInfo[object_id]->texture_coords;
					buffer_ptr->vertices = { X - BLOCK_VISIBLE_SIZE / 2, Y - BLOCK_VISIBLE_SIZE / 2 };
					buffer_ptr->tex_vertices = tex_coords_ptr[0];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X - BLOCK_VISIBLE_SIZE / 2, Y + BLOCK_VISIBLE_SIZE * 1.5 };
					buffer_ptr->tex_vertices = tex_coords_ptr[1];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE * 1.5, Y + BLOCK_VISIBLE_SIZE * 1.5 };
					buffer_ptr->tex_vertices = tex_coords_ptr[2];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE * 1.5, Y - BLOCK_VISIBLE_SIZE / 2 };
					buffer_ptr->tex_vertices = tex_coords_ptr[3];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_size += 4;
					index_size += 6;
				}
				Y += BLOCK_VISIBLE_SIZE;
			}
			X += BLOCK_VISIBLE_SIZE;
		}
		//secondly update buffer for all visible objects on the camera
		X = leftX * BLOCK_VISIBLE_SIZE;
		for (int i = leftX; i < rightX; i++) {
			Y = bottomY * BLOCK_VISIBLE_SIZE;
			for (int j = bottomY; j < topY; j++) {
				if (!sprites_Array[i][j].object.object_type || sprites_Array[i][j].object.object_type == isCompObjPart) { //no object in this slot
					if (sprites_Array[i][j].wall_id) { //skip if there is a wall
						Y += BLOCK_VISIBLE_SIZE;
						continue;
					}
					tex_coords_ptr = objectInfo[0]->texture_coords;
					buffer_ptr->vertices = { X, Y };
					buffer_ptr->tex_vertices = tex_coords_ptr[0];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X, Y + BLOCK_VISIBLE_SIZE };
					buffer_ptr->tex_vertices = tex_coords_ptr[1];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE, Y + BLOCK_VISIBLE_SIZE };
					buffer_ptr->tex_vertices = tex_coords_ptr[2];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE, Y };
					buffer_ptr->tex_vertices = tex_coords_ptr[3];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;
					buffer_size += 4;
					index_size += 6;
					Y += BLOCK_VISIBLE_SIZE;
					continue;
				}
				if (sprites_Array[i][j].object.object_type == isBlock) {  //simple object
					object_id = sprites_Array[i][j].object.object_id;
					if (objectInfo[object_id]->get_block_type() == isTreeTop) {
						tex_coords_ptr = objectInfo[object_id]->texture_coords;
						buffer_ptr->vertices = { X - BLOCK_VISIBLE_SIZE, Y };
						buffer_ptr->tex_vertices = tex_coords_ptr[0];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;

						buffer_ptr->vertices = { X - BLOCK_VISIBLE_SIZE, Y + BLOCK_VISIBLE_SIZE * 3 };
						buffer_ptr->tex_vertices = tex_coords_ptr[1];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;

						buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE * 2, Y + BLOCK_VISIBLE_SIZE * 3 };
						buffer_ptr->tex_vertices = tex_coords_ptr[2];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;

						buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE * 2, Y };
						buffer_ptr->tex_vertices = tex_coords_ptr[3];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;
					}
					else {
						tex_coords_ptr = objectInfo[object_id]->texture_coords;
						buffer_ptr->vertices = { X, Y };
						buffer_ptr->tex_vertices = tex_coords_ptr[0];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;

						buffer_ptr->vertices = { X, Y + BLOCK_VISIBLE_SIZE };
						buffer_ptr->tex_vertices = tex_coords_ptr[1];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;

						buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE, Y + BLOCK_VISIBLE_SIZE };
						buffer_ptr->tex_vertices = tex_coords_ptr[2];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;

						buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE, Y };
						buffer_ptr->tex_vertices = tex_coords_ptr[3];
						buffer_ptr->tex_id = 0.f;
						buffer_ptr++;
					}
				}
				else {  //or complex object
					object_id = sprites_Array[i][j].object.object_id;
					tex_coords_ptr = objectInfo[object_id]->texture_coords;
					buffer_ptr->vertices = { X, Y };
					buffer_ptr->tex_vertices = tex_coords_ptr[0];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X, Y + BLOCK_VISIBLE_SIZE * objectInfo[object_id]->get_sizeY()};
					buffer_ptr->tex_vertices = tex_coords_ptr[1];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE * objectInfo[object_id]->get_sizeX(), Y + BLOCK_VISIBLE_SIZE * objectInfo[object_id]->get_sizeY()};
					buffer_ptr->tex_vertices = tex_coords_ptr[2];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;

					buffer_ptr->vertices = { X + BLOCK_VISIBLE_SIZE * objectInfo[object_id]->get_sizeX(), Y };
					buffer_ptr->tex_vertices = tex_coords_ptr[3];
					buffer_ptr->tex_id = 0.f;
					buffer_ptr++;
				}

				buffer_size += 4;
				index_size += 6;
				Y += BLOCK_VISIBLE_SIZE;
			}
			X += BLOCK_VISIBLE_SIZE;
		}
		int dropped_items_size = dropped_items.size();
		buffer_size += 4 * dropped_items_size;
		index_size += 6 * dropped_items_size;
		for (int i = 0; i < dropped_items_size; i++) {
			float w = dropped_items[i].hitbox.size.x;
			float h = dropped_items[i].hitbox.size.y;
			object_id = dropped_items[i].id;
			X = dropped_items[i].hitbox.center.x - w * 0.5f;
			Y = dropped_items[i].hitbox.center.y - h * 0.5f;

			tex_coords_ptr = objectInfo[object_id]->texture_coords;
			buffer_ptr->vertices = { X, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[0];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;

			buffer_ptr->vertices = { X, Y + h };
			buffer_ptr->tex_vertices = tex_coords_ptr[1];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;

			buffer_ptr->vertices = { X + w, Y + h };
			buffer_ptr->tex_vertices = tex_coords_ptr[2];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;

			buffer_ptr->vertices = { X + w, Y };
			buffer_ptr->tex_vertices = tex_coords_ptr[3];
			buffer_ptr->tex_id = 0.f;
			buffer_ptr++;
		}
		sprite_vbo_p->bind_VBO();
		glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size * sizeof(Vertex2f), sprite_vertices_buffer);

		return 1;
	}
	}
}

void Game::render() {
	switch (game_render_state) {
	case inMainMenu: {
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		ui_elements_vbo->bind_VBO();
		glBufferSubData(GL_ARRAY_BUFFER, 0, buttons_amount * 4 * sizeof(ColorVertex2f), buttons_buffer);

		color_ui_SP->activate_shader();
		ui_elements_vao->bind_VAO();
		glDrawElements(GL_TRIANGLES, buttons_amount * 6, GL_UNSIGNED_INT, 0);
		
		text_manager->add_centered_text_to_buffer("Main menu", ScreenHeight * 0.1, glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.9), glm::vec4(0.f, 1.f, 0.f, 1.f));
		text_manager->add_centered_text_to_buffer("version 1.0.0", ScreenHeight * 0.04, glm::vec2(ScreenWidth * 0.07, ScreenHeight * 0.03), glm::vec4(0.f, 1.f, 0.f, 1.f));
		for (int i = 0; i < 3; i++) {
			text_manager->add_centered_text_to_buffer(main_buttons[i].text.c_str(), ScreenHeight * 0.075, main_buttons[i].center_pos, glm::vec4(0.f, 1.f, 0.f, 1.f));
		}
		text_manager->render_text();

		break;
	}
	case inWorldExplorer: {
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		ui_elements_vbo->bind_VBO();
		glBufferSubData(GL_ARRAY_BUFFER, 0, buttons_amount * 4 * sizeof(ColorVertex2f), buttons_buffer);
		
		color_ui_SP->activate_shader();
		ui_elements_vao->bind_VAO();
		glDrawElements(GL_TRIANGLES, buttons_amount * 6, GL_UNSIGNED_INT, 0);

		for (int i = 3; i < 5; i++) {
			text_manager->add_centered_text_to_buffer(main_buttons[i].text.c_str(), ScreenHeight * 0.075, main_buttons[i].center_pos, glm::vec4(0.f, 1.f, 0.f, 1.f));
		}
		int size = world_buttons.size();
		for (int i = 0; i < size; i++) {
			text_manager->add_centered_text_to_buffer(world_buttons[i].text.c_str(), ScreenHeight * 0.05, world_buttons[i].center_pos, glm::vec4(0.f, 1.f, 0.f, 1.f));
		}
		text_manager->render_text();
		
		break;
	}
	case inWorldCreator: {
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		ui_elements_vbo->bind_VBO();
		glBufferSubData(GL_ARRAY_BUFFER, 0, buttons_amount * 4 * sizeof(ColorVertex2f), buttons_buffer);

		color_ui_SP->activate_shader();
		ui_elements_vao->bind_VAO();
		glDrawElements(GL_TRIANGLES, buttons_amount * 6, GL_UNSIGNED_INT, 0);

		text_manager->add_centered_text_to_buffer("World name:", ScreenHeight * 0.075, glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.6), glm::vec4(0.f, 1.f, 0.f, 1.f));
		text_manager->add_centered_text_to_buffer(main_buttons[5].text.c_str(), ScreenHeight * 0.075, main_buttons[5].center_pos, glm::vec4(0.f, 1.f, 0.f, 1.f));
		text_manager->add_centered_text_to_buffer(text_field.text.c_str(), ScreenHeight * 0.065, text_field.center_pos, glm::vec4(0.f, 0.f, 0.f, 1.f));
		text_manager->render_text();

		break;
	}
	case WorldIsCreating: {
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		text_manager->add_centered_text_to_buffer("Generating the world...", ScreenHeight * 0.1, glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.5), glm::vec4(0.f, 0.2f, 1.f, 1.f));
		text_manager->render_text();
		break;
	}
	case WorldIsLoading: {
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		text_manager->add_centered_text_to_buffer("Loading the world...", ScreenHeight * 0.1, glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.5), glm::vec4(0.f, 0.2f, 1.f, 1.f));
		text_manager->render_text();
		break;
	}
	case WorldIsSaving: {
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		text_manager->add_centered_text_to_buffer("Saving the world...", ScreenHeight * 0.1, glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.5), glm::vec4(0.f, 0.2f, 1.f, 1.f));
		text_manager->render_text();
		break;
	}
	case inGame: {
		glClearColor(skyColor[0], skyColor[1], skyColor[2], 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTextureUnit(0, a_textures.get_texture_index(0));

		//0)ambient objects (sun, clouds, etc.)
		ambient_sprite_SP_ptr->activate_shader();
		instance_vao_p->bind_VAO();
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 12);

		//1) batch render all objects, items and walls
		sprite_SP_ptr->activate_shader();
		sprite_SP_ptr->set_uniform_float("day_ratio", day_ratio);
		sprite_vao_p->bind_VAO();
		glDrawElements(GL_TRIANGLES, index_size, GL_UNSIGNED_INT, 0);

		//2) instance render all entities(player, enemies, npc, projectiles, etc.)
			//[[hitbox
		entity_SP->activate_shader();
		entity_SP->set_Uniform_Mat4("modelMatrix", viewMatrix);
		entity_vao->bind_VAO();
		glDrawElements(GL_LINE_LOOP, 1 * 6, GL_UNSIGNED_INT, 0);
			//]]
		entity_sprite_SP_ptr->activate_shader();
		entity_sprite_SP_ptr->set_uniform_float("day_ratio", day_ratio);
		//should make common VAO for instanced rendering
		instance_vao_p->bind_VAO();
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, entities_count);

		//2.5 render damage text
		int size = damage_text.size();
		if (size) {
			text_manager->set_view_matrix(viewMatrix);
			for (DamageText& text : damage_text) {
				text_manager->add_centered_text_to_buffer(text.text.c_str(), text.text_height, text.start_pos, text.color);
			}
			text_manager->render_text();
			text_manager->set_view_matrix(glm::mat4(1.f));
		}

		//3) batch render all slots in inventory
		//4) batch render all sprites in inventory
		//5) batch render all text in inventory
		glBindTextureUnit(0, a_textures.get_texture_index(0));
		color_ui_SP->activate_shader();
		glm::mat4 model = glm::mat4(1.f);
		inventory_vao->bind_VAO();
		if (inventoryIsOpen) {  //draw inventory slots, inventory objects, text that contains amount of each object
			color_ui_SP->set_Uniform_Mat4("modelMatrix", model);
			glDrawElements(GL_TRIANGLES, (66 + additional_slots) * 6, GL_UNSIGNED_INT, 0);

			UI_sprite_SP_ptr->activate_shader();
			UI_sprite_SP_ptr->set_Uniform_Mat4("viewMatrix", model);
			UI_sprite_SP_ptr->set_uniform_float("day_ratio", 1.0);
			inventory_objects_vao->bind_VAO();
			glDrawElements(GL_TRIANGLES, inventory_index_size, GL_UNSIGNED_INT, 0);

			text_manager->update_text_array(inventory_text_info, inventory_text_size, ScreenHeight * 0.03, glm::vec4(1.0, 1.0, 1.0, 1.f));
			text_manager->render_text();
		}
		else {
			model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.f));
			//float dy = ScreenHeight * (1 / 0.8) - ScreenHeight;  = 0.25 * ScreenHeight
			model = glm::translate(model, glm::vec3(0.f, ScreenHeight * 0.25f, 0.f));
			color_ui_SP->set_Uniform_Mat4("modelMatrix", model);
			glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

			UI_sprite_SP_ptr->activate_shader();
			UI_sprite_SP_ptr->set_Uniform_Mat4("viewMatrix", model);
			UI_sprite_SP_ptr->set_uniform_float("day_ratio", 1.0);
			inventory_objects_vao->bind_VAO();
			glDrawElements(GL_TRIANGLES, inventory_index_size, GL_UNSIGNED_INT, 0);

			text_manager->set_view_matrix(model);
			text_manager->update_text_array(inventory_text_info, inventory_text_size, ScreenHeight * 0.03, glm::vec4(1.0, 1.0, 1.0, 1.f));
			text_manager->render_text();
		}

		//6) draw object info box for the active slot or for the one that mouse cursor is pointing on
		//7) render info box text
		for (int i = 0; i < 2; i++) {  
			if (object_info_box[i].show_box) {
				text_manager->update_object_info_box_text(object_info_box[i].box_string.c_str(), ScreenHeight * 0.03, object_info_box[i].starting_pos, object_info_box[i].text_color, object_info_box[i].box_size);
				object_info_box[i].box_vertices[0] = object_info_box[i].box_vertices[6] = object_info_box[i].starting_pos.x;
				object_info_box[i].box_vertices[1] = object_info_box[i].box_vertices[19] = object_info_box[i].starting_pos.y - object_info_box[i].box_size[1];
				object_info_box[i].box_vertices[12] = object_info_box[i].box_vertices[18] = object_info_box[i].starting_pos.x + object_info_box[i].box_size[0];
				object_info_box[i].box_vertices[7] = object_info_box[i].box_vertices[13] = object_info_box[i].starting_pos.y;
				inventory_vbo->bind_VBO();
				glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * 4, object_info_box[i].box_vertices);
				color_ui_SP->activate_shader();
				inventory_vao->bind_VAO();
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				text_manager->render_text();
				//break;
			}
		}
		//reset matrices
		text_manager->set_view_matrix(glm::mat4(1.f));
		color_ui_SP->activate_shader();
		color_ui_SP->set_Uniform_Mat4("modelMatrix", glm::mat4(1.f));

		//8)render the rest, but should rework this to minimize render calls and make a better system, for example render all things for UI in one call and all text for UI in one call
		std::stringstream ss;
		ss << "HP:" << player.stats.currentHP;
		text_manager->add_text_to_buffer(ss.str().c_str(), ScreenHeight * 0.05, glm::vec2(ScreenWidth * 0.87, ScreenHeight * 0.95), glm::vec4(1.f, 0.f, 0.f, 1.f));

		ss.str("");
		ss << "MANA:" << player.stats.currentMANA;
		text_manager->add_text_to_buffer(ss.str().c_str(), ScreenHeight * 0.05, glm::vec2(ScreenWidth * 0.87, ScreenHeight * 0.9), glm::vec4(0.f, 0.f, 1.f, 1.f));

		ss.str("");
		ss << "FPS:" << current_FPS;
		//ss << "DEF: " << player.stats.DEF;
		text_manager->add_text_to_buffer(ss.str().c_str(), ScreenHeight * 0.04, glm::vec2(ScreenWidth * 0.75, ScreenHeight * 0.8), getRainbowColor(rainbow_color_time));

		ss.str("");
		ss << "C:" << int((ScreenWidth / 2 - camera.dX) / BLOCK_VISIBLE_SIZE) << "," << int((ScreenHeight / 2 - camera.dY) / BLOCK_VISIBLE_SIZE);
		text_manager->add_text_to_buffer(ss.str().c_str(), ScreenHeight * 0.04, glm::vec2(ScreenWidth * 0.75, ScreenHeight * 0.75), getRainbowColor(rainbow_color_time));

		ss.str("");
		ss << "E:" << entities.size() << " P:" << projectiles.size() << "" << " D:" << dropped_items.size();
		text_manager->add_text_to_buffer(ss.str().c_str(), ScreenHeight * 0.04, glm::vec2(ScreenWidth * 0.75, ScreenHeight * 0.7), getRainbowColor(rainbow_color_time));

		ss.str("");
		ss << "Time: " << int(cycle_time / 60) << ":" << int(cycle_time) % 60;
		text_manager->add_text_to_buffer(ss.str().c_str(), ScreenHeight * 0.04, glm::vec2(ScreenWidth * 0.75, ScreenHeight * 0.65), getRainbowColor(rainbow_color_time));

		ss.str("");
		ss << "w.a. : " << glm::degrees(active_weapon.angle);
		text_manager->add_text_to_buffer(ss.str().c_str(), ScreenHeight * 0.04, glm::vec2(ScreenWidth * 0.75, ScreenHeight * 0.6), getRainbowColor(rainbow_color_time));

		if (entity_info_text.isActive) {
			text_manager->add_text_to_buffer(entity_info_text.info.c_str(), ScreenHeight * 0.02, entity_info_text.start_pos, glm::vec4(1.f, 1.f, 1.f, 1.f));
		}
		text_manager->render_text();

		break;
	}
	}
}

void Game::toggle_Fullscreen(GLFWwindow* window) {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int count;
	const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

	if (glfwGetWindowMonitor(window) == nullptr) {
		// Switch to full screen
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else {
		// Switch back to windowed mode with Full HD resolution
		glfwSetWindowMonitor(window, nullptr, 100, 100, 1920, 1080, GLFW_DONT_CARE);
	}
	//update projection matrix for all shader programs
	projectionMatrix = glm::ortho(0.f, ScreenWidth, 0.f, ScreenHeight);

	sprite_SP_ptr->activate_shader();
	sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);

	UI_sprite_SP_ptr->activate_shader();
	UI_sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);

	entity_sprite_SP_ptr->activate_shader();
	entity_sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);

	ambient_sprite_SP_ptr->activate_shader();
	ambient_sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);

	color_ui_SP->activate_shader();
	color_ui_SP->set_Uniform_Mat4("projectionMatrix", projectionMatrix);

	entity_SP->activate_shader();
	entity_SP->set_Uniform_Mat4("projectionMatrix", projectionMatrix);

	text_manager->set_projection_mat(projectionMatrix);

	//update inventory vertices buffer based on the new screen resolution
	init_inventory_buffer();
	//48 is the standard amount of visible blocks on X axis with standard zoom (100%) - (for Full HD visible size is 40.f)
	float ratio = std::round(ScreenWidth / 48) / BLOCK_VISIBLE_SIZE;
	//change visible block size based on the new resolution
	BLOCK_VISIBLE_SIZE = std::round(ScreenWidth / 48);
	//change player variables based on new visible block size
	player.speed = BLOCK_VISIBLE_SIZE * 5; //5 blocks per second
	player.jump_speed = BLOCK_VISIBLE_SIZE * 15; //able to jump on 6 blocks height
	//change player coords
	player.hitbox.center.x *= ratio; //take the value which is new divided by the standard
	player.hitbox.center.y *= ratio;
	player.hitbox.size *= ratio;
	player.sprite_size *= ratio;
	camera.dX = -player.hitbox.center.x + ScreenWidth / 2;
	camera.dY = -player.hitbox.center.y + ScreenHeight / 2;
	camera.rightBorderDx = -world_width * BLOCK_VISIBLE_SIZE + ScreenWidth + camera.scalingDx;
	camera.topBorderDy = -world_height * BLOCK_VISIBLE_SIZE + ScreenHeight + camera.scalingDy;

	//change ambient objects coords
	ambientController.clouds[0].position.x *= ratio;
	ambientController.clouds[1].position.x *= ratio;
	ambientController.far_clouds[0].position.x *= ratio;
	ambientController.far_clouds[1].position.x *= ratio;
	ambientController.far_clouds[2].position.x *= ratio;
	ambientController.sky_stars[0].position *= ratio;
	ambientController.sky_stars[1].position *= ratio;
	ambientController.sky_stars[2].position *= ratio;
	ambientController.sky_stars[3].position *= ratio;
	ambientController.sky_stars[4].position *= ratio;
	ambientController.sky_stars[5].position *= ratio;

	//change buttons
	init_world_buttons();
	init_main_buttons();

	//shader uniforms
	sprite_SP_ptr->activate_shader();
	sprite_SP_ptr->set_uniform_float("blockSize", BLOCK_VISIBLE_SIZE);
	entity_sprite_SP_ptr->activate_shader();
	entity_sprite_SP_ptr->set_uniform_float("blockSize", BLOCK_VISIBLE_SIZE);
}

void Game::init() {
	//load save files if they exist
	load_available_saves();
	init_world_buttons();
	init_main_buttons();

	//load textures
	a_textures.load_2D_texture("Resources/textures/2D_Textures.png", true); //id is 0

	//fill blocks (simple objects) info
	objectInfo[0] = new BlockInfo("air", isSolidBlock, 0, 0, false, false);
	objectInfo[1] = new BlockInfo("Stone", isSolidBlock, 1, 1, true, false);
	objectInfo[2] = new BlockInfo("Dirt", isSolidBlock, 1, 2, true, false);
	objectInfo[3] = new BlockInfo("Grass Dirt", isSolidBlock, 1, 3, true, false);
	objectInfo[4] = new BlockInfo("Sand", isSolidBlock, 1, 4, true, false);
	objectInfo[5] = new BlockInfo("Oak", isWood, 1, 5, false, false);
	objectInfo[6] = new BlockInfo("Oak Planks", isSolidBlock, 1, 6, true, false);
	objectInfo[7] = new BlockInfo("Crimson Stone", isSolidBlock, 1, 7, true, false);
	objectInfo[8] = new BlockInfo("Corrupted Stone", isSolidBlock, 1, 8, true, false);
	objectInfo[9] = new BlockInfo("Copper Ore", isSolidBlock, 1, 9, true, false);
	objectInfo[10] = new BlockInfo("Iron Ore", isSolidBlock, 1, 10, true, false);
	objectInfo[11] = new BlockInfo("Gold Ore", isSolidBlock, 1, 11, true, false);
	objectInfo[12] = new BlockInfo("Water", isLiquid, 1, 12, false, false);
	objectInfo[13] = new BlockInfo("Snow", isSolidBlock, 1, 13, true, false);
	objectInfo[14] = new BlockInfo("Ice", isSolidBlock, 1, 14, true, false);
	objectInfo[15] = new BlockInfo("Snow Grass Dirt", isSolidBlock, 1, 15, true, false);
	objectInfo[16] = new BlockInfo("Crimson Grass Dirt", isSolidBlock, 1, 16, true, false);
	objectInfo[17] = new BlockInfo("Corrupted Grass Dirt", isSolidBlock, 1, 17, true, false);
	objectInfo[18] = new BlockInfo("Grass", isGrass, 1, 18, false, false);
	objectInfo[19] = new BlockInfo("Corrupted Grass", isGrass, 1, 19, false, false);
	objectInfo[20] = new BlockInfo("Crimson Grass", isGrass, 1, 20, false, false);
	objectInfo[21] = new BlockInfo("Day Flower", isFlower, 1, 21, false, false);
	objectInfo[22] = new BlockInfo("Torch", isTorch, 0.f, true, glm::vec3(1.f, 1.f, 0.f), 5.f, 1, 22, false, false);
	objectInfo[23] = new BlockInfo("Torch(l)", isTorch, 0.f, true, glm::vec3(1.f, 1.f, 0.f), 5.f, 1, 22, false, false);
	objectInfo[24] = new BlockInfo("Torch(r)", isTorch, 0.f, true, glm::vec3(1.f, 1.f, 0.f), 5.f, 1, 22, false, false);
	objectInfo[25] = new BlockInfo("Ice Torch", isTorch, 0.f, true, glm::vec3(0.f, 0.9f, 1.f), 5.f, 1, 25, false, false);
	objectInfo[26] = new BlockInfo("Ice Torch(l)", isTorch, 0.f, true, glm::vec3(0.f, 0.9f, 1.f), 5.f, 1, 25, false, false);
	objectInfo[27] = new BlockInfo("Ice Torch(r)", isTorch, 0.f, true, glm::vec3(0.f, 0.9f, 1.f), 5.f, 1, 25, false, false);
	objectInfo[28] = new BlockInfo("Oak Platform", isPlatform, 1.f, 28, false, true);
	objectInfo[29] = new BlockInfo("Oak Platform(l)", isPlatform, 1.f, 28, false, true);
	objectInfo[30] = new BlockInfo("Oak Platform(r)", isPlatform, 1.f, 28, false, true);
	objectInfo[31] = new BlockInfo("Oak Top", isTreeTop, 1, 5, false, false);
	//fill walls info
	objectInfo[32] = new ObjectInfo("Stone Wall");
	objectInfo[33] = new ObjectInfo("Dirt Wall");
	objectInfo[34] = new ObjectInfo("Oak Wall");
	objectInfo[35] = new ObjectInfo("Crimson Wall");
	objectInfo[36] = new ObjectInfo("Corrupted Wall");
	objectInfo[37] = new ObjectInfo("Snow Wall");
	//fill complex objects info
	objectInfo[38] = new ComplexObjectInfo("Workbench", isWorkbench, 1, 38, 2, 1, false, true);
	objectInfo[39] = new ComplexObjectInfo("Furnace", isFurnace, 0.f, true, glm::vec3(1.f, 1.f, 0.f), 3.f, 1, 39, 3, 2, false, false);
	objectInfo[40] = new ComplexObjectInfo("Anvil", isAnvil, 1, 40, 2, 1, false, false);
	objectInfo[41] = new ComplexObjectInfo("Oak Table", isTable, 1, 41, 3, 2, false, false);
	objectInfo[42] = new ComplexObjectInfo("Oak Chair", isChair, 1, 42, 1, 2, false, false); //points to the left side
	objectInfo[43] = new ComplexObjectInfo("Oak Chair", isChair, 1, 42, 1, 2, false, false); //points to the right side
	objectInfo[44] = new ComplexObjectInfo("Oak Door", isDoor, 1, 44, 1, 3, true, false); //closed
	objectInfo[45] = new ComplexObjectInfo("Oak Door", isDoor, 1, 44, 2, 3, false, false); //opened to the left side
	objectInfo[46] = new ComplexObjectInfo("Oak Door", isDoor, 1, 44, 2, 3, false, false); //opened to the right side
	objectInfo[47] = new ComplexObjectInfo("Chest", isChest, 1, 47, 2, 2, false, false);
	//fill weapons info
	objectInfo[48] = new InstrumentalWeaponInfo("Copper Pickaxe", isPickaxe, 1.f, 1, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[49] = new InstrumentalWeaponInfo("Copper Axe", isAxe, 1.f, 1, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[50] = new WeaponInfo("Copper Dagger", isDagger, 3, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[51] = new WeaponInfo("Copper Sword", isSword, 4, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[52] = new InstrumentalWeaponInfo("Copper Hammer", isHammer, 1.f, 2, 0.5f, 1.f, 0.f, -1, 2.f, 2.f, 5.f, false, 3);
	objectInfo[53] = new WeaponInfo("Copper Bow", isBow, 3, 0.f, 15.f, 1.f, -1, 1.5f, 1.5f, 5.f, false, 0);

	objectInfo[54] = new InstrumentalWeaponInfo("Iron Pickaxe", isPickaxe, 1.f, 1, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[55] = new InstrumentalWeaponInfo("Iron Axe", isAxe, 1.f, 1, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[56] = new WeaponInfo("Iron Dagger", isDagger, 3, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[57] = new WeaponInfo("Iron Sword", isSword, 4, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[58] = new InstrumentalWeaponInfo("Iron Hammer", isHammer, 1.f, 2, 0.5f, 1.f, 0.f, -1, 2.f, 2.f, 5.f, false, 3);
	objectInfo[59] = new WeaponInfo("Iron Bow", isBow, 3, 0.f, 15.f, 1.f, -1, 1.5f, 1.5f, 5.f, false, 0);

	objectInfo[60] = new InstrumentalWeaponInfo("Golden Pickaxe", isPickaxe, 1.f, 1, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[61] = new InstrumentalWeaponInfo("Golden Axe", isAxe, 1.f, 1, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[62] = new WeaponInfo("Golden Dagger", isDagger, 3, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[63] = new WeaponInfo("Golden Sword", isSword, 4, 0.5f, 1.f, 0.f, -1, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[64] = new InstrumentalWeaponInfo("Golden Hammer", isHammer, 1.f, 2, 0.5f, 1.f, 0.f, -1, 2.f, 2.f, 5.f, false, 3);
	objectInfo[65] = new WeaponInfo("Golden Bow", isBow, 3, 0.f, 30.f, 0.25f, -1, 1.5f, 1.5f, 5.f, false, 0);
	//fill coin info
	objectInfo[66] = new ObjectInfo(isCoin, "Copper Coin");
	objectInfo[67] = new ObjectInfo(isCoin, "Silver Coin");
	objectInfo[68] = new ObjectInfo(isCoin, "Gold Coin");
	objectInfo[69] = new ObjectInfo(isCoin, "Platinum Coin");
	//fill materials info
	objectInfo[70] = new ObjectInfo(isMaterial, "Copper Ingot");
	objectInfo[71] = new ObjectInfo(isMaterial, "Iron Ingot");
	objectInfo[72] = new ObjectInfo(isMaterial, "Gold Ingot");
	objectInfo[73] = new ObjectInfo(isMaterial, "Gel");
	//fill ammo info
	objectInfo[74] = new AmmoInfo("Wooden Arrow", isArrow, 5, 1.5f, 1.5f, 0);
	objectInfo[75] = new AmmoInfo("Flaming Arrow", isArrow, 0.f, true, glm::vec3(1.f, 1.f, 0.f), 3.f, 7, 1.5f, 1.5f, 1);
	objectInfo[75]->effectId = 5; objectInfo[75]->effectDuration = 5.f;
	objectInfo[76] = new AmmoInfo("Frostburn Arrow", isArrow, 0.f, true, glm::vec3(0.f, 0.9f, 1.f), 3.f, 9, 1.5f, 1.5f, 2);
	objectInfo[76]->effectId = 6; objectInfo[76]->effectDuration = 5.f;
	objectInfo[77] = new AmmoInfo("Jester's Arrow", isArrow, 0.f, true, glm::vec3(1.f, 0.9f, 0.9f), 3.f, 10, 1.5f, 1.5f, 3);
	objectInfo[78] = new AmmoInfo("Unholy Arrow", isArrow, 0.f, true, glm::vec3(0.306f, 0.086f, 0.702f), 3.f, 14, 1.5f, 1.5f, 4);
	objectInfo[79] = new AmmoInfo("Hellfire Arrow", isArrow, 0.f, true, glm::vec3(1.f, 0.76f, 0.03f), 3.f, 16, 1.5f, 1.5f, 5);

	objectInfo[80] = new ComplexObjectInfo("Life Crystal", isLyfeCrystal, 0.f, true, glm::vec3(0.8f, 0.2f, 0.2f), 3.f, 1, 80, 2, 2, false, false);
	objectInfo[80]->effectId = 0;
	objectInfo[81] = new BlockInfo("Glass", isGlass, 0, 81, true, false);
	objectInfo[82] = new BlockInfo("Mushroom", isFlower, 0, 82, false, false);
	objectInfo[83] = new BlockInfo("Bottle", isBottle, 0, 83, false, false);
	objectInfo[84] = new BlockInfo("Bottle of water", isBottle, 0, 84, false, false);
	objectInfo[85] = new ObjectInfo(isMaterial, "Lens");
	objectInfo[86] = new ObjectInfo(isMaterial, "Fallen Star");
	objectInfo[87] = new ObjectInfo(isConsumable, "Mana Crystal");
	objectInfo[87]->effectId = 1;
	objectInfo[88] = new ObjectInfo(isPotion, "Healing Potion");
	objectInfo[88]->effectId = 10;
	objectInfo[89] = new ObjectInfo(isPotion, "Speed Potion");
	objectInfo[89]->effectId = 3; objectInfo[89]->effectDuration = 120;
	objectInfo[90] = new ObjectInfo(isPotion, "Defense Potion");
	objectInfo[90]->effectId = 2; objectInfo[90]->effectDuration = 120;
	objectInfo[91] = new ObjectInfo(isPotion, "Regeneration Potion");
	objectInfo[91]->effectId = 4; objectInfo[91]->effectDuration = 120;
	objectInfo[92] = new ObjectInfo(isPotion, "Better Healing Potion");
	objectInfo[92]->effectId = 10;

	objectInfo[93] = new MagicalWeaponInfo("Magic wand", isMagical, 5, 5, 0.f, 5.f, 1.f, 0, 1.5f, 1.5f, 5.f, false, 11);
	objectInfo[94] = new WeaponInfo("Shuriken", isThrowable, 10, 0.f, 10.f, 0.5f, 13, 1.5f, 1.5f, 5.f, true, 3);
	objectInfo[95] = new WeaponInfo("Throwing knife", isThrowable, 5, 0.f, 10.f, 0.5f, 14, 1.5f, 1.5f, 5.f, false, 3);
	objectInfo[96] = new MagicalWeaponInfo("Game.cpp", isMagical, 0, 0, 0.f, 10.f, 0.5f, 11, 1.5f, 1.5f, 0.f, false, 14);
	objectInfo[97] = new AmmoInfo("Bullet", isBullet, 0.f, true, glm::vec3(0.5f, 0.f, 0.f), 1.f, 7, 1.f, 1.f, 12);
	objectInfo[98] = new WeaponInfo("Pistol", isGun, 20, 0.f, 20.f, 0.5f, 0, 2.f, 1.f, 10.f, false, 12);
	objectInfo[99] = new WeaponInfo("Shotgun", isShotgun, 15, 0.f, 20.f, 1.f, 0, 2.f, 1.f, 10.f, false, 13);
	objectInfo[100] = new WeaponInfo("Trident", isSword, 15, 0.25f, 1.5f, 0.f, 0, 2.5f, 2.5f, 10.f, false, 3);

	float epsilon = 0.125f / 512;
	float block_s = 1.f / 32.f;
	//air
	objectInfo[0]->texture_coords[0] = glm::vec2(1.f - epsilon, 0.f + epsilon);
	objectInfo[0]->texture_coords[1] = glm::vec2(1.f - epsilon, 0.f + epsilon);
	objectInfo[0]->texture_coords[2] = glm::vec2(1.f - epsilon, 0.f + epsilon);
	objectInfo[0]->texture_coords[3] = glm::vec2(1.f - epsilon, 0.f + epsilon);
	for (int i = 0; i < 30; i++) { //blocks
		int yTex = i / 32;
		int xTex = i % 32;
		objectInfo[i + 1]->texture_coords[0] = glm::vec2(block_s * xTex + epsilon, 1.0 - block_s * yTex - block_s + epsilon);
		objectInfo[i + 1]->texture_coords[1] = glm::vec2(block_s * xTex + epsilon, 1.0 - block_s * yTex - epsilon);
		objectInfo[i + 1]->texture_coords[2] = glm::vec2(block_s * xTex + block_s - epsilon, 1.0 - block_s * yTex - epsilon);
		objectInfo[i + 1]->texture_coords[3] = glm::vec2(block_s * xTex + block_s - epsilon, 1.0 - block_s * yTex - block_s + epsilon);
	}
	//oak tree top
	objectInfo[31]->texture_coords[0] = glm::vec2(5 * block_s + epsilon, 23 * block_s + epsilon);
	objectInfo[31]->texture_coords[1] = glm::vec2(5 * block_s + epsilon, 26 * block_s - epsilon);
	objectInfo[31]->texture_coords[2] = glm::vec2(8 * block_s - epsilon, 26 * block_s - epsilon);
	objectInfo[31]->texture_coords[3] = glm::vec2(8 * block_s - epsilon, 23 * block_s + epsilon);
	for (int i = 16; i <= 21; i++) { //walls
		int yTex = i / 16;
		int xTex = i % 16;
		objectInfo[i + 16]->texture_coords[0] = glm::vec2(block_s * 2 * xTex + epsilon, 1.0 - block_s * 2 * yTex - block_s * 2 + epsilon);
		objectInfo[i + 16]->texture_coords[1] = glm::vec2(block_s * 2 * xTex + epsilon, 1.0 - block_s * 2 * yTex - epsilon);
		objectInfo[i + 16]->texture_coords[2] = glm::vec2(block_s * 2 * xTex + block_s * 2 - epsilon, 1.0 - block_s * 2 * yTex - epsilon);
		objectInfo[i + 16]->texture_coords[3] = glm::vec2(block_s * 2 * xTex + block_s * 2 - epsilon, 1.0 - block_s * 2 * yTex - block_s * 2 + epsilon);
	}
	//complex objects texture coords
	//workbench
	objectInfo[38]->texture_coords[0] = glm::vec2(epsilon, block_s * 26 + epsilon);
	objectInfo[38]->texture_coords[1] = glm::vec2(epsilon, block_s * 27 - epsilon);
	objectInfo[38]->texture_coords[2] = glm::vec2(block_s * 2 - epsilon, block_s * 27 - epsilon);
	objectInfo[38]->texture_coords[3] = glm::vec2(block_s * 2 - epsilon, block_s * 26 + epsilon);
	//furnace
	objectInfo[39]->texture_coords[0] = glm::vec2(block_s * 2 + epsilon, block_s * 26 + epsilon);
	objectInfo[39]->texture_coords[1] = glm::vec2(block_s * 2 + epsilon, block_s * 28 - epsilon);
	objectInfo[39]->texture_coords[2] = glm::vec2(block_s * 5 - epsilon, block_s * 28 - epsilon);
	objectInfo[39]->texture_coords[3] = glm::vec2(block_s * 5 - epsilon, block_s * 26 + epsilon);
	//anvil
	objectInfo[40]->texture_coords[0] = glm::vec2(block_s * 5 + epsilon, block_s * 26 + epsilon);
	objectInfo[40]->texture_coords[1] = glm::vec2(block_s * 5 + epsilon, block_s * 27 - epsilon);
	objectInfo[40]->texture_coords[2] = glm::vec2(block_s * 7 - epsilon, block_s * 27 - epsilon);
	objectInfo[40]->texture_coords[3] = glm::vec2(block_s * 7 - epsilon, block_s * 26 + epsilon);
	//table
	objectInfo[41]->texture_coords[0] = glm::vec2(block_s * 7 + epsilon, block_s * 26 + epsilon);
	objectInfo[41]->texture_coords[1] = glm::vec2(block_s * 7 + epsilon, block_s * 28 - epsilon);
	objectInfo[41]->texture_coords[2] = glm::vec2(block_s * 10 - epsilon, block_s * 28 - epsilon);
	objectInfo[41]->texture_coords[3] = glm::vec2(block_s * 10 - epsilon, block_s * 26 + epsilon);
	//chair(to left)
	objectInfo[42]->texture_coords[0] = glm::vec2(block_s * 10 + epsilon, block_s * 26 + epsilon);
	objectInfo[42]->texture_coords[1] = glm::vec2(block_s * 10 + epsilon, block_s * 28 - epsilon);
	objectInfo[42]->texture_coords[2] = glm::vec2(block_s * 11 - epsilon, block_s * 28 - epsilon);
	objectInfo[42]->texture_coords[3] = glm::vec2(block_s * 11 - epsilon, block_s * 26 + epsilon);
	//chair(to right)
	objectInfo[43]->texture_coords[0] = glm::vec2(block_s * 11 + epsilon, block_s * 26 + epsilon);
	objectInfo[43]->texture_coords[1] = glm::vec2(block_s * 11 + epsilon, block_s * 28 - epsilon);
	objectInfo[43]->texture_coords[2] = glm::vec2(block_s * 12 - epsilon, block_s * 28 - epsilon);
	objectInfo[43]->texture_coords[3] = glm::vec2(block_s * 12 - epsilon, block_s * 26 + epsilon);
	//door(closed)
	objectInfo[44]->texture_coords[0] = glm::vec2(block_s * 4 + epsilon, block_s * 23 + epsilon);
	objectInfo[44]->texture_coords[1] = glm::vec2(block_s * 4 + epsilon, block_s * 26 - epsilon);
	objectInfo[44]->texture_coords[2] = glm::vec2(block_s * 5 - epsilon, block_s * 26 - epsilon);
	objectInfo[44]->texture_coords[3] = glm::vec2(block_s * 5 - epsilon, block_s * 23 + epsilon);
	//door(to left)
	objectInfo[45]->texture_coords[0] = glm::vec2(epsilon, block_s * 23 + epsilon);
	objectInfo[45]->texture_coords[1] = glm::vec2(epsilon, block_s * 26 - epsilon);
	objectInfo[45]->texture_coords[2] = glm::vec2(block_s * 2 - epsilon, block_s * 26 - epsilon);
	objectInfo[45]->texture_coords[3] = glm::vec2(block_s * 2 - epsilon, block_s * 23 + epsilon);
	//door(to right)
	objectInfo[46]->texture_coords[0] = glm::vec2(block_s * 2 + epsilon, block_s * 23 + epsilon);
	objectInfo[46]->texture_coords[1] = glm::vec2(block_s * 2 + epsilon, block_s * 26 - epsilon);
	objectInfo[46]->texture_coords[2] = glm::vec2(block_s * 4 - epsilon, block_s * 26 - epsilon);
	objectInfo[46]->texture_coords[3] = glm::vec2(block_s * 4 - epsilon, block_s * 23 + epsilon);
	//chest
	objectInfo[47]->texture_coords[0] = glm::vec2(block_s * 12 + epsilon, block_s * 26 + epsilon);
	objectInfo[47]->texture_coords[1] = glm::vec2(block_s * 12 + epsilon, block_s * 28 - epsilon);
	objectInfo[47]->texture_coords[2] = glm::vec2(block_s * 14 - epsilon, block_s * 28 - epsilon);
	objectInfo[47]->texture_coords[3] = glm::vec2(block_s * 14 - epsilon, block_s * 26 + epsilon);
	//life crystal
	objectInfo[80]->texture_coords[0] = glm::vec2(block_s * 16 + epsilon, block_s * 26 + epsilon);
	objectInfo[80]->texture_coords[1] = glm::vec2(block_s * 16 + epsilon, block_s * 28 - epsilon);
	objectInfo[80]->texture_coords[2] = glm::vec2(block_s * 18 - epsilon, block_s * 28 - epsilon);
	objectInfo[80]->texture_coords[3] = glm::vec2(block_s * 18 - epsilon, block_s * 26 + epsilon);

	//weapons, coins and materials
	for (int i = 0; i < 26; i++) {
		int yTex = i / 32;
		int xTex = i % 32;
		objectInfo[i + 48]->texture_coords[0] = glm::vec2(block_s * xTex + epsilon, 1.0 - block_s * (9 + yTex) - block_s + epsilon);
		objectInfo[i + 48]->texture_coords[1] = glm::vec2(block_s * xTex + epsilon, 1.0 - block_s * (9 + yTex) - epsilon);
		objectInfo[i + 48]->texture_coords[2] = glm::vec2(block_s * xTex + block_s - epsilon, 1.0 - block_s * (9 + yTex) - epsilon);
		objectInfo[i + 48]->texture_coords[3] = glm::vec2(block_s * xTex + block_s - epsilon, 1.0 - block_s * (9 + yTex) - block_s + epsilon);
	}
	//ammo
	for (int i = 0; i < 6; i++) {
		objectInfo[i + 74]->texture_coords[0] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + epsilon, 1.0 - block_s * 9 - block_s + epsilon);
		objectInfo[i + 74]->texture_coords[1] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + epsilon, 1.0 - block_s * 9 - epsilon);
		objectInfo[i + 74]->texture_coords[2] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + block_s - epsilon, 1.0 - block_s * 9 - epsilon);
		objectInfo[i + 74]->texture_coords[3] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + block_s - epsilon, 1.0 - block_s * 9 - block_s + epsilon);
	}
	for (int i = 0; i < 14; i++) { //items after ammo
		objectInfo[i + 83]->texture_coords[0] = glm::vec2(block_s * i + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
		objectInfo[i + 83]->texture_coords[1] = glm::vec2(block_s * i + epsilon, 1.0 - block_s * 10 - epsilon);
		objectInfo[i + 83]->texture_coords[2] = glm::vec2(block_s * i + block_s - epsilon, 1.0 - block_s * 10 - epsilon);
		objectInfo[i + 83]->texture_coords[3] = glm::vec2(block_s * i + block_s - epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	}
	//bullet texture
	objectInfo[97]->texture_coords[0] = glm::vec2(block_s * 18 + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	objectInfo[97]->texture_coords[1] = glm::vec2(block_s * 18 + epsilon, 1.0 - block_s * 10 - epsilon);
	objectInfo[97]->texture_coords[2] = glm::vec2(block_s * 18 + block_s - epsilon, 1.0 - block_s * 10 - epsilon);
	objectInfo[97]->texture_coords[3] = glm::vec2(block_s * 18 + block_s - epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	//pistol texture
	objectInfo[98]->texture_coords[0] = glm::vec2(block_s * 16 + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	objectInfo[98]->texture_coords[1] = glm::vec2(block_s * 16 + epsilon, 1.0 - block_s * 10 - epsilon);
	objectInfo[98]->texture_coords[2] = glm::vec2(block_s * 16 + block_s * 2 - epsilon, 1.0 - block_s * 10 - epsilon);
	objectInfo[98]->texture_coords[3] = glm::vec2(block_s * 16 + block_s * 2 - epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	//shotgun texture
	objectInfo[99]->texture_coords[0] = glm::vec2(block_s * 20 + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	objectInfo[99]->texture_coords[1] = glm::vec2(block_s * 20 + epsilon, 1.0 - block_s * 10 - epsilon);
	objectInfo[99]->texture_coords[2] = glm::vec2(block_s * 20 + block_s * 2 - epsilon, 1.0 - block_s * 10 - epsilon);
	objectInfo[99]->texture_coords[3] = glm::vec2(block_s * 20 + block_s * 2 - epsilon, 1.0 - block_s * 10 - block_s + epsilon);

	//ambient objects
	ambientController = AmbientController(dayTime);
	//sun
	ambientController.sun.sprite_size = glm::vec2(4.f, 4.f);
	ambientController.texture_coords[0][0] = glm::vec2(10 * block_s + epsilon, 23 * block_s + epsilon);
	ambientController.texture_coords[0][1] = glm::vec2(10 * block_s + epsilon, 26 * block_s - epsilon);
	ambientController.texture_coords[0][2] = glm::vec2(13 * block_s - epsilon, 26 * block_s - epsilon);
	ambientController.texture_coords[0][3] = glm::vec2(13 * block_s - epsilon, 23 * block_s + epsilon);
	//moon1
	ambientController.moon[0].sprite_size = glm::vec2(2.f, 2.f);
	ambientController.texture_coords[1][0] = glm::vec2(13 * block_s + epsilon, 23 * block_s + epsilon);
	ambientController.texture_coords[1][1] = glm::vec2(13 * block_s + epsilon, 25 * block_s - epsilon);
	ambientController.texture_coords[1][2] = glm::vec2(15 * block_s - epsilon, 25 * block_s - epsilon);
	ambientController.texture_coords[1][3] = glm::vec2(15 * block_s - epsilon, 23 * block_s + epsilon);
	//moon2
	ambientController.moon[1].sprite_size = glm::vec2(2.f, 2.f);
	ambientController.texture_coords[2][0] = glm::vec2(15 * block_s + epsilon, 23 * block_s + epsilon);
	ambientController.texture_coords[2][1] = glm::vec2(15 * block_s + epsilon, 25 * block_s - epsilon);
	ambientController.texture_coords[2][2] = glm::vec2(17 * block_s - epsilon, 25 * block_s - epsilon);
	ambientController.texture_coords[2][3] = glm::vec2(17 * block_s - epsilon, 23 * block_s + epsilon);
	//clouds and far clouds
	ambientController.clouds[0].position = { 0.f, ScreenHeight - ScreenWidth * 0.125f };
	ambientController.clouds[1].position = { ScreenWidth, ScreenHeight - ScreenWidth * 0.125f };
	ambientController.far_clouds[0].position = { 0.f, ScreenHeight - ScreenWidth * 0.13f };
	ambientController.far_clouds[1].position = { ScreenWidth * 0.5, ScreenHeight - ScreenWidth * 0.13f };
	ambientController.far_clouds[2].position = { ScreenWidth, ScreenHeight - ScreenWidth * 0.13f };
	ambientController.texture_coords[3][0] = glm::vec2(epsilon, 12 * block_s + epsilon);
	ambientController.texture_coords[3][1] = glm::vec2(epsilon, 14 * block_s - epsilon);
	ambientController.texture_coords[3][2] = glm::vec2(16 * block_s - epsilon, 14 * block_s - epsilon);
	ambientController.texture_coords[3][3] = glm::vec2(16 * block_s - epsilon, 12 * block_s + epsilon);
	//sky stars
	ambientController.sky_stars[0].position = { ScreenWidth * 0.5, 0.f };
	ambientController.sky_stars[1].position = { 0.f, ScreenHeight * 0.f };
	ambientController.sky_stars[2].position = { -ScreenWidth * 0.5, 0.f };
	ambientController.sky_stars[3].position = { ScreenWidth * 0.5, ScreenHeight * 0.5 };
	ambientController.sky_stars[4].position = { 0.f, ScreenHeight * 0.5f };
	ambientController.sky_stars[5].position = { -ScreenWidth * 0.5, ScreenHeight * 0.5 };
	ambientController.texture_coords[4][0] = glm::vec2(epsilon, 3 * block_s + epsilon);
	ambientController.texture_coords[4][1] = glm::vec2(epsilon, 12 * block_s - epsilon);
	ambientController.texture_coords[4][2] = glm::vec2(16 * block_s - epsilon, 12 * block_s - epsilon);
	ambientController.texture_coords[4][3] = glm::vec2(16 * block_s - epsilon, 3 * block_s + epsilon);

	//craftable items info
	craftable_items[0] = CraftableItem(38, 1, Nothing, std::vector<CraftingPair>{CraftingPair{ 6, 15 }}); //workbench
	craftable_items[1] = CraftableItem(28, 2, Nothing, std::vector<CraftingPair>{CraftingPair{ 6, 1 }}); //2 oak platforms
	craftable_items[2] = CraftableItem(22, 3, Nothing, std::vector<CraftingPair>{CraftingPair{ 6, 1 }, CraftingPair{ 73, 1 }}); //3 torches
	craftable_items[3] = CraftableItem(25, 3, Nothing, std::vector<CraftingPair>{CraftingPair{ 14, 1 }, CraftingPair{ 22, 3 }}); //3 ice torches
	craftable_items[4] = CraftableItem(39, 1, Workbench, std::vector<CraftingPair>{CraftingPair{ 1, 20 }, CraftingPair{ 22, 3 }, CraftingPair{ 6, 4 }}); //furnace
	craftable_items[5] = CraftableItem(40, 1, Workbench, std::vector<CraftingPair>{CraftingPair{ 71, 5 }}); //anvil

	craftable_items[6] = CraftableItem(70, 1, Furnace, std::vector<CraftingPair>{CraftingPair{ 9, 3 }}); //copper ingot
	craftable_items[7] = CraftableItem(71, 1, Furnace, std::vector<CraftingPair>{CraftingPair{ 10, 3 }}); //iron ingot
	craftable_items[8] = CraftableItem(72, 1, Furnace, std::vector<CraftingPair>{CraftingPair{ 11, 4 }}); //gold ingot

	craftable_items[9] = CraftableItem(74, 25, Workbench, std::vector<CraftingPair>{CraftingPair{ 6, 1 }, CraftingPair{ 1, 1 }}); //wooden arrow
	craftable_items[10] = CraftableItem(75, 10, Nothing, std::vector<CraftingPair>{CraftingPair{ 74, 10 }, CraftingPair{22, 1}}); //flaming arrow
	craftable_items[11] = CraftableItem(76, 10, Nothing, std::vector<CraftingPair>{CraftingPair{ 74, 10 }, CraftingPair{25, 1}}); //frostburn arrow

	//player info
	//standing pos
	player.tex_coords[0] = glm::vec2(epsilon, 17 * block_s + epsilon);
	player.tex_coords[1] = glm::vec2(epsilon, 20 * block_s - epsilon);
	player.tex_coords[2] = glm::vec2(2 * block_s - epsilon, 20 * block_s - epsilon);
	player.tex_coords[3] = glm::vec2(2 * block_s - epsilon, 17 * block_s + epsilon);
	//jumping pos
	player.tex_coords[4] = glm::vec2(2 * block_s + epsilon, 17 * block_s + epsilon);
	player.tex_coords[5] = glm::vec2(2 * block_s + epsilon, 20 * block_s - epsilon);
	player.tex_coords[6] = glm::vec2(4 * block_s - epsilon, 20 * block_s - epsilon);
	player.tex_coords[7] = glm::vec2(4 * block_s - epsilon, 17 * block_s + epsilon);
	//walk 1
	player.tex_coords[8] = glm::vec2(4 * block_s + epsilon, 17 * block_s + epsilon);
	player.tex_coords[9] = glm::vec2(4 * block_s + epsilon, 20 * block_s - epsilon);
	player.tex_coords[10] = glm::vec2(6 * block_s - epsilon, 20 * block_s - epsilon);
	player.tex_coords[11] = glm::vec2(6 * block_s - epsilon, 17 * block_s + epsilon);
	//walk 2
	player.tex_coords[12] = glm::vec2(6 * block_s + epsilon, 17 * block_s + epsilon);
	player.tex_coords[13] = glm::vec2(6 * block_s + epsilon, 20 * block_s - epsilon);
	player.tex_coords[14] = glm::vec2(8 * block_s - epsilon, 20 * block_s - epsilon);
	player.tex_coords[15] = glm::vec2(8 * block_s - epsilon, 17 * block_s + epsilon);
	//walk 3
	player.tex_coords[16] = glm::vec2(10 * block_s + epsilon, 17 * block_s + epsilon);
	player.tex_coords[17] = glm::vec2(10 * block_s + epsilon, 20 * block_s - epsilon);
	player.tex_coords[18] = glm::vec2(12 * block_s - epsilon, 20 * block_s - epsilon);
	player.tex_coords[19] = glm::vec2(12 * block_s - epsilon, 17 * block_s + epsilon);
	//walk 4
	player.tex_coords[20] = glm::vec2(12 * block_s + epsilon, 17 * block_s + epsilon);
	player.tex_coords[21] = glm::vec2(12 * block_s + epsilon, 20 * block_s - epsilon);
	player.tex_coords[22] = glm::vec2(14 * block_s - epsilon, 20 * block_s - epsilon);
	player.tex_coords[23] = glm::vec2(14 * block_s - epsilon, 17 * block_s + epsilon);
	//entities info
	entityInfo[0] = new ProjectileInfo(Arrow, 74, 5, 1, true, glm::vec2(1.f, 1.f), false, false);
	entityInfo[1] = new ProjectileInfo(Arrow, 75, 7, 1, true, glm::vec2(1.f, 1.f), true, true, AppliableEffect(5, 5.f), AppliableLight(3.f, glm::vec3(1.f, 1.f, 0.f)));
	entityInfo[2] = new ProjectileInfo(Arrow, 76, 9, 1, true, glm::vec2(1.f, 1.f), true, true, AppliableEffect(6, 5.f), AppliableLight(3.f, glm::vec3(0.f, 0.9f, 1.f)));
	entityInfo[3] = new ProjectileInfo(Arrow, 77, 10, 9999, false, glm::vec2(1.f, 1.f), false, true, AppliableLight(3.f, glm::vec3(1.f, 0.9f, 0.9f)));
	entityInfo[4] = new ProjectileInfo(Arrow, 78, 14, 1, true, glm::vec2(1.f, 1.f), false, true, AppliableLight(3.f, glm::vec3(0.306f, 0.086f, 0.702f)));
	entityInfo[5] = new ProjectileInfo(Arrow, 79, 16, 1, true, glm::vec2(1.f, 1.f), false, true, AppliableLight(3.f, glm::vec3(1.f, 0.76f, 0.03f)));

	entityInfo[11] = new ProjectileInfo(MagicBall, -1, 9999999, 1, false, glm::vec2(2.f, 1.f), false, true, AppliableLight(3.f, glm::vec3(0.506f, 0.286f, 0.902f))); //NULL projectile
	entityInfo[12] = new ProjectileInfo(Bullet, 97, 7, 1, false, glm::vec2(1.f, 1.f), false, true, AppliableLight(0.5f, glm::vec3(0.5f, 0.5f, 0.f))); //bullet projectile
	entityInfo[13] = new ProjectileInfo(Throwable, 94, 10, 4, true, glm::vec2(1.f, 1.f), false, false); //shuriken projectile
	entityInfo[14] = new ProjectileInfo(Throwable, 95, 5, 2, true, glm::vec2(1.f, 1.f), false, false); //throwing knife projectile

	glm::vec2* tex_ptr;
	for (int i = 0; i < 6; i++) { //projectile arrows tex
		tex_ptr = entityInfo[i]->get_tex_coords_ptr();
		tex_ptr[0] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + epsilon, 1.0 - block_s * 9 - block_s + epsilon);
		tex_ptr[1] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + epsilon, 1.0 - block_s * 9 - epsilon);
		tex_ptr[2] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + block_s - epsilon, 1.0 - block_s * 9 - epsilon);
		tex_ptr[3] = glm::vec2(block_s * 26 + 0.6875 * block_s * i + 0.03125 * block_s + block_s - epsilon, 1.0 - block_s * 9 - block_s + epsilon);
	}
	//"NULL" projectile
	tex_ptr = entityInfo[11]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(block_s * 14 + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	tex_ptr[1] = glm::vec2(block_s * 14 + epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[2] = glm::vec2(block_s * 14 + block_s * 2 - epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[3] = glm::vec2(block_s * 14 + block_s * 2 - epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	//bullet projectile
	tex_ptr = entityInfo[12]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(block_s * 19 + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	tex_ptr[1] = glm::vec2(block_s * 19 + epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[2] = glm::vec2(block_s * 19 + block_s - epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[3] = glm::vec2(block_s * 19 + block_s - epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	//shuriken projectile
	tex_ptr = entityInfo[13]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(block_s * 11 + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	tex_ptr[1] = glm::vec2(block_s * 11 + epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[2] = glm::vec2(block_s * 11 + block_s - epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[3] = glm::vec2(block_s * 11 + block_s - epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	//throwing knife
	tex_ptr = entityInfo[14]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(block_s * 12 + epsilon, 1.0 - block_s * 10 - block_s + epsilon);
	tex_ptr[1] = glm::vec2(block_s * 12 + epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[2] = glm::vec2(block_s * 12 + block_s - epsilon, 1.0 - block_s * 10 - epsilon);
	tex_ptr[3] = glm::vec2(block_s * 12 + block_s - epsilon, 1.0 - block_s * 10 - block_s + epsilon);

	entityInfo[6] = new SlimeInfo("Slime", 30, 5, 0, 12.f, 3.f, glm::vec2(1.375f, 1.18f), glm::vec2(2.f, 2.f), false);
	std::vector<DropInfo>& enemy_drop_info = entityInfo[6]->get_drop_v();
	enemy_drop_info.reserve(2);
	enemy_drop_info.emplace_back(DropInfo(66, 100.f, 5, 10));
	enemy_drop_info.emplace_back(DropInfo(73, 80.f, 1, 2));
	tex_ptr = entityInfo[6]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(block_s * 10 + epsilon, block_s * 23 + epsilon);
	tex_ptr[1] = glm::vec2(block_s * 10 + epsilon, block_s * 25 - epsilon);
	tex_ptr[2] = glm::vec2(block_s * 12 - epsilon, block_s * 25 - epsilon);
	tex_ptr[3] = glm::vec2(block_s * 12 - epsilon, block_s * 23 + epsilon);
	tex_ptr[4] = glm::vec2(block_s * 12 + epsilon, block_s * 23 + epsilon);
	tex_ptr[5] = glm::vec2(block_s * 12 + epsilon, block_s * 25 - epsilon);
	tex_ptr[6] = glm::vec2(block_s * 14 - epsilon, block_s * 25 - epsilon);
	tex_ptr[7] = glm::vec2(block_s * 14 - epsilon, block_s * 23 + epsilon);

	entityInfo[7] = new ZombieInfo("Zombie", 500, 10, 2, 12.f, 2.f, glm::vec2(1.5f, 2.85f), glm::vec2(2.f, 3.f), true);
	std::vector<DropInfo>& enemy_drop_info2 = entityInfo[7]->get_drop_v();
	enemy_drop_info2.reserve(2);
	enemy_drop_info2.emplace_back(DropInfo(66, 100, 20));
	enemy_drop_info2.emplace_back(DropInfo(67, 50, 1));

	tex_ptr = entityInfo[7]->get_tex_coords_ptr();
	//standing pos
	tex_ptr[0] = glm::vec2(epsilon, 14 * block_s + epsilon);
	tex_ptr[1] = glm::vec2(epsilon, 17 * block_s - epsilon);
	tex_ptr[2] = glm::vec2(2 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[3] = glm::vec2(2 * block_s - epsilon, 14 * block_s + epsilon);
	//jumping pos
	tex_ptr[4] = glm::vec2(2 * block_s + epsilon, 14 * block_s + epsilon);
	tex_ptr[5] = glm::vec2(2 * block_s + epsilon, 17 * block_s - epsilon);
	tex_ptr[6] = glm::vec2(4 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[7] = glm::vec2(4 * block_s - epsilon, 14 * block_s + epsilon);
	//walk 1
	tex_ptr[8] = glm::vec2(4 * block_s + epsilon, 14 * block_s + epsilon);
	tex_ptr[9] = glm::vec2(4 * block_s + epsilon, 17 * block_s - epsilon);
	tex_ptr[10] = glm::vec2(6 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[11] = glm::vec2(6 * block_s - epsilon, 14 * block_s + epsilon);
	//walk 2
	tex_ptr[12] = glm::vec2(6 * block_s + epsilon, 14 * block_s + epsilon);
	tex_ptr[13] = glm::vec2(6 * block_s + epsilon, 17 * block_s - epsilon);
	tex_ptr[14] = glm::vec2(8 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[15] = glm::vec2(8 * block_s - epsilon, 14 * block_s + epsilon);
	//walk 3
	tex_ptr[16] = glm::vec2(8 * block_s + epsilon, 14 * block_s + epsilon);
	tex_ptr[17] = glm::vec2(8 * block_s + epsilon, 17 * block_s - epsilon);
	tex_ptr[18] = glm::vec2(10 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[19] = glm::vec2(10 * block_s - epsilon, 14 * block_s + epsilon);
	//walk 4
	tex_ptr[20] = glm::vec2(10 * block_s + epsilon, 14 * block_s + epsilon);
	tex_ptr[21] = glm::vec2(10 * block_s + epsilon, 17 * block_s - epsilon);
	tex_ptr[22] = glm::vec2(12 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[23] = glm::vec2(12 * block_s - epsilon, 14 * block_s + epsilon);

	entityInfo[8] = new FlyingEyeInfo("Demon Eye", 60, 18, 2, 12.f, 3.f, glm::vec2(1.f, 1.f), glm::vec2(3.f, 1.5f), false);
	std::vector<DropInfo>& enemy_drop_info3 = entityInfo[8]->get_drop_v();
	enemy_drop_info3.reserve(2);
	enemy_drop_info3.emplace_back(DropInfo(66, 100.f, 75));
	enemy_drop_info3.emplace_back(DropInfo(73, 80.f, 1, 2));
	tex_ptr = entityInfo[8]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(12 * block_s + epsilon, 16 * block_s + epsilon);
	tex_ptr[1] = glm::vec2(12 * block_s + epsilon, 17 * block_s - epsilon);
	tex_ptr[2] = glm::vec2(14 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[3] = glm::vec2(14 * block_s - epsilon, 16 * block_s + epsilon);
	tex_ptr[4] = glm::vec2(14 * block_s + epsilon, 16 * block_s + epsilon);
	tex_ptr[5] = glm::vec2(14 * block_s + epsilon, 17 * block_s - epsilon);
	tex_ptr[6] = glm::vec2(16 * block_s - epsilon, 17 * block_s - epsilon);
	tex_ptr[7] = glm::vec2(16 * block_s - epsilon, 16 * block_s + epsilon);

	entityInfo[9] = new FlyingEyeInfo("Dilated Eye", 50, 17, 0, 12.f, 3.f, glm::vec2(1.f, 1.f), glm::vec2(3.f, 1.5f), false);
	std::vector<DropInfo>& enemy_drop_info4 = entityInfo[9]->get_drop_v();
	enemy_drop_info4.reserve(2);
	enemy_drop_info4.emplace_back(DropInfo(66, 100.f, 75));
	enemy_drop_info4.emplace_back(DropInfo(73, 80.f, 1, 2));
	tex_ptr = entityInfo[9]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(12 * block_s + epsilon, 15 * block_s + epsilon);
	tex_ptr[1] = glm::vec2(12 * block_s + epsilon, 16 * block_s - epsilon);
	tex_ptr[2] = glm::vec2(14 * block_s - epsilon, 16 * block_s - epsilon);
	tex_ptr[3] = glm::vec2(14 * block_s - epsilon, 15 * block_s + epsilon);
	tex_ptr[4] = glm::vec2(14 * block_s + epsilon, 15 * block_s + epsilon);
	tex_ptr[5] = glm::vec2(14 * block_s + epsilon, 16 * block_s - epsilon);
	tex_ptr[6] = glm::vec2(16 * block_s - epsilon, 16 * block_s - epsilon);
	tex_ptr[7] = glm::vec2(16 * block_s - epsilon, 15 * block_s + epsilon);

	entityInfo[10] = new FlyingEyeInfo("Green Eye", 60, 19, 0, 12.f, 3.f, glm::vec2(1.f, 1.f), glm::vec2(3.f, 1.5f), false);
	std::vector<DropInfo>& enemy_drop_info5 = entityInfo[10]->get_drop_v();
	enemy_drop_info5.reserve(2);
	enemy_drop_info5.emplace_back(DropInfo(66, 100.f, 75));
	enemy_drop_info5.emplace_back(DropInfo(73, 80.f, 1, 2));
	tex_ptr = entityInfo[10]->get_tex_coords_ptr();
	tex_ptr[0] = glm::vec2(12 * block_s + epsilon, 14 * block_s + epsilon);
	tex_ptr[1] = glm::vec2(12 * block_s + epsilon, 15 * block_s - epsilon);
	tex_ptr[2] = glm::vec2(14 * block_s - epsilon, 15 * block_s - epsilon);
	tex_ptr[3] = glm::vec2(14 * block_s - epsilon, 14 * block_s + epsilon);
	tex_ptr[4] = glm::vec2(14 * block_s + epsilon, 14 * block_s + epsilon);
	tex_ptr[5] = glm::vec2(14 * block_s + epsilon, 15 * block_s - epsilon);
	tex_ptr[6] = glm::vec2(16 * block_s - epsilon, 15 * block_s - epsilon);
	tex_ptr[7] = glm::vec2(16 * block_s - epsilon, 14 * block_s + epsilon);

	//effects info
	effects[0] = new GameEffects::IncHP_LifeCrystal();
	effects[1] = new GameEffects::IncMANA_ManaCrystal();
	effects[2] = new GameEffects::IncreaseDefense();
	effects[3] = new GameEffects::IncreaseSpeed();
	effects[4] = new GameEffects::IncreaseRegeneration();
	effects[5] = new GameEffects::Burning();
	effects[6] = new GameEffects::FrostBurning();
	effects[7] = new GameEffects::Poisoning();
	effects[8] = new GameEffects::EffectBase(EffectType::isDebuff, "Potion Sickness", false);
	effects[9] = new GameEffects::MushroomHealing();
	effects[10] = new GameEffects::PotionHealing();
	for (int i = 0; i < 7; i++) { //effects sprite tex coords
		tex_ptr = effects[i + 2]->sprite_coords;
		tex_ptr[0] = glm::vec2(block_s * i + epsilon, 1.0 - block_s * 11 - block_s + epsilon);
		tex_ptr[1] = glm::vec2(block_s * i + epsilon, 1.0 - block_s * 11 - epsilon);
		tex_ptr[2] = glm::vec2(block_s * i + block_s - epsilon, 1.0 - block_s * 11 - epsilon);
		tex_ptr[3] = glm::vec2(block_s * i + block_s - epsilon, 1.0 - block_s * 11 - block_s + epsilon);
	}
	//particles info
	particlesInfo[0].emitsLight = true; particlesInfo[0].light = AppliableLight(0.5f, glm::vec3(1.f, 1.f, 0.f));
	tex_ptr = particlesInfo[0].tex_coords;
	tex_ptr[0] = glm::vec2(block_s * 14 + epsilon, 1.0 - block_s * 6 - block_s + epsilon);
	tex_ptr[1] = glm::vec2(block_s * 14 + epsilon, 1.0 - block_s * 6 - epsilon);
	tex_ptr[2] = glm::vec2(block_s * 14 + block_s - epsilon, 1.0 - block_s * 6 - epsilon);
	tex_ptr[3] = glm::vec2(block_s * 14 + block_s - epsilon, 1.0 - block_s * 6 - block_s + epsilon);
	particlesInfo[1].emitsLight = true; particlesInfo[1].light = AppliableLight(0.5f, glm::vec3(0.f, 0.9f, 1.f));
	tex_ptr = particlesInfo[1].tex_coords;
	tex_ptr[0] = glm::vec2(block_s * 15 + epsilon, 1.0 - block_s * 6 - block_s + epsilon);
	tex_ptr[1] = glm::vec2(block_s * 15 + epsilon, 1.0 - block_s * 6 - epsilon);
	tex_ptr[2] = glm::vec2(block_s * 15 + block_s - epsilon, 1.0 - block_s * 6 - epsilon);
	tex_ptr[3] = glm::vec2(block_s * 15 + block_s - epsilon, 1.0 - block_s * 6 - block_s + epsilon);

	//reserve memory for objects
	projectiles.reserve(100);
	entities.reserve(500);
	dropped_items.reserve(1000);
	particles_v.reserve(200);
	damage_text.reserve(100);

	//projection and view matrices
	viewMatrix = glm::mat4(1.f);
	projectionMatrix = glm::ortho(0.f, ScreenWidth, 0.f, ScreenHeight);

	//shader programs
	sprite_SP_ptr = new ShaderProgram("Resources/shaders/sprites.vert", "Resources/shaders/sprites.frag");
	entity_sprite_SP_ptr = new ShaderProgram("Resources/shaders/sprites2.vert", "Resources/shaders/sprites.frag");
	ambient_sprite_SP_ptr = new ShaderProgram("Resources/shaders/ambient sprites.vert", "Resources/shaders/ambient sprites.frag");
	//different shader program for ui sprites, that are not affected by world lighting
	UI_sprite_SP_ptr = new ShaderProgram("Resources/shaders/ui sprites.vert", "Resources/shaders/ui sprites.frag");

	sprite_SP_ptr->activate_shader();
	sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);
	GLuint u_id = glGetUniformLocation(sprite_SP_ptr->id, "u_textures");
	int samplers2D[2] = { 0, 1 };
	glUniform1iv(u_id, 2, samplers2D);

	UI_sprite_SP_ptr->activate_shader();
	UI_sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);
	u_id = glGetUniformLocation(UI_sprite_SP_ptr->id, "u_textures");
	glUniform1iv(u_id, 2, samplers2D);

	entity_sprite_SP_ptr->activate_shader();
	entity_sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);
	u_id = glGetUniformLocation(entity_sprite_SP_ptr->id, "u_textures");
	glUniform1iv(u_id, 2, samplers2D);

	ambient_sprite_SP_ptr->activate_shader();
	ambient_sprite_SP_ptr->set_Uniform_Mat4("projectionMatrix", projectionMatrix);
	ambient_sprite_SP_ptr->set_Uniform_Mat4("viewMatrix", glm::mat4(1.f));
	u_id = glGetUniformLocation(ambient_sprite_SP_ptr->id, "u_textures");
	glUniform1iv(u_id, 2, samplers2D);

	sprite_ssbo_p = new SSBO();
	sprite_ssbo_p->set_data(nullptr, sizeof(EntityData) * 1000, GL_DYNAMIC_DRAW);
	entity_sprite_SP_ptr->activate_shader();
	entity_sprite_SP_ptr->set_ssbo("SSBO", *sprite_ssbo_p, 0);

	spriteLightMapSSBO = new SSBO();
	//spriteLightMapSSBO->set_data(nullptr, sizeof(ShaderLightingInfo) * world_width * world_height, GL_DYNAMIC_DRAW);
	spriteLightMapSSBO->set_persistent_storage_data(nullptr, sizeof(ShaderLightingInfo) * world_width * world_height);
	lightMap_data = static_cast<ShaderLightingInfo*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ShaderLightingInfo) * world_width * world_height, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

	sprite_SP_ptr->activate_shader();
	sprite_SP_ptr->set_ssbo("SpriteLightsSSBO", *spriteLightMapSSBO, 1);
	entity_sprite_SP_ptr->activate_shader();
	entity_sprite_SP_ptr->set_ssbo("SpriteLightsSSBO", *spriteLightMapSSBO, 2);

	sprite_ambient_ssbo_p = new SSBO();
	sprite_ambient_ssbo_p->set_data(nullptr, sizeof(SpriteData) * 20, GL_DYNAMIC_DRAW);
	ambient_sprite_SP_ptr->activate_shader();
	ambient_sprite_SP_ptr->set_ssbo("SSBO", *sprite_ambient_ssbo_p, 3);
	
	//VAO, VBO and EBO for sprites
	sprite_vao_p = new VAO();
	sprite_vao_p->bind_VAO();

	sprite_vbo_p = new VBO();
	sprite_vbo_p->set_data(nullptr, MAX_VERTEX_AMOUNT * sizeof(Vertex2f), GL_DYNAMIC_DRAW);

	sprite_ebo_p = new EBO();
	//fill index buffer
	sprite_index_buffer = new GLuint[MAX_INDEX_AMOUNT];
	for (int i = 0; i < MAX_SQUARES; i++) {
		sprite_index_buffer[i * 6] = sprite_index_buffer[i * 6 + 3] = i * 4;
		sprite_index_buffer[i * 6 + 2] = sprite_index_buffer[i * 6 + 4] = sprite_index_buffer[i * 6] + 2;
		sprite_index_buffer[i * 6 + 1] = sprite_index_buffer[i * 6 + 2] - 1;
		sprite_index_buffer[i * 6 + 5] = sprite_index_buffer[i * 6 + 2] + 1;
	}
	//allocate memory for vertices buffer
	sprite_vertices_buffer = new Vertex2f[13648 * 4]; //the max size of buffer for drawing blocks is 12848 (6324 for blocks and 6324 for walls) with 50% zoom + 1000 for dropped items
	//just for fun, 25% zoom
	//sprite_vertices_buffer = new Vertex2f[25296 * 4];

	sprite_ebo_p->set_data(sprite_index_buffer, MAX_INDEX_AMOUNT * sizeof(GLuint), GL_STATIC_DRAW);

	sprite_vao_p->link_Attribute(*sprite_vbo_p, 0, 2, GL_FLOAT, sizeof(Vertex2f), (void*)0);
	sprite_vao_p->link_Attribute(*sprite_vbo_p, 1, 2, GL_FLOAT, sizeof(Vertex2f), (void*)(2 * sizeof(float)));
	sprite_vao_p->link_Attribute(*sprite_vbo_p, 2, 1, GL_FLOAT, sizeof(Vertex2f), (void*)(4 * sizeof(float)));
	sprite_vao_p->unbind_VAO();
	sprite_vbo_p->unbind_VBO();
	sprite_ebo_p->unbind_EBO();

	//init VAO, VBO, EBO for instanced rendering(used for entities and ambient sprites)
	GLfloat vertices[] = {
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f
	};
	GLuint indices[] = {
		0, 1, 2,
		0, 2, 3
	};
	instance_vao_p = new VAO();
	instance_vao_p->bind_VAO();
	instance_vbo_p = new VBO(vertices, sizeof(vertices));
	instance_ebo_p = new EBO(indices, sizeof(indices));
	instance_vao_p->link_Attribute(*instance_vbo_p, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
	instance_vao_p->unbind_VAO();
	instance_vbo_p->unbind_VBO();
	instance_ebo_p->unbind_EBO();

	//init inventory slots
	init_inventory_buffer();

	//init color shader program
	color_ui_SP = new ShaderProgram("Resources/shaders/color.vert", "Resources/shaders/color.frag");
	color_ui_SP->activate_shader();
	color_ui_SP->set_Uniform_Mat4("projectionMatrix", projectionMatrix);
	color_ui_SP->set_Uniform_Mat4("modelMatrix", glm::mat4(1.f));

	//init VAO, VBO, EBO for ui elements(that use color SP)
	ui_elements_vao = new VAO;
	ui_elements_vao->bind_VAO();
	ui_elements_vbo = new VBO;
	ui_elements_vbo->set_data(nullptr, MAX_UI_COLOR_ELEMENTS * sizeof(ColorVertex2f) * 4, GL_DYNAMIC_DRAW);
	ui_elements_ebo = new EBO;
	ui_elements_ebo->set_data(sprite_index_buffer, MAX_UI_COLOR_ELEMENTS * 6 * sizeof(GLuint), GL_STATIC_DRAW);
	ui_elements_vao->link_Attribute(*ui_elements_vbo, 0, 2, GL_FLOAT, 6 * sizeof(float), (void*)0);
	ui_elements_vao->link_Attribute(*ui_elements_vbo, 1, 4, GL_FLOAT, 6 * sizeof(float), (void*)(2 * sizeof(float)));

	//init inventory VAO, VBO, EBO for slots(color SP) and objects(sprites SP)
	inventory_vao = new VAO();
	inventory_vao->bind_VAO();
	inventory_vbo = new VBO();
	inventory_vbo->set_data(nullptr, INVENTORY_SIZE * sizeof(ColorVertex2f) * 4, GL_DYNAMIC_DRAW);
	inventory_ebo = new EBO();
	inventory_ebo->set_data(sprite_index_buffer, INVENTORY_SIZE * 6 * sizeof(GLuint), GL_STATIC_DRAW);
	inventory_vao->link_Attribute(*inventory_vbo, 0, 2, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
	inventory_vao->link_Attribute(*inventory_vbo, 1, 4, GL_FLOAT, 6 * sizeof(GLfloat), (void*)(2*sizeof(float)));
	inventory_vao->unbind_VAO();
	inventory_vbo->unbind_VBO();
	inventory_ebo->unbind_EBO();

	inventory_objects_vao = new VAO();
	inventory_objects_vao->bind_VAO();
	inventory_objects_vbo = new VBO();
	inventory_objects_vbo->set_data(nullptr, INVENTORY_SIZE * 4 * sizeof(Vertex2f), GL_DYNAMIC_DRAW);
	inventory_objects_ebo = new EBO();
	inventory_objects_ebo->set_data(sprite_index_buffer, INVENTORY_SIZE * 6 * sizeof(GLuint), GL_STATIC_DRAW);
	inventory_objects_vao->link_Attribute(*inventory_objects_vbo, 0, 2, GL_FLOAT, sizeof(Vertex2f), (void*)0);
	inventory_objects_vao->link_Attribute(*inventory_objects_vbo, 1, 2, GL_FLOAT, sizeof(Vertex2f), (void*)(2 * sizeof(float)));
	inventory_objects_vao->link_Attribute(*inventory_objects_vbo, 2, 1, GL_FLOAT, sizeof(Vertex2f), (void*)(4 * sizeof(float)));
	inventory_objects_vao->unbind_VAO();
	inventory_objects_vbo->unbind_VBO();
	inventory_objects_ebo->unbind_EBO();

	for (int i = 0; i < 10; i++) {
		inventory_array[i].object = ItemObject(isBlock, 1 + rand() % 24);
		inventory_array[i].amount = rand() % 901;
	}
	/*for (int i = 10; i < 20; i++) {
		inventory_array[i].object = ItemObject(isComplexObject, i + 28);
		inventory_array[i].amount = 5 + rand() % 6;
	}*/
	for (int i = 20; i < 26; i++) {
		inventory_array[i].object = ItemObject(isWeapon, i + 28);
		inventory_array[i].amount = 1;
	}
	inventory_array[26].object = ItemObject(isWall, 32); //stone wall
	inventory_array[26].amount = 500;
	inventory_array[27].object = ItemObject(isBlock, 31); //tree top
	inventory_array[27].amount = 200;
	inventory_array[28].object = ItemObject(isBlock, 22); //torch
	inventory_array[28].amount = 200;
	inventory_array[29].object = ItemObject(isBlock, 25); //ice torch
	inventory_array[29].amount = 200;
	inventory_array[30].object = ItemObject(isBlock, 28); //platform
	inventory_array[30].amount = 200;
	inventory_array[31].object = ItemObject(isBlock, 6); //oak planks
	inventory_array[31].amount = 900;
	inventory_array[32].object = ItemObject(isCoin, 68); //gold coin
	inventory_array[32].amount = 77;
	inventory_array[33].object = ItemObject(isAmmo, 74); //arrow
	inventory_array[33].amount = 500;
	inventory_array[34].object = ItemObject(isAmmo, 75); //flaming arrow
	inventory_array[34].amount = 500;
	inventory_array[35].object = ItemObject(isAmmo, 76); //ice flaming arrow
	inventory_array[35].amount = 500;
	inventory_array[36].object = ItemObject(isAmmo, 77); //arrow
	inventory_array[36].amount = 500;
	inventory_array[37].object = ItemObject(isAmmo, 78); //flaming arrow
	inventory_array[37].amount = 500;
	inventory_array[38].object = ItemObject(isAmmo, 79); //ice flaming arrow
	inventory_array[38].amount = 500;
	inventory_array[39].object = ItemObject(isWeapon, 63); //golden sword
	inventory_array[39].amount = 1;
	inventory_array[40].object = ItemObject(isComplexObject, 80); //life crystal
	inventory_array[40].amount = 50;
	inventory_array[41].object = ItemObject(isConsumable, 87); //mana crystal
	inventory_array[41].amount = 50;
	inventory_array[42].object = ItemObject(isPotion, 88); //healing potion
	inventory_array[42].amount = 50;
	inventory_array[43].object = ItemObject(isPotion, 89); //defense
	inventory_array[43].amount = 50;
	inventory_array[44].object = ItemObject(isPotion, 90); //speed
	inventory_array[44].amount = 50;
	inventory_array[45].object = ItemObject(isPotion, 91); //regen
	inventory_array[45].amount = 50;
	inventory_array[46].object = ItemObject(isWeapon, 60); //golden pickaxe
	inventory_array[46].amount = 1;

	inventory_array[10].object = ItemObject(isWeapon, 93); //magic wand
	inventory_array[10].amount = 1;
	inventory_array[11].object = ItemObject(isWeapon, 96); //Game.cpp
	inventory_array[11].amount = 1;
	inventory_array[12].object = ItemObject(isAmmo, 97); //bullets
	inventory_array[12].amount = 500;
	inventory_array[13].object = ItemObject(isWeapon, 98); //pistol
	inventory_array[13].amount = 1;
	inventory_array[14].object = ItemObject(isWeapon, 99); //shotgun
	inventory_array[14].amount = 1;
	inventory_array[15].object = ItemObject(isWeapon, 94); //shuriken
	inventory_array[15].amount = 300;
	inventory_array[16].object = ItemObject(isWeapon, 95); //throwing knofe
	inventory_array[16].amount = 300;
	inventory_array[17].object = ItemObject(isWeapon, 100); //trident
	inventory_array[17].amount = 1;
	inventory_array[18].object = ItemObject(isWeapon, 65); //golden bow
	inventory_array[18].amount = 1;

	entity_SP = new ShaderProgram("Resources/shaders/color.vert", "Resources/shaders/color.frag");
	entity_SP->activate_shader();
	entity_SP->set_Uniform_Mat4("projectionMatrix", projectionMatrix);
	entity_vao = new VAO;
	entity_vao->bind_VAO();
	entity_vbo = new VBO;
	entity_vbo->set_data(nullptr, 10 * 24 * 4, GL_DYNAMIC_DRAW); //for 10 entities
	entity_ebo = new EBO;
	entity_ebo->set_data(sprite_index_buffer, 10 * 6 * sizeof(GLuint), GL_STATIC_DRAW); //for 10 entities
	entity_vao->link_Attribute(*entity_vbo, 0, 2, GL_FLOAT, 6 * sizeof(float), (void*)0);
	entity_vao->link_Attribute(*entity_vbo, 1, 4, GL_FLOAT, 6 * sizeof(float), (void*)(2 * sizeof(float)));

	player.hitbox.center = glm::vec2(300 * BLOCK_VISIBLE_SIZE, 920 * BLOCK_VISIBLE_SIZE);
	player.hitbox.size = glm::vec2(1.5f * BLOCK_VISIBLE_SIZE, 2.9f * BLOCK_VISIBLE_SIZE);
	player.sprite_size = glm::vec2(2.f * BLOCK_VISIBLE_SIZE, 3.f * BLOCK_VISIBLE_SIZE);
	camera.dX = -player.hitbox.center.x + ScreenWidth / 2;
	camera.dY = -player.hitbox.center.y + ScreenHeight / 2;
	camera.rightBorderDx = -world_width * BLOCK_VISIBLE_SIZE + ScreenWidth + camera.scalingDx;
	camera.topBorderDy = -world_height * BLOCK_VISIBLE_SIZE + ScreenHeight + camera.scalingDy;

	//text manager
	text_manager = new Text();
	text_manager->set_projection_mat(projectionMatrix);
	text_manager->set_view_matrix(glm::mat4(1.f));

	//audio manager
	init_audio();

	//openGL settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//shader uniforms
	sprite_SP_ptr->activate_shader();
	sprite_SP_ptr->set_uniform_float("blockSize", BLOCK_VISIBLE_SIZE);
	sprite_SP_ptr->set_Uniform_iVec2("worldSize", glm::vec2(world_width, world_height));
	entity_sprite_SP_ptr->activate_shader();
	entity_sprite_SP_ptr->set_uniform_float("blockSize", BLOCK_VISIBLE_SIZE);
	entity_sprite_SP_ptr->set_Uniform_iVec2("worldSize", glm::vec2(world_width, world_height));

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			lightMap_data[j * world_width + i] = ShaderLightingInfo{ 0.f, 0.f, 0.f, 0.f, 0.f };
		}
	}
}

void Game::init_inventory_buffer() {
	float Xpos;
	float Ypos = ScreenHeight * 0.90;
	ColorVertex2f* ptr = inventory_vert_buf;
	for (int i = 0; i < 5; i++) { //main 50 slots
		Xpos = ScreenWidth * 0.01;
		for (int j = 0; j < 10; j++) {
			ptr->vertices = { Xpos, Ypos };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05};
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;

			Xpos += ScreenHeight * 0.055;
		}
		Ypos -= ScreenHeight * 0.055;
	}
	Xpos = ScreenWidth * 0.01 + 10 * 0.055 * ScreenHeight;
	for (int i = 0; i < 2; i++) { //8 slots (4 for coins and ammo)
		Ypos = 0.845 * ScreenHeight;
		for (int j = 0; j < 4; j++) {
			ptr->vertices = { Xpos, Ypos };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;

			Ypos -= ScreenHeight * 0.055;
		}
		Xpos += ScreenHeight * 0.055;
	}
	Ypos = 0.6 * ScreenHeight;
	Xpos = ScreenWidth * 0.99 - 0.055 * ScreenHeight;
	for (int i = 0; i < 8; i++) { //8 slots for armor and accessories
		ptr->vertices = { Xpos, Ypos };
		ptr->color = { 0.f, 0.f, 0.7f, 0.7f };
		ptr++;
		ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
		ptr->color = { 0.f, 0.f, 0.7f, 0.7f };
		ptr++;
		ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
		ptr->color = { 0.f, 0.f, 0.7f, 0.7f };
		ptr++;
		ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
		ptr->color = { 0.f, 0.f, 0.7f, 0.7f };
		ptr++;

		Ypos -= ScreenHeight * 0.055;
	}

	
	ptr = inv_chest_slots_buf;
	//40 slots for chests (used with inventory when any chest is open by the player)
	Ypos = 0.625 * ScreenHeight;
	for (int i = 0; i < 4; i++) {
		Xpos = ScreenWidth * 0.01;
		for (int j = 0; j < 10; j++) {
			ptr->vertices = { Xpos, Ypos };
			ptr->color = { 0.35f, 0.25f, 0.15f, 0.7f };
			ptr++;
			ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
			ptr->color = { 0.35f, 0.25f, 0.15f, 0.7f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
			ptr->color = { 0.35f, 0.25f, 0.15f, 0.7f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
			ptr->color = { 0.35f, 0.25f, 0.15f, 0.7f };
			ptr++;

			Xpos += ScreenHeight * 0.055;
		}
		Ypos -= ScreenHeight * 0.055;
	}

	ptr = crafting_system.helper_slots;
	//70 helper slots for crafting
	Ypos = 0.625 * ScreenHeight - 0.055 * ScreenHeight;
	for (int i = 0; i < 5; i++) {
		Xpos = ScreenWidth * 0.01 + 11 * 0.055 * ScreenHeight;
		for (int j = 0; j < 14; j++) {
			ptr->vertices = { Xpos, Ypos };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;
			ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
			ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
			ptr++;

			Xpos += ScreenHeight * 0.055;
		}
		Ypos -= ScreenHeight * 0.055;
	}

	ptr = crafting_system.main_slots;
	//main slots
	//main crafting slot
	Ypos = 0.215 * ScreenHeight;
	Xpos = ScreenWidth * 0.01 + ScreenHeight * 0.0425;
	ptr->vertices = { Xpos, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.075 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.075 , Ypos + ScreenHeight * 0.075 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.075, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	//up first
	Ypos = 0.295 * ScreenHeight;
	Xpos = ScreenWidth * 0.01 + ScreenHeight * 0.055;
	ptr->vertices = { Xpos, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	//up second
	Ypos = 0.35 * ScreenHeight;
	Xpos = ScreenWidth * 0.01 + ScreenHeight * 0.055;
	ptr->vertices = { Xpos, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	//down first
	Ypos = 0.16 * ScreenHeight;
	Xpos = ScreenWidth * 0.01 + ScreenHeight * 0.055;
	ptr->vertices = { Xpos, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	//down second
	Ypos = 0.105 * ScreenHeight;
	Xpos = ScreenWidth * 0.01 + ScreenHeight * 0.055;
	ptr->vertices = { Xpos, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;
	ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
	ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
	ptr++;

	//needed items slots
	ptr = crafting_system.needed_items_slots;
	Ypos = 0.2275 * ScreenHeight;
	Xpos = ScreenWidth * 0.01 + ScreenHeight * 0.1225;
	for (int i = 0; i < 10; i++) {
		ptr->vertices = { Xpos, Ypos };
		ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
		ptr++;
		ptr->vertices = { Xpos, Ypos + ScreenHeight * 0.05 };
		ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
		ptr++;
		ptr->vertices = { Xpos + ScreenHeight * 0.05 , Ypos + ScreenHeight * 0.05 };
		ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
		ptr++;
		ptr->vertices = { Xpos + ScreenHeight * 0.05, Ypos };
		ptr->color = { 0.f, 0.f, 0.7f, 0.5f };
		ptr++;

		Xpos += ScreenHeight * 0.055;
	}
}

void Game::init_main_buttons() {
	main_buttons.clear();
	main_buttons = {
		//main menu
		Button("Play", glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.75), glm::vec2(ScreenWidth * 0.25, ScreenHeight * 0.075), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f)),
		Button("Options", glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.65), glm::vec2(ScreenWidth * 0.25, ScreenHeight * 0.075), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f)),
		Button("Exit", glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.55), glm::vec2(ScreenWidth * 0.25, ScreenHeight * 0.075), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f)),
		//world explorer
		Button("Play", glm::vec2(ScreenWidth * 0.25, ScreenHeight * 0.1), glm::vec2(ScreenWidth * 0.25, ScreenHeight * 0.075), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f)),
		Button("New", glm::vec2(ScreenWidth * 0.75, ScreenHeight * 0.1), glm::vec2(ScreenWidth * 0.25, ScreenHeight * 0.075), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f)),
		//world creator
		Button("Create", glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.4), glm::vec2(ScreenWidth * 0.25, ScreenHeight * 0.075), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f)),
		//rendering options
		Button("Fullscreen", glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.9), glm::vec2(ScreenWidth * 0.4, ScreenHeight * 0.05), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f))
	};
	text_field = TextField("", glm::vec2(ScreenWidth * 0.5, ScreenHeight * 0.5), glm::vec2(ScreenWidth * 0.4, ScreenHeight * 0.075), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
}

void Game::init_world_buttons() {
	world_buttons.clear();
	float y = ScreenHeight * 0.9;
	for (auto& savePath : save_Files) {
		world_buttons.emplace_back(Button(savePath, glm::vec2(ScreenWidth * 0.2, y), glm::vec2(ScreenWidth * 0.3, ScreenHeight * 0.05), glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.9f, 0.9f, 0.9f, 1.f)));
		y -= ScreenHeight * 0.075;
	}
}

void Game::init_audio() {
	audio_manager = new AudioManager();
	//sounds
	std::vector<std::string> sounds = {
		// file                           //id
		"bow_shoot.wav",                  //0
		"Item_3_potion.wav",			  //1
		"Item_14_explosion.wav",		  //2
		"Item_169_swing.wav",			  //3
		"minecraft-doors-close.wav",	  //4
		"minecraft-doors-open.wav",		  //5
		"NPC_Hit_1.wav",				  //6
		"NPC_Killed_1.wav",				  //7
		"NPC_Killed_2.wav",				  //8
		"Zombie_0.wav",					  //9
		"Zombie_1.wav",					  //10
		"magical_weapon.wav",             //11
		"Item_11_gun.wav",                //12
		"Item_36_shotgun.wav",            //13
		"Item_33_heavy_laser.wav",        //14
	};
	for (auto& sound : sounds) {
		audio_manager->load_Sound("Resources/Audio/sounds/" + sound);
	}
	//music
	std::vector<std::string> music_tracks = {
		// file                           //id
		"Music-Overworld_Day.mp3",        //0
		"Music-Overworld_Night.mp3"       //1
	};
	for (auto& track : music_tracks) {
		audio_manager->load_Music("Resources/Audio/music/" + track);
	}
}

void Game::recolor_active_bar_slot() {
	inventory_vert_buf[active_bar_slot * 4].color = { 1.f, 1.f, 0.f, 1.f };
	inventory_vert_buf[active_bar_slot * 4 + 1].color = { 1.f, 1.f, 0.f, 1.f };
	inventory_vert_buf[active_bar_slot * 4 + 2].color = { 1.f, 1.f, 0.f, 1.f };
	inventory_vert_buf[active_bar_slot * 4 + 3].color = { 1.f, 1.f, 0.f, 1.f };
}

void Game::reset_active_bar_slot() {
	inventory_vert_buf[active_bar_slot * 4].color = { 0.f, 0.f, 0.7f, 0.5f };
	inventory_vert_buf[active_bar_slot * 4 + 1].color = { 0.f, 0.f, 0.7f, 0.5f };
	inventory_vert_buf[active_bar_slot * 4 + 2].color = { 0.f, 0.f, 0.7f, 0.5f };
	inventory_vert_buf[active_bar_slot * 4 + 3].color = { 0.f, 0.f, 0.7f, 0.5f };
}

void Game::update_item_info_box(ObjectType type, int id) {
	object_info_box[1].show_box = true;
	object_info_box[1].box_string = "";
	object_info_box[1].text_color = { 1.f, 1.f, 1.f, 1.f };
	if (type == isWeapon) {
		object_info_box[1].box_string += objectInfo[id]->name + '/';
		object_info_box[1].box_string += std::to_string(objectInfo[id]->get_damage()) + " damage/";
		object_info_box[1].box_string += std::to_string((int)objectInfo[id]->get_crit_chance()) + "% crit chance";
		object_info_box[1].text_color = getRainbowColor(rainbow_color_time);
	}
	else if (type == isAmmo) {
		object_info_box[1].box_string += objectInfo[id]->name + '/';
		object_info_box[1].box_string += std::to_string(objectInfo[id]->get_damage()) + " damage";
	}
	else {
		object_info_box[1].box_string += objectInfo[id]->name;
	}
	object_info_box[1].starting_pos = glm::vec2(mouse.mouseX + ScreenHeight * 0.02, mouse.mouseY + ScreenHeight * 0.01);
}

void Game::remove_inventory_item(int slot_index) {
	inventory_array[slot_index].amount--;
	if (inventory_array[slot_index].amount == 0) {
		inventory_array[slot_index].object = ItemObject(None, 0);
	}
}

void Game::update_effect_info_box(Effect& effect) {
	object_info_box[1].show_box = true;
	object_info_box[1].box_string = "";
	object_info_box[1].text_color = { 1.f, 1.f, 1.f, 1.f };
	object_info_box[1].box_string += effects[effect.id]->name + '/' + std::to_string((int)(effect.duration - effect.current_duration)) + "s";
	object_info_box[1].starting_pos = glm::vec2(mouse.mouseX * 1.25 + ScreenHeight * 0.02, mouse.mouseY * 1.25 - ScreenHeight * 0.24);
}

void Game::update_main_crafting_slot(int colorVertex_id, int craft_index, int &text_info_index, Vertex2f*& buffer_ptr) {
	float dX = 1.f, dY = 1.f;
	int id = crafting_system.available_crafts[craft_index];
	if (craftable_items[id].craftable_amount > 1) {
		inventory_text_info[text_info_index * 3] = craftable_items[id].craftable_amount;
		inventory_text_info[text_info_index * 3 + 1] = crafting_system.main_slots[colorVertex_id].vertices.x;
		inventory_text_info[text_info_index * 3 + 2] = crafting_system.main_slots[colorVertex_id].vertices.y;
		text_info_index++;
		inventory_text_size++;
	}
	int object_id = craftable_items[id].item_id;
	if (objectInfo[object_id]->objectType == isComplexObject) {
		float sizeX = objectInfo[object_id]->get_sizeX();
		float sizeY = objectInfo[object_id]->get_sizeY();
		if (sizeX > sizeY)
			dY = sizeY / sizeX;
		else
			dX = sizeX / sizeY;
	}
	float X = crafting_system.main_slots[colorVertex_id].vertices.x + ScreenHeight * (0.005 + 0.04 * ((1.f - dX) / 2.f));
	float Y = crafting_system.main_slots[colorVertex_id].vertices.y + ScreenHeight * (0.005 + 0.04 * ((1.f - dY) / 2.f));
	if (mouse.mouseX >= X && mouse.mouseX <= X + ScreenHeight * 0.04 &&
		mouse.mouseY >= Y && mouse.mouseY <= Y + ScreenHeight * 0.04)
		update_item_info_box(objectInfo[object_id]->objectType, object_id);
	glm::vec2* tex_ptr = objectInfo[object_id]->texture_coords;
	buffer_ptr->vertices = { X, Y };
	buffer_ptr->tex_vertices = tex_ptr[0];
	buffer_ptr->tex_id = 0.f;
	buffer_ptr++;
	buffer_ptr->vertices = { X, Y + ScreenHeight * 0.04 * dY };
	buffer_ptr->tex_vertices = tex_ptr[1];
	buffer_ptr->tex_id = 0.f;
	buffer_ptr++;
	buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y + ScreenHeight * 0.04 * dY };
	buffer_ptr->tex_vertices = tex_ptr[2];
	buffer_ptr->tex_id = 0.f;
	buffer_ptr++;
	buffer_ptr->vertices = { X + ScreenHeight * 0.04 * dX, Y };
	buffer_ptr->tex_vertices = tex_ptr[3];
	buffer_ptr->tex_id = 0.f;
	buffer_ptr++;
	buffer_size += 4;
}

void Game::craft_item(int craft_id) {
	int craftable_item_id = craftable_items[craft_id].item_id;
	ObjectType type = currently_moving_object.object.type;
	int id = currently_moving_object.object.id;
	//check if currently mouse is holding item with different id from craftable item (if it is, then we can't craft an item and return)
	if (type && id != craftable_item_id)
		return;
	//if they have the same id, then just add amount to currently holding item
	else if (type && id == craftable_item_id && item_is_stackable(id, type)) {
		currently_moving_object.amount += craftable_items[craft_id].craftable_amount;
	}
	//if mouse doesn't hold any item, then we craft item and give it to mouse cursor
	else {
		currently_moving_object.amount = craftable_items[craft_id].craftable_amount;
		currently_moving_object.object = ItemObject(objectInfo[craftable_item_id]->objectType, craftable_item_id);
	}
	//use items from inventory to craft needed item
	int amount = 0;
	for (auto& item : craftable_items[craft_id].items_needed) {
		amount = item.amount;
		for (int j = 0; j < 66; j++) {
			if (inventory_array[j].object.type && inventory_array[j].object.id == item.id) {
				if (amount >= inventory_array[j].amount) {
					amount -= inventory_array[j].amount;
					inventory_array[j].amount = 0;
					inventory_array[j].object = ItemObject(None, 0);
				}
				else {
					inventory_array[j].amount -= amount;
					amount = 0;
				}
			}
			if (amount <= 0)
				break;
		}
	}
}

std::vector<int> Game::get_available_crafts() {
	crafting_system.is_near_workbench = false;
	crafting_system.is_near_furnace = false;
	crafting_system.is_near_anvil = false;
	int leftX = (player.hitbox.center.x - player.hitbox.size.x * 0.5f) / BLOCK_VISIBLE_SIZE - 2;
	int rightX = leftX + 5;
	int bottomY = (player.hitbox.center.y - player.hitbox.size.y * 0.5f) / BLOCK_VISIBLE_SIZE - 2;
	int topY = bottomY + 6;
	if (leftX < 0) leftX = 0;
	if (rightX >= world_width) rightX = world_width - 1;
	if (bottomY < 0) bottomY = 0;
	if (topY >= world_height) topY = world_height - 1;

	for (int i = leftX; i <= rightX; i++) {
		for (int j = bottomY; j <= topY; j++) {
			if (sprites_Array[i][j].object.object_type == isComplexObject)
				switch (objectInfo[sprites_Array[i][j].object.object_id]->get_comp_obj_type()) {
				case isWorkbench:
					crafting_system.is_near_workbench = true;
					break;
				case isFurnace:
					crafting_system.is_near_furnace = true;
					break;
				case isAnvil:
					crafting_system.is_near_anvil = true;
					break;
				}
		}
	}

	std::vector<int> available_crafts;
	int amount = 0;
	bool can_craft = false;
	bool hasCondition = false;
	//for each craft
	for (int i = 0; i < MAX_CRAFTS_AVAILABLE; i++) {
		hasCondition = true;
		switch (craftable_items[i].condition) {
		case Workbench:
			if (!crafting_system.is_near_workbench) hasCondition = false;
			break;
		case Furnace:
			if (!crafting_system.is_near_furnace) hasCondition = false;
			break;
		case Anvil:
			if (!crafting_system.is_near_anvil) hasCondition = false;
			break;
		}
		if (hasCondition) {
			for (auto& item : craftable_items[i].items_needed) {
				can_craft = false;
				amount = 0;
				for (int j = 0; j < 66; j++) {
					if (inventory_array[j].object.type && inventory_array[j].object.id == item.id) {
						can_craft = true;
						amount += inventory_array[j].amount;
					}
				}
				if (amount < item.amount)
					can_craft = false;
				if (!can_craft)
					break;
			}
			if (can_craft)
				available_crafts.emplace_back(i);
		}
	}
	return available_crafts;
}

void Game::use_item_with_effect(int inventory_slot) {
	int object_id = inventory_array[inventory_slot].object.id;
	int effect_id = objectInfo[object_id]->effectId;
	EffectType type = effects[effect_id]->type;
	if (type == isImmediateBuff) {
		if (effects[effect_id]->applyEffect(player.stats))
			remove_inventory_item(inventory_slot);
	}
	else if (type == isHealing) {
		if (!player.stats.hasPotionSickness) {
			effects[effect_id]->applyEffect(player.stats);
			player.stats.hasPotionSickness = true;
			//apply potion sickness effect (id = 8)
			player.effects.emplace_back(Effect(60.f, 8));
			remove_inventory_item(inventory_slot);
		}
	}
	else {
		apply_player_effect(Effect(objectInfo[object_id]->effectDuration, effect_id));
		remove_inventory_item(inventory_slot);
	}
}

void Game::apply_player_effect(Effect effect) {
	int size = player.effects.size();
	for (int i = 0; i < size; i++) {
		//update effect duration if player already has it
		if (player.effects[i].id == effect.id) {
			player.effects[i].current_duration = 0.f;
			return;
		}
	}
	effects[effect.id]->applyEffect(player.stats);
	player.effects.emplace_back(effect);
}

void Game::apply_entity_effect(Effect effect, EntityStats& stats) {
	int size = stats.effects.size();
	for (int i = 0; i < size; i++) {
		//update effect duration if entity already has it
		if (stats.effects[i].id == effect.id) {
			stats.effects[i].current_duration = 0.f;
			return;
		}
	}
	effects[effect.id]->applyEntityEffect(stats);
	stats.effects.emplace_back(effect);
}

void Game::activate_weapon(WeaponType type) {
	active_weapon.weapon_type = type;
	if (type == isThrowable) {
		if (active_weapon.projectile_attack_current_cd <= 0.f) {
			float x = mouse.mouseX - player.hitbox.center.x - camera.dX;
			float y = mouse.mouseY - player.hitbox.center.y - camera.dY;
			if (x == 0.f) {
				if (y < 0) active_weapon.angle = glm::radians(90.f);
				else active_weapon.angle = glm::radians(-90.f);
			}
			else active_weapon.angle = std::atan(y / x);
			if (player.hitbox.center.x + camera.dX >= mouse.mouseX) {
				active_weapon.angle += glm::radians(180.f);
			}
			int weapon_id = inventory_array[active_bar_slot].object.id;
			active_weapon.projectile_attack_current_cd = objectInfo[weapon_id]->get_weapon_proj_cd();
			active_weapon.speed_factor = objectInfo[weapon_id]->get_speed_factor();
			throw_projectile(weapon_id);
		}
		return;
	}
	//apply new info
	active_weapon.isActive = true;
	int weapon_id = inventory_array[active_bar_slot].object.id;
	Smart_ptr<ObjectInfo>& weapon_info = objectInfo[weapon_id];
	active_weapon.weapon_id = weapon_id;
	active_weapon.sizeX = weapon_info->get_sizeX() * BLOCK_VISIBLE_SIZE;
	active_weapon.sizeY = weapon_info->get_sizeY() * BLOCK_VISIBLE_SIZE;
	active_weapon.speed_factor = weapon_info->get_speed_factor();
	active_weapon.hit_cd = weapon_info->get_hit_cd();
	active_weapon.projectile_attack_cd = weapon_info->get_weapon_proj_cd();
	//make new matrix, then translate and scale it for the player
	glm::mat4 mat(1.f);
	//update texture coords
	glm::vec2* tex_ptr = objectInfo[weapon_id]->texture_coords;
	active_weapon.tex_coords[0] = tex_ptr[0];
	active_weapon.tex_coords[1] = tex_ptr[1];
	active_weapon.tex_coords[2] = tex_ptr[2];
	active_weapon.tex_coords[3] = tex_ptr[3];
	//apply different angle based on weapon type
	if (type == isBow || type == isGun || type == isMagical || type == isShotgun) {
		float x = mouse.mouseX - player.hitbox.center.x - camera.dX;
		float y = mouse.mouseY - player.hitbox.center.y - camera.dY;
		if (x == 0.f) {
			if (y < 0) active_weapon.angle = glm::radians(90.f);
			else active_weapon.angle = glm::radians(-90.f);
		}
		else active_weapon.angle = std::atan(y / x);
		if (player.hitbox.center.x + camera.dX >= mouse.mouseX) {
			active_weapon.angle += glm::radians(180.f);
			active_weapon.render_upside_down = true;
		}
		else active_weapon.render_upside_down = false;

		mat = glm::translate(mat, glm::vec3(player.hitbox.center.x, player.hitbox.center.y + player.hitbox.size.y * 0.1, 0.f));
		mat = glm::rotate(mat, active_weapon.angle, glm::vec3(0.f, 0.f, 1.f));
		mat = glm::translate(mat, glm::vec3(-active_weapon.sizeX / 2 + BLOCK_VISIBLE_SIZE, -active_weapon.sizeY / 2, 0.f));
		mat = glm::scale(mat, glm::vec3(active_weapon.sizeX, active_weapon.sizeY, 0.f));
		
		if (active_weapon.projectile_attack_current_cd <= 0.f) {
			active_weapon.projectile_attack_current_cd = active_weapon.projectile_attack_cd;
			if (type == isBow) shoot_arrow(active_weapon.weapon_id);
			else if (type == isGun || type == isShotgun) shoot_bullet(active_weapon.weapon_id);
			else if (type == isMagical) shoot_magic(active_weapon.weapon_id);
		}
	}
	else { //pickaxe, axe, sword, hammer
		active_weapon.time_to_finish_swing = 150.f / (180.f * active_weapon.speed_factor);
		active_weapon.hitboxIsActive = true;
		active_weapon.hitbox.size = { BLOCK_VISIBLE_SIZE, BLOCK_VISIBLE_SIZE * 2 };
		if (player.hitbox.center.x + camera.dX >= mouse.mouseX) { //start swinging to left
			active_weapon.angle = 0.f;
			active_weapon.points_to_left = true;
		}
		else {  //start swinging to right
			active_weapon.angle = 90.f;
			active_weapon.points_to_left = false;
		}
		mat = glm::translate(mat, glm::vec3(player.hitbox.center.x, player.hitbox.center.y, 0.f));
		mat = glm::rotate(mat, glm::radians(active_weapon.angle), glm::vec3(0.f, 0.f, 1.f));
		glm::mat4 hitboxMatrix = mat;
		mat = glm::translate(mat, glm::vec3(BLOCK_VISIBLE_SIZE, BLOCK_VISIBLE_SIZE, 0.f));
		mat = glm::scale(mat, glm::vec3(active_weapon.sizeX, active_weapon.sizeY, 0.f));
		hitboxMatrix = glm::translate(hitboxMatrix, glm::vec3(active_weapon.sizeX + BLOCK_VISIBLE_SIZE / 2, active_weapon.sizeY + BLOCK_VISIBLE_SIZE / 2, 0.f));
		active_weapon.hitbox.transformMatrix = hitboxMatrix;
		//audio
		audio_manager->play_Sound(objectInfo[weapon_id]->get_weapon_usable_audio_id());
	}
	//apply matrix
	active_weapon.modelMatrix = mat;
}

bool Game::shoot_arrow(int weapon_id) {
	for (int i = 50; i <= 53; i++) {
		if (inventory_array[i].object.type == isAmmo && objectInfo[inventory_array[i].object.id]->get_ammo_type() == isArrow) {
			int projectile_id = objectInfo[inventory_array[i].object.id]->get_entity_id();
			inventory_array[i].amount--;
			if (!inventory_array[i].amount) {
				inventory_array[i].object = ItemObject(None, 0);
			}
			float V = BLOCK_VISIBLE_SIZE * active_weapon.speed_factor;
			float fx = std::cos(active_weapon.angle);
			float fy = std::sin(active_weapon.angle);
			int damage = objectInfo[weapon_id]->get_damage() + entityInfo[projectile_id]->get_dmg();
			bool isCrit = false;
			if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) {
				damage *= 2;
				isCrit = true;
			}
			if (entityInfo[projectile_id]->proj_uses_gravity()) {
				projectiles.emplace_back(Smart_ptr<EntityBase>(new GravityProjectile(projectile_id, V * fx, V * fy, active_weapon.angle,
					player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
					entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit, 90.f)));
			}
			else {
				projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, active_weapon.angle + glm::radians(90.f),
					player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
					entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
			}
			//audio
			audio_manager->play_Sound(objectInfo[weapon_id]->get_weapon_usable_audio_id());
			//light
			if (entityInfo[projectile_id]->emitsLight) {
				AppliableLight light = entityInfo[projectile_id]->light;
				update_lighting(projectiles.back()->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
			}
			return true;
		}
	}
	return false;
}

bool Game::shoot_bullet(int weapon_id) {
	for (int i = 50; i <= 53; i++) {
		if (inventory_array[i].object.type == isAmmo && objectInfo[inventory_array[i].object.id]->get_ammo_type() == isBullet) {
			int projectile_id = objectInfo[inventory_array[i].object.id]->get_entity_id();
			inventory_array[i].amount--;
			if (!inventory_array[i].amount) {
				inventory_array[i].object = ItemObject(None, 0);
			}
			float V = BLOCK_VISIBLE_SIZE * active_weapon.speed_factor, fx, fy;
			int damage = objectInfo[weapon_id]->get_damage() + entityInfo[projectile_id]->get_dmg();
			AppliableLight light = entityInfo[projectile_id]->light;
			bool hasLight = entityInfo[projectile_id]->emitsLight;
			bool isCrit = false;
			if (active_weapon.weapon_type == isShotgun) {
				//first bullet
				float angle = active_weapon.angle + glm::radians(float(-5 + rand() % 11));
				fx = std::cos(angle); fy = std::sin(angle);
				isCrit = false;
				if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) { damage *= 2; isCrit = true; }
				projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, angle,
					player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
					entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
				if (hasLight) {
					update_lighting(projectiles.back()->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
				}
				//second bullet
				angle = active_weapon.angle + glm::radians(float(-5 + rand() % 11));
				fx = std::cos(angle); fy = std::sin(angle);
				isCrit = false;
				if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) { damage *= 2; isCrit = true; }
				projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, angle,
					player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
					entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
				if (hasLight) {
					update_lighting(projectiles.back()->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
				}
				//third bullet
				angle = active_weapon.angle + glm::radians(float(-5 + rand() % 11));
				fx = std::cos(angle); fy = std::sin(angle);
				isCrit = false;
				if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) { damage *= 2; isCrit = true; }
				projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, angle,
					player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
					entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
				if (hasLight) {
					update_lighting(projectiles.back()->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
				}
				//fourth bullet
				angle = active_weapon.angle + glm::radians(float(-5 + rand() % 11));
				fx = std::cos(angle); fy = std::sin(angle);
				isCrit = false;
				if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) { damage *= 2; isCrit = true; }
				projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, angle,
					player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
					entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
				if (hasLight) {
					update_lighting(projectiles.back()->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
				}
			}
			else {
				fx = std::cos(active_weapon.angle);
				fy = std::sin(active_weapon.angle);
				isCrit = false;
				if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) {
					damage *= 2;
					isCrit = true;
				}
				projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, active_weapon.angle,
					player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
					entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
				//light
				if (hasLight) {
					update_lighting(projectiles.back()->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
				}
			}
			//audio
			audio_manager->play_Sound(objectInfo[weapon_id]->get_weapon_usable_audio_id());
			return true;
		}
	}
	return false;
}

bool Game::shoot_magic(int weapon_id) {
	float V = BLOCK_VISIBLE_SIZE * active_weapon.speed_factor;
	float fx = std::cos(active_weapon.angle);
	float fy = std::sin(active_weapon.angle);
	int projectile_id = objectInfo[weapon_id]->get_weapon_proj_id();
	int damage = objectInfo[weapon_id]->get_damage() + entityInfo[projectile_id]->get_dmg();
	bool isCrit = false;
	float angle = glm::degrees(active_weapon.angle);
	if (angle > 90.f) angle += 180.f;
	if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) {
		damage *= 2;
		isCrit = true;
	}
	projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, glm::radians(angle),
		player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
		entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
	//audio
	audio_manager->play_Sound(objectInfo[weapon_id]->get_weapon_usable_audio_id());
	//light
	if (entityInfo[projectile_id]->emitsLight) {
		AppliableLight light = entityInfo[projectile_id]->light;
		update_lighting(projectiles.back()->hitbox.center, light.light_color, light.light_radius * BLOCK_VISIBLE_SIZE, true);
	}
	return false;
}

bool Game::throw_projectile(int weapon_id) {
	float V = BLOCK_VISIBLE_SIZE * active_weapon.speed_factor;
	float fx = std::cos(active_weapon.angle);
	float fy = std::sin(active_weapon.angle);
	int projectile_id = objectInfo[weapon_id]->get_weapon_proj_id();
	int damage = entityInfo[projectile_id]->get_dmg();
	bool isCrit = false;
	float angle = glm::degrees(active_weapon.angle);
	if (angle > 90.f) angle += 180.f;
	if (1 + rand() % 100 <= objectInfo[weapon_id]->get_crit_chance()) {
		damage *= 2;
		isCrit = true;
	}
	if (entityInfo[projectile_id]->proj_uses_gravity()) {
		projectiles.emplace_back(Smart_ptr<EntityBase>(new GravityProjectile(projectile_id, V * fx, V * fy, active_weapon.angle,
			player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
			entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit, -90.f)));
	}
	else {
		projectiles.emplace_back(Smart_ptr<EntityBase>(new LinearProjectile(projectile_id, V * fx, V * fy, active_weapon.angle + glm::radians(-90.f),
			player.hitbox.center.x + BLOCK_VISIBLE_SIZE * fx, player.hitbox.center.y + player.hitbox.size.y * 0.1f + BLOCK_VISIBLE_SIZE * fy,
			entityInfo[projectile_id]->spriteSize, damage, entityInfo[projectile_id]->get_proj_max_enemy_hits(), isCrit)));
	}
	inventory_array[active_bar_slot].amount--;
	if (!inventory_array[active_bar_slot].amount) {
		inventory_array[active_bar_slot].object = ItemObject(None, 0);
	}
	//audio
	audio_manager->play_Sound(objectInfo[weapon_id]->get_weapon_usable_audio_id());
	return false;
}

void Game::update_active_weapon(float deltaTime) {
	glm::mat4 mat(1.f);
	//update is different based on weapon type
	WeaponType type = active_weapon.weapon_type;
	if (type == isBow || type == isGun || type == isMagical || type == isShotgun) {
		active_weapon.time_spent += deltaTime;
		if (!mouse.left_button) {
			active_weapon.time_spent = 0.f;
			active_weapon.isActive = false;
			active_weapon.render_upside_down = false;
			return;
		}
		float x = mouse.mouseX - player.hitbox.center.x - camera.dX;
		float y = mouse.mouseY - player.hitbox.center.y - camera.dY;
		if (x == 0.f) {
			if (y < 0) active_weapon.angle = glm::radians(90.f);
			else active_weapon.angle = glm::radians(-90.f);
		}
		else active_weapon.angle = std::atan(y / x);
		if (player.hitbox.center.x + camera.dX >= mouse.mouseX) {
			active_weapon.angle += glm::radians(180.f);
			active_weapon.render_upside_down = true;
		}
		else active_weapon.render_upside_down = false;

		mat = glm::translate(mat, glm::vec3(player.hitbox.center.x, player.hitbox.center.y + player.hitbox.size.y * 0.1f, 0.f));
		mat = glm::rotate(mat, active_weapon.angle, glm::vec3(0.f, 0.f, 1.f));
		mat = glm::translate(mat, glm::vec3(-active_weapon.sizeX / 2 + BLOCK_VISIBLE_SIZE, -active_weapon.sizeY / 2, 0.f));
		mat = glm::scale(mat, glm::vec3(active_weapon.sizeX, active_weapon.sizeY, 0.f));

		if (active_weapon.projectile_attack_current_cd <= 0.f) { //incorrect
			active_weapon.projectile_attack_current_cd = active_weapon.projectile_attack_cd;
			//MAKE SOUND!!!
			if (type == isBow) shoot_arrow(active_weapon.weapon_id);
			else if (type == isGun || type == isShotgun) shoot_bullet(active_weapon.weapon_id);
			else if (type == isMagical) shoot_magic(active_weapon.weapon_id);
		}
	}
	else { //pickaxe, axe, sword, hammer
		active_weapon.time_to_finish_swing -= deltaTime;
		if (active_weapon.points_to_left) {
			active_weapon.angle += deltaTime * 180.f * active_weapon.speed_factor;
			if (active_weapon.angle >= 150.f) {
				active_weapon.isActive = false;
				active_weapon.hitboxIsActive = false;
			}
		}
		else {
			active_weapon.angle -= deltaTime * 180.f * active_weapon.speed_factor;
			if (active_weapon.angle <= -60.f) {
				active_weapon.isActive = false;
				active_weapon.hitboxIsActive = false;
			}
		}
		mat = glm::translate(mat, glm::vec3(player.hitbox.center.x, player.hitbox.center.y, 0.f));
		mat = glm::rotate(mat, glm::radians(active_weapon.angle), glm::vec3(0.f, 0.f, 1.f));
		glm::mat4 hitboxMatrix = mat;
		mat = glm::translate(mat, glm::vec3(BLOCK_VISIBLE_SIZE, BLOCK_VISIBLE_SIZE, 0.f));
		mat = glm::scale(mat, glm::vec3(active_weapon.sizeX, active_weapon.sizeY, 0.f));
		hitboxMatrix = glm::translate(hitboxMatrix, glm::vec3(active_weapon.sizeX * 0.5 + BLOCK_VISIBLE_SIZE, active_weapon.sizeY * 0.5 + BLOCK_VISIBLE_SIZE, 0.f));
		hitboxMatrix = glm::rotate(hitboxMatrix, glm::radians(-45.f), glm::vec3(0.f, 0.f, 1.f));
		active_weapon.hitbox.transformMatrix = hitboxMatrix;
	}
	//apply matrix
	active_weapon.modelMatrix = mat;
}

void Game::drop_item(int id, float X, float Y, int amount, float Xinc, bool pick_cd, float cd_time) {
	DroppedItem item;
	item.id = id;
	item.amount = amount;
	item.hitbox.center.x = X;
	item.hitbox.center.y = Y;
	item.Xinc = Xinc;
	item.has_pick_cd = pick_cd;
	//item.cd_time = cd_time;
	ObjectType type = objectInfo[id]->objectType;
	if (type == isComplexObject) {
		item.hitbox.size.x = objectInfo[id]->get_sizeX() * BLOCK_VISIBLE_SIZE * 0.8;
		item.hitbox.size.y = objectInfo[id]->get_sizeY() * BLOCK_VISIBLE_SIZE * 0.8;
	}
	else if (type == isWeapon) {
		item.hitbox.size.x = objectInfo[id]->get_sizeX() * BLOCK_VISIBLE_SIZE * 0.8;
		item.hitbox.size.y = objectInfo[id]->get_sizeY() * BLOCK_VISIBLE_SIZE * 0.8;
	}
	else {
		item.hitbox.size.x = BLOCK_VISIBLE_SIZE * 0.8;
		item.hitbox.size.y = BLOCK_VISIBLE_SIZE * 0.8;
	}
	dropped_items.emplace_back(item);
}

void Game::drop_enemy_items(int enemy_id, float xPos, float yPos) {
	std::vector<DropInfo>& drop_v = entityInfo[enemy_id]->get_drop_v();
	int size = drop_v.size();
	for (int i = 0; i < size; i++) {
		//drop with specific chance
		if (1 + rand() % 100 <= (int)drop_v[i].chance) {
			//calculate amount based on range (if no range then just one item)
			int amount = drop_v[i].amount1 + (rand() % (drop_v[i].amount2 - drop_v[i].amount1 + 1));
			float Xinc;
			if (rand() % 2) Xinc = 1.f;
			else Xinc = -1.f;
			Xinc *= (rand() % 3) * BLOCK_VISIBLE_SIZE;
			drop_item(drop_v[i].id, xPos, yPos, amount, Xinc, false, 0.f);
		}
	}
}

bool Game::try_to_pick_item(DroppedItem& item) {
	int i = 0;
	int index_of_first_free_slot = 0;
	bool has_free_slot = false;
	ObjectType type;
	int id;
	while (i < 50) {
		type = inventory_array[i].object.type;
		id = inventory_array[i].object.id;
		if (!type && !has_free_slot) {
			has_free_slot = true;
			index_of_first_free_slot = i;
		}
		if (type && id == item.id && inventory_array[i].amount < 999 && item_is_stackable(id, type)) {
			inventory_array[i].amount += item.amount;
			item.amount -= item.amount;
			return true;
		}
		i++;
	}
	if (has_free_slot) {
		inventory_array[index_of_first_free_slot].object = ItemObject(objectInfo[item.id]->objectType, item.id);
		inventory_array[index_of_first_free_slot].amount += item.amount;
		item.amount -= item.amount;
		return true;
	}
	return false;
}

bool Game::item_is_stackable(int id, ObjectType type) {
	if (type == isWeapon) {
		if (objectInfo[id]->is_stackable())
			return true;
		else
			return false;
	}
	else if (type == isArmor || type == isAccessory)
		return false;
	else
		return true;
}

void Game::update_wall_visibility(int x, int y) {
	if (x >= 0 && x < world_width && y >= 0 && y < world_height) {
		for (int i = x - 1; i <= x + 1; i++) {
			for (int j = y - 1; j <= y + 1; j++) {
				if (i >= 0 && i < world_width && j >= 0 && j < world_height)
					if (!(is_solid_block(i, j))) {
						sprites_Array[x][y].wall_is_visible = true;
						return;
					}
			}
		}
		sprites_Array[x][y].wall_is_visible = false;
	}
}

void Game::update_lighting(glm::vec2 light_pos, glm::vec3 light_color, float radius, bool add) {
	int left = (light_pos.x - radius) / BLOCK_VISIBLE_SIZE;
	int right = (light_pos.x + radius) / BLOCK_VISIBLE_SIZE;
	int bottom = (light_pos.y - radius) / BLOCK_VISIBLE_SIZE;
	int top = (light_pos.y + radius) / BLOCK_VISIBLE_SIZE;
	if (std::fmod(light_pos.x, BLOCK_VISIBLE_SIZE) == 0.f) {
		right--;
	}
	if (std::fmod(light_pos.y, BLOCK_VISIBLE_SIZE) == 0.f) {
		top--;
	}
	if (left < 0) left = 0;
	if (right >= world_width) right = world_width - 1;
	if (bottom < 0) bottom = 0;
	if (top >= world_height) top = world_height - 1;

	float radiusSq = radius * radius;
	float blockLeft, blockRight, blockTop, blockBottom;
	float dX, dY, distanceSq, lightIntensity;
	for (int i = left; i <= right; i++) {
		for (int j = bottom; j <= top; j++) {
			blockLeft = i * BLOCK_VISIBLE_SIZE;
			blockRight = blockLeft + BLOCK_VISIBLE_SIZE;
			blockBottom = j * BLOCK_VISIBLE_SIZE;
			blockTop = blockBottom + BLOCK_VISIBLE_SIZE;
			//calculate closest points to light_pos and get distances
			dX = light_pos.x - std::max(std::min(light_pos.x, blockRight), blockLeft);
			dY = light_pos.y - std::max(std::min(light_pos.y, blockTop), blockBottom);
			distanceSq = dX * dX + dY * dY;
			//check if intersects with circle
			if (distanceSq <= radiusSq) {
				lightIntensity = (radius - std::sqrt(distanceSq)) / radius;
				if (add) {
					ShaderLightingInfo info = lightMap_data[j * world_width + i];
					info.r += light_color.x * lightIntensity;
					info.g += light_color.y * lightIntensity;
					info.b += light_color.z * lightIntensity;
					lightMap_data[j * world_width + i] = info;
				}
				else {
					ShaderLightingInfo info = lightMap_data[j * world_width + i];
					info.r -= light_color.x * lightIntensity;
					info.g -= light_color.y * lightIntensity;
					info.b -= light_color.z * lightIntensity;
					lightMap_data[j * world_width + i] = info;
				}
			}
		}
	}
}

void Game::update_block_light_impact(int column, int line, bool use_recursive_method, int recursive_depth) {
	if (recursive_depth == 0)
		return;
	bool isUpdated = false;
	ShaderLightingInfo blockLightInfo = lightMap_data[line * world_width + column];
	bool isSolidBlock = is_solid_block(column, line);
	int wallId = sprites_Array[column][line].wall_id;
	if (isSolidBlock) wallId = -1;
	//if no wall and no solid block
	if (!isSolidBlock && !wallId) {
		if (blockLightInfo.global_light_impact != 1.f || blockLightInfo.source_light_impact != 1.f) {
			blockLightInfo.global_light_impact = 1.f;
			blockLightInfo.source_light_impact = 1.f;
			isUpdated = true;
			//std::cout << "[" << column << ", " << line << "] " << "no wall and no solid block, set both parameters to 1.f" << std::endl;
		}
	}
	else {
		float max_nearby_src_imp = 0.f, max_nearby_glb_imp = 0.f;
		bool air_nearby = false;
		bool no_solid_block_nearby = false; //at least in one nearby slot there is no solid block
		bool wall_nearby = false; //at least in one nearby slot there is only wall and no solid block

		if (column - 1 >= 0) {
			ShaderLightingInfo left = lightMap_data[line * world_width + column - 1];
			if (left.global_light_impact > max_nearby_glb_imp) max_nearby_glb_imp = left.global_light_impact;
			if (left.source_light_impact > max_nearby_src_imp) max_nearby_src_imp = left.source_light_impact;
			if (!is_solid_block(column - 1, line)) {
				no_solid_block_nearby = true;
				if (!sprites_Array[column - 1][line].wall_id) {
					air_nearby = true;
				}
				else wall_nearby = true;
			}
		}
		if (column + 1 < world_width) {
			ShaderLightingInfo right = lightMap_data[line * world_width + column + 1];
			if (right.global_light_impact > max_nearby_glb_imp) max_nearby_glb_imp = right.global_light_impact;
			if (right.source_light_impact > max_nearby_src_imp) max_nearby_src_imp = right.source_light_impact;
			if (!is_solid_block(column + 1, line)) {
				no_solid_block_nearby = true;
				if (!sprites_Array[column + 1][line].wall_id) {
					air_nearby = true;
				}
				else wall_nearby = true;
			}
		}
		if (line - 1 >= 0) {
			ShaderLightingInfo bottom = lightMap_data[(line - 1) * world_width + column];
			if (bottom.global_light_impact > max_nearby_glb_imp) max_nearby_glb_imp = bottom.global_light_impact;
			if (bottom.source_light_impact > max_nearby_src_imp) max_nearby_src_imp = bottom.source_light_impact;
			if (!is_solid_block(column, line - 1)) {
				no_solid_block_nearby = true;
				if (!sprites_Array[column][line - 1].wall_id) {
					air_nearby = true;
				}
				else wall_nearby = true;
			}
		}
		if (line + 1 < world_height) {
			ShaderLightingInfo top = lightMap_data[(line + 1) * world_width + column];
			if (top.global_light_impact > max_nearby_glb_imp) max_nearby_glb_imp = top.global_light_impact;
			if (top.source_light_impact > max_nearby_src_imp) max_nearby_src_imp = top.source_light_impact;
			if (!is_solid_block(column, line + 1)) {
				no_solid_block_nearby = true;
				if (!sprites_Array[column][line + 1].wall_id) {
					air_nearby = true;
				}
				else wall_nearby = true;
			}
		}

		if (air_nearby && (blockLightInfo.global_light_impact != 1.f || blockLightInfo.source_light_impact != 1.f) ) {
			blockLightInfo.global_light_impact = 1.f;
			blockLightInfo.source_light_impact = 1.f;
			isUpdated = true;
			//std::cout << "[" << column << ", " << line << "] " << "air nearby, set both to 1.f" << std::endl;
		}
		else if (max_nearby_glb_imp == 0.f && blockLightInfo.global_light_impact != 0.f) {
			blockLightInfo.global_light_impact = 0.f;
			isUpdated = true;
			//std::cout << "[" << column << ", " << line << "] " << "max nearby global is 0.f, so set to 0.f" << std::endl;
		}
		else if (!air_nearby && max_nearby_glb_imp > 0.f && max_nearby_glb_imp - blockLightInfo.global_light_impact != 0.25f) {
			blockLightInfo.global_light_impact = max_nearby_glb_imp - 0.25f;
			isUpdated = true;
			//std::cout << "[" << column << ", " << line << "] " << "set 0.25 lower global than max nearby" << std::endl;
		}
		//additionally check source light impact if it's a solid block or a wall
		if (isSolidBlock) {
			if (wall_nearby && blockLightInfo.source_light_impact != 1.f) { //wall nearby, but what if it's under the solid block?
				blockLightInfo.source_light_impact = 1.f;
				isUpdated = true;
				//std::cout << "[" << column << ", " << line << "] " << "wall nearby, set source to 1.f" << std::endl;
			}
			else if (!no_solid_block_nearby && max_nearby_src_imp == 0.f && blockLightInfo.source_light_impact != 0.f) { //everywhere nearby are solid blocks
				blockLightInfo.source_light_impact = 0.f;
				isUpdated = true;
				//std::cout << "[" << column << ", " << line << "] " << "solid blocks nearby and max nearby src is 0.f, set src to 0.f" << std::endl;
			}
			else if (!no_solid_block_nearby && max_nearby_src_imp > 0.f && max_nearby_src_imp - blockLightInfo.source_light_impact != 0.25f) { //everywhere nearby are solid blocks
				blockLightInfo.source_light_impact = max_nearby_src_imp - 0.25f;
				isUpdated = true;
				//std::cout << "[" << column << ", " << line << "] " << "solid blocks nearby, set 0.25 lower than max nearby src " << std::endl;
			}
		}
		else if (wallId && blockLightInfo.source_light_impact != 1.f) {
			blockLightInfo.source_light_impact = 1.f;
			isUpdated = true;
			//std::cout << "[" << column << ", " << line << "] " << "is wall and no solid block, set source impact to 1.f" << std::endl;
		}
	}

	if (!isUpdated) return;
	lightMap_data[line * world_width + column] = blockLightInfo;
	
	if (use_recursive_method) {
		if (column - 1 >= 0) {
			update_block_light_impact(column - 1, line, true, recursive_depth - 1);
		}
		if (column + 1 < world_width) {
			update_block_light_impact(column + 1, line, true, recursive_depth - 1);
		}
		if (line - 1 >= 0) {
			update_block_light_impact(column, line - 1, true, recursive_depth - 1);
		}
		if (line + 1 < world_height) {
			update_block_light_impact(column, line + 1, true, recursive_depth - 1);
		}
	}
}

bool Game::is_solid_block(int column, int line) {
	if (sprites_Array[column][line].object.object_type == isBlock && objectInfo[sprites_Array[column][line].object.object_id]->get_block_type() == isSolidBlock)
		return true;
	else
		return false;
}

void Game::set_block(int x, int y, int id) {
	sprites_Array[x][y].object = GameObject(inventory_array[active_bar_slot].object.type, id);
	for (int i = x - 1; i <= x + 1; i++) {
		for (int j = y - 1; j <= y + 1; j++) {
			update_wall_visibility(i, j);
		}
	}

	if (objectInfo[id]->emitsLight) {
		update_lighting(glm::vec2(x * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, y * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f),
			objectInfo[id]->light_color, objectInfo[id]->light_radius * BLOCK_VISIBLE_SIZE, true);
	}

	update_block_light_impact(x, y, false, 1);
	if (x - 1 >= 0) {
		update_block_light_impact(x - 1, y, true, 30);
	}
	if (x + 1 < world_width) {
		update_block_light_impact(x + 1, y, true, 30);
	}
	if (y - 1 >= 0) {
		update_block_light_impact(x, y - 1, true, 30);
	}
	if (y + 1 < world_height) {
		update_block_light_impact(x, y + 1, true, 30);
	}
}

void Game::destroy_object(int x, int y) {
	//remove block
	int object_id = sprites_Array[x][y].object.object_id;
	sprites_Array[x][y].object = GameObject(None, 0);
	//update wall visibility
	for (int i = x - 1; i <= x + 1; i++) {
		for (int j = y - 1; j <= y + 1; j++) {
			if (i >= 0 && i < world_width && j >= 0 && j < world_height)
				sprites_Array[i][j].wall_is_visible = true;
		}
	}
	//remove lighting
	if (objectInfo[object_id]->emitsLight) {
		update_lighting(glm::vec2(x * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f, y * BLOCK_VISIBLE_SIZE + BLOCK_VISIBLE_SIZE * 0.5f),
			objectInfo[object_id]->light_color, objectInfo[object_id]->light_radius * BLOCK_VISIBLE_SIZE, false);
	}
	//update light impact
	update_block_light_impact(x, y, false, 1);
	if (x - 1 >= 0) {
		update_block_light_impact(x - 1, y, true, 30);
	}
	if (x + 1 < world_width) {
		update_block_light_impact(x + 1, y, true, 30);
	}
	if (y - 1 >= 0) {
		update_block_light_impact(x, y - 1, true, 30);
	}
	if (y + 1 < world_height) {
		update_block_light_impact(x, y + 1, true, 30);
	}
}

void Game::destroy_complex_object(int x, int y) {
	int object_id = sprites_Array[x][y].object.object_id;
	int width = x + objectInfo[object_id]->get_sizeX(), height = y + objectInfo[object_id]->get_sizeY();
	for (int i = x; i < width; i++)
		for (int j = y; j < height; j++) {
			sprites_Array[i][j].object = GameObject(None, 0);
		}
	//remove lighting
	if (objectInfo[object_id]->emitsLight) {
		update_lighting(glm::vec2((x + objectInfo[object_id]->get_sizeX() * 0.5f) * BLOCK_VISIBLE_SIZE, (y + objectInfo[object_id]->get_sizeY() * 0.5f) * BLOCK_VISIBLE_SIZE),
			objectInfo[object_id]->light_color, objectInfo[object_id]->light_radius * BLOCK_VISIBLE_SIZE, false);
	}
}

void Game::set_wall(int x, int y, unsigned short id) {
	sprites_Array[x][y].wall_id = id;
	//update light impact if no solid block
	if (!is_solid_block(x, y)) {
		update_block_light_impact(x, y, false, 1);
		if (x - 1 >= 0) {
			update_block_light_impact(x - 1, y, true, 30);
		}
		if (x + 1 < world_width) {
			update_block_light_impact(x + 1, y, true, 30);
		}
		if (y - 1 >= 0) {
			update_block_light_impact(x, y - 1, true, 30);
		}
		if (y + 1 < world_height) {
			update_block_light_impact(x, y + 1, true, 30);
		}
	}
}

void Game::destroy_wall(int x, int y) {
	sprites_Array[x][y].wall_id = 0;
	//update light impact
	update_block_light_impact(x, y, false, 1);
	if (x - 1 >= 0) {
		update_block_light_impact(x - 1, y, true, 30);
	}
	if (x + 1 < world_width) {
		update_block_light_impact(x + 1, y, true, 30);
	}
	if (y - 1 >= 0) {
		update_block_light_impact(x, y - 1, true, 30);
	}
	if (y + 1 < world_height) {
		update_block_light_impact(x, y + 1, true, 30);
	}
}

void Game::uninit() {
	sprite_vao_p->delete_VAO();
	sprite_vbo_p->delete_VBO();
	sprite_ebo_p->delete_EBO();
	sprite_ssbo_p->delete_SSBO();
	sprite_ambient_ssbo_p->delete_SSBO();
	spriteLightMapSSBO->bind_SSBO();
	if (lightMap_data)
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	lightMap_data = nullptr;
	spriteLightMapSSBO->delete_SSBO();

	inventory_objects_vao->delete_VAO();
	inventory_objects_vbo->delete_VBO();
	inventory_objects_ebo->delete_EBO();

	sprite_SP_ptr->delete_shader();
	entity_sprite_SP_ptr->delete_shader();
	ambient_sprite_SP_ptr->delete_shader();
	UI_sprite_SP_ptr->delete_shader();

	delete[]sprite_vertices_buffer;
	delete[]sprite_index_buffer;
	if (sprites_Array) {
		for (int i = 0; i < world_width; i++) {
			delete[] sprites_Array[i];
		}
		delete[] sprites_Array;
	}

	instance_vao_p->delete_VAO();
	instance_vbo_p->delete_VBO();
	instance_ebo_p->delete_EBO();

	ui_elements_vao->delete_VAO();
	ui_elements_vbo->delete_VBO();
	ui_elements_ebo->delete_EBO();

	inventory_vao->delete_VAO();
	inventory_vbo->delete_VBO();
	inventory_ebo->delete_EBO();
	color_ui_SP->delete_shader();

	entity_vao->delete_VAO();
	entity_vbo->delete_VBO();
	entity_ebo->delete_EBO();
	entity_SP->delete_shader();
}

void Game::generate_world() {
	//array of sprites
	sprites_Array = new WorldSlot * [world_width] {nullptr};
	for (int i = 0; i < world_width; i++) {
		sprites_Array[i] = new WorldSlot[world_height];
	}
	int biome_width = world_width / 10;
	srand(time(NULL));
	//generate flat surface with tiny waves
	for (int i = 0; i < world_width; i++) {
		int j = 0;
		//int point = 891 + sin(0.1 * i) * 20;
		int point = 899 + sin(i * 0.05) * 1;
		for (; j < point; j++) {  //stone layer

			sprites_Array[i][j].object = GameObject(isBlock, 1);
		}
		while (j < point + 8) { //dirt layer
			sprites_Array[i][j].object = GameObject(isBlock, 2);
			j++;
		}
		sprites_Array[i][j].object = GameObject(isBlock, 3); //dirt with grass
	}
	//generate some kind of desert on either left or right side of the map
	bool desertIsOnLeft = rand() % 2;
	int left_biome_border, right_biome_border;
	if (desertIsOnLeft) {
		left_biome_border = world_width * 0.3 - 1;
		right_biome_border = left_biome_border + biome_width;
	}
	else {
		left_biome_border = world_width * 0.6 - 1;
		right_biome_border = left_biome_border + biome_width;
	}
	float value = 0.f;
	for (int i = left_biome_border; i < right_biome_border; i++) {
		int point = 904 + sin(3.14159 * value / 180.) * 15;
		for (int j = 600; j < point; j++) {
			if (sprites_Array[i][j].object.object_type) {
				sprites_Array[i][j].object = GameObject(None, 0);
			}
			sprites_Array[i][j].object = GameObject(isBlock, 4);
		}
		value += 180 / 420.;
	}
	//generate oceans on both sides
	left_biome_border = 0;
	right_biome_border = left_biome_border + world_width * 0.05;
	value = 180.f;
	for (int i = left_biome_border; i < right_biome_border - 20; i++) { //place sand in a curve line and fill with water blocks on left side
		int point = 829 + (cos(3.14159 * value / 180.) + 1) * 80;
		int j = point - 40;
		for (; j < point; j++) {  //place sand
			if (sprites_Array[i][j].object.object_type) {
				sprites_Array[i][j].object = GameObject(None, 0);
			}
			sprites_Array[i][j].object = GameObject(isBlock, 4);
		}
		while (j <= 905) {  //place water
			if (sprites_Array[i][j].object.object_type) {
				sprites_Array[i][j].object = GameObject(None, 0);
			}
			sprites_Array[i][j].object = GameObject(isBlock, 12);
			j++;
		}
		while (j <= 907) {  //remove redundant blocks
			if (sprites_Array[i][j].object.object_type) {
				if (sprites_Array[i][j].object.object_id != 4) {
					sprites_Array[i][j].object = GameObject(None, 0);
				}
			}
			j++;
		}
		value += 90 / 190.;
	}
	for (int i = right_biome_border - 20; i < right_biome_border; i++) { //make straigh place for 20 blocks after the ocean
		int point = 908;
		for (int j = point - 40; j < point; j++) {
			if (sprites_Array[i][j].object.object_type) {
				sprites_Array[i][j].object = GameObject(None, 0);
			}
			sprites_Array[i][j].object = GameObject(isBlock, 4);
		}
	}
	right_biome_border = world_width - 1;
	left_biome_border = right_biome_border - world_width * 0.05;
	value = 90.f;
	for (int i = left_biome_border; i <= right_biome_border; i++) { //place sand in a curve line and fill with water blocks on right side
		int point = 829 + (cos(3.14159 * value / 180.) + 1) * 80;
		int j = point - 40;
		for (; j < point; j++) {  //place sand
			if (sprites_Array[i][j].object.object_type) {
				sprites_Array[i][j].object = GameObject(None, 0);
			}
			sprites_Array[i][j].object = GameObject(isBlock, 4);
		}
		while (j <= 905) {  //place water
			if (sprites_Array[i][j].object.object_type) {
				sprites_Array[i][j].object = GameObject(None, 0);
			}
			sprites_Array[i][j].object = GameObject(isBlock, 12);
			j++;
		}
		while (j <= 907) {  //remove redundant blocks
			if (sprites_Array[i][j].object.object_type) {
				if (sprites_Array[i][j].object.object_id != 4) {
					sprites_Array[i][j].object = GameObject(None, 0);
				}
			}
			j++;
		}
		value += 90 / 190.;
	}
	for (int i = left_biome_border - 20; i < left_biome_border; i++) { //make straigh place for 20 blocks after the ocean
		int point = 908;
		for (int j = point - 40; j < point; j++) {
			if (sprites_Array[i][j].object.object_type) {
				sprites_Array[i][j].object = GameObject(None, 0);
			}
			sprites_Array[i][j].object = GameObject(isBlock, 4);
		}
	}
}

void Game::save_world_in_file(const char* fileName) {
	std::ofstream write(fileName, std::ios::binary);
	ObjectType type;
	unsigned short value;
	write.write(reinterpret_cast<const char*>(&world_width), sizeof(unsigned short));
	write.write(reinterpret_cast<const char*>(&world_height), sizeof(unsigned short));
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			write.write(reinterpret_cast<const char*>(&sprites_Array[i][j].wall_id), sizeof(unsigned short));
			write.write(reinterpret_cast<const char*>(&sprites_Array[i][j].object.object_id), sizeof(unsigned short));
			type = sprites_Array[i][j].object.object_type;
			write.write(reinterpret_cast<const char*>(&type), sizeof(ObjectType));
			if (type == isCompObjPart) {
				value = sprites_Array[i][j].object.component->get_column();
				write.write(reinterpret_cast<const char*>(&value), sizeof(unsigned short));
				value = sprites_Array[i][j].object.component->get_line();
				write.write(reinterpret_cast<const char*>(&value), sizeof(unsigned short));
			}
			else if (type == isComplexObject) {
				ComplexObjectType complexType = objectInfo[sprites_Array[i][j].object.object_id]->get_comp_obj_type();
				if (complexType == isDoor) {
					uint8_t door_state = sprites_Array[i][j].object.component->get_door_state();
					write.write(reinterpret_cast<const char*>(&door_state), sizeof(uint8_t));
				}
				else if (complexType == isChest) {
					InventoryPair* chest_ptr = sprites_Array[i][j].object.component->get_chest_slots();
					for (int c = 0; c < 40; c++) {
						write.write(reinterpret_cast<const char*>(&chest_ptr[c].amount), sizeof(unsigned short));
						write.write(reinterpret_cast<const char*>(&chest_ptr[c].object.id), sizeof(unsigned short));
						write.write(reinterpret_cast<const char*>(&chest_ptr[c].object.type), sizeof(ObjectType));
					}
				}
			}
		}
	}
	write.close();
}

void Game::load_world_from_file(const char* fileName) {
	std::ifstream read(fileName, std::ios::binary);
	ObjectType type;
	unsigned short value, id, column, line;
	uint8_t door_state;
	read.read(reinterpret_cast<char*>(&world_width), sizeof(unsigned short));
	read.read(reinterpret_cast<char*>(&world_height), sizeof(unsigned short));

	sprites_Array = new WorldSlot * [world_width] {nullptr};
	for (int i = 0; i < world_width; i++) {
		sprites_Array[i] = new WorldSlot[world_height];
	}

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			read.read(reinterpret_cast<char*>(&sprites_Array[i][j].wall_id), sizeof(unsigned short));
			read.read(reinterpret_cast<char*>(&id), sizeof(unsigned short));
			read.read(reinterpret_cast<char*>(&type), sizeof(ObjectType));
			if (type == isCompObjPart) {
				read.read(reinterpret_cast<char*>(&column), sizeof(unsigned short));
				read.read(reinterpret_cast<char*>(&line), sizeof(unsigned short));
				sprites_Array[i][j].object = GameObject(type, id, new ComplexObjectPartComponent(column, line));
			}
			else if (type == isComplexObject) {
				ComplexObjectType complexType = objectInfo[sprites_Array[i][j].object.object_id]->get_comp_obj_type();
				if (complexType == isDoor) {
					read.read(reinterpret_cast<char*>(&door_state), sizeof(uint8_t));
					sprites_Array[i][j].object = GameObject(type, id, new DoorComponent(door_state));
				}
				else if (complexType == isChest) {
					sprites_Array[i][j].object = GameObject(type, id, new ChestComponent);
					InventoryPair* chest_ptr = sprites_Array[i][j].object.component->get_chest_slots();
					for (int c = 0; c < 40; c++) {
						read.read(reinterpret_cast<char*>(&chest_ptr[c].amount), sizeof(unsigned short));
						read.read(reinterpret_cast<char*>(&chest_ptr[c].object.id), sizeof(unsigned short));
						read.read(reinterpret_cast<char*>(&chest_ptr[c].object.type), sizeof(ObjectType));
					}
				}
			}
			else {
				sprites_Array[i][j].object = GameObject(type, id);
			}
		}
	}
	read.close();
}

void Game::create_the_world_thread() {
	generate_world();

	save_world_in_file(active_world.c_str());

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			update_wall_visibility(i, j);
			lightMap_data[j * world_width + i] = ShaderLightingInfo{ 0.f, 0.f, 0.f, 0.f, 0.f };
		}
	}
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			update_block_light_impact(i, j, true, 5);
		}
	}

	creating_the_world = false;
}

void Game::load_the_world_thread() {
	load_world_from_file(active_world.c_str());

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			update_wall_visibility(i, j);
		}
	}

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			update_block_light_impact(i, j, true, 5);

			Smart_ptr<ObjectInfo>& object = objectInfo[sprites_Array[i][j].object.object_id];
			if (object->emitsLight) {
				if (object->objectType == ObjectType::isBlock) {
					update_lighting(glm::vec2(BLOCK_VISIBLE_SIZE * (i + 0.5f), BLOCK_VISIBLE_SIZE * (j + 0.5f)), object->light_color, object->light_radius * BLOCK_VISIBLE_SIZE, true);
				}
				else {
					update_lighting(glm::vec2((i + object->get_sizeX() * 0.5f) * BLOCK_VISIBLE_SIZE, (j + object->get_sizeY() * 0.5f) * BLOCK_VISIBLE_SIZE),
						object->light_color, object->light_radius * BLOCK_VISIBLE_SIZE, true);
				}
			}
		}
	}

	loading_the_world = false;
}

void Game::exit_and_save_the_world_thread() {
	save_world_in_file(active_world.c_str());

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			lightMap_data[j * world_width + i] = ShaderLightingInfo{ 0.f, 0.f, 0.f, 0.f, 0.f };
		}
	}

	if (sprites_Array) {
		for (int i = 0; i < world_width; i++) {
			delete[] sprites_Array[i];
		}
		delete[] sprites_Array;
		sprites_Array = nullptr;
	}
	entities.clear();
	projectiles.clear();
	dropped_items.clear();
	damage_text.clear();
	particles_v.clear();

	saving_the_world = false;
}

void Game::load_available_saves() {
	std::string path = "Saves";
	for (const auto& entry : fs::directory_iterator(path))
		save_Files.push_back(entry.path().string());
}