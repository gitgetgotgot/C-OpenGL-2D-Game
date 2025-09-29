#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Hitbox_2D_AABB {
	glm::vec2 center;
	glm::vec2 size;
};

struct Hitbox_2D_OBB {
	glm::vec2 center;
	glm::vec2 size;
	float angle;
	glm::mat4 transformMatrix;

	void update_transform() {
		transformMatrix = glm::mat4(1.f);
		transformMatrix = glm::translate(transformMatrix, glm::vec3(center.x, center.y, 0.f));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f));
	}
};

enum CollisionType : uint8_t { NONE, LEFT, RIGHT, TOP, BOTTOM, CORNER };

namespace CollisionManager {

	inline bool checkCollisionAABB(Hitbox_2D_AABB& hit1, Hitbox_2D_AABB& hit2) {
		float h1Left = hit1.center.x - hit1.size.x * 0.5f;
		float h1Right = h1Left + hit1.size.x;
		float h2Left = hit2.center.x - hit2.size.x * 0.5f;
		float h2Right = h2Left + hit2.size.x;

		if (h1Left <= h2Right && h1Right >= h2Left) {

			float h1Bottom = hit1.center.y - hit1.size.y * 0.5f;
			float h1Top = h1Bottom + hit1.size.y;
			float h2Bottom = hit2.center.y - hit2.size.y * 0.5f;
			float h2Top = h2Bottom + hit2.size.y;

			if (h1Bottom <= h2Top && h1Top >= h2Bottom) {
				return true;
			}
		}
		return false;
	}

	inline CollisionType getTypeCollisionAABBwithBlock(Hitbox_2D_AABB& hit1, int column, int line, float blockSize) {
		float h1Left = hit1.center.x - hit1.size.x * 0.5f;
		float h1Right = h1Left + hit1.size.x;
		float h2Left = column * blockSize;
		float h2Right = h2Left + blockSize;

		if (h1Left <= h2Right && h1Right >= h2Left) {

			float h1Bottom = hit1.center.y - hit1.size.y * 0.5f;
			float h1Top = h1Bottom + hit1.size.y;
			float h2Bottom = line * blockSize;
			float h2Top = h2Bottom + blockSize;

			if (h1Bottom <= h2Top && h1Top >= h2Bottom) {
				float dx = (h1Left + hit1.size.x * 0.5f) - (h2Left + blockSize * 0.5f);
				float dy = (h1Bottom + hit1.size.y * 0.5f) - (h2Bottom + blockSize * 0.5f);
				dx = (hit1.size.x + blockSize) * 0.5f - std::abs(dx);
				dy = (hit1.size.y + blockSize) * 0.5f - std::abs(dy);

				if (dx > dy) {
					if (h1Top > h2Top)
						return BOTTOM;
					else
						return TOP;
				}
				else if (dx < dy) {
					if (h1Left < h2Left)
						return RIGHT;
					else
						return LEFT;
				}
				else
					return CORNER;
			}
		}
		return NONE;
	}

	//the first hitbox is main and return type is for it
	inline CollisionType getTypeCollisionAABB(Hitbox_2D_AABB& hit1, Hitbox_2D_AABB& hit2) {
		float h1Left = hit1.center.x - hit1.size.x * 0.5f;
		float h1Right = h1Left + hit1.size.x;
		float h2Left = hit2.center.x - hit2.size.x * 0.5f;
		float h2Right = h2Left + hit2.size.x;

		if (h1Left <= h2Right && h1Right >= h2Left) {

			float h1Bottom = hit1.center.y - hit1.size.y * 0.5f;
			float h1Top = h1Bottom + hit1.size.y;
			float h2Bottom = hit2.center.y - hit2.size.y * 0.5f;
			float h2Top = h2Bottom + hit2.size.y;

			if (h1Bottom <= h2Top && h1Top >= h2Bottom) {
				float dx = (h1Left + hit1.size.x * 0.5f) - (h2Left + hit2.size.x * 0.5f);
				float dy = (h1Bottom + hit1.size.y * 0.5f) - (h2Bottom + hit2.size.y * 0.5f);
				dx = (hit1.size.x + hit2.size.x) * 0.5f - std::abs(dx);
				dy = (hit1.size.y + hit2.size.y) * 0.5f - std::abs(dy);

				if (dx > dy) {
					if (h1Top > h2Top)
						return BOTTOM;
					else
						return TOP;
				}
				else if (dx < dy) {
					if (h1Left < h2Left)
						return RIGHT;
					else
						return LEFT;
				}
				else
					return CORNER;
			}
		}
		return NONE;
	}

	inline void projectOnAxis(glm::vec2* vertices, glm::vec2& axis, float& min, float& max) {
		min = max = glm::dot(vertices[0], axis);
		for (int i = 1; i < 4; i++) {
			float projection = glm::dot(vertices[i], axis);
			min = std::min(min, projection);
			max = std::max(max, projection);
		}
	}

	inline bool checkOverlapOnAxis(glm::vec2* verticesA, glm::vec2* verticesB, glm::vec2& axis) {
		float minA, maxA, minB, maxB;
		projectOnAxis(verticesA, axis, minA, maxA);
		projectOnAxis(verticesB, axis, minB, maxB);
		return !(maxA < minB || maxB < minA);
	}

	inline bool checkCollision_AABB_with_OBB(Hitbox_2D_AABB& hit1, Hitbox_2D_OBB& hit2) {
		glm::vec2 vertices1[4], vertices2[4];

		float leftX = hit1.center.x - hit1.size.x * 0.5f;
		float bottomY = hit1.center.y - hit1.size.y * 0.5f;

		vertices1[0] = { leftX, bottomY };
		vertices1[1] = { leftX, bottomY + hit1.size.y };
		vertices1[2] = { leftX + hit1.size.x, bottomY + hit1.size.y };
		vertices1[3] = { leftX + hit1.size.x, bottomY };

		leftX = -hit2.size.x * 0.5f;
		bottomY = -hit2.size.y * 0.5f;

		vertices2[0] = { leftX, bottomY };
		vertices2[1] = { leftX, bottomY + hit2.size.y };  
		vertices2[2] = { leftX + hit2.size.x, bottomY + hit2.size.y };
		vertices2[3] = { leftX + hit2.size.x, bottomY };

		for (int i = 0; i < 4; i++) {
			glm::vec4 vertex = hit2.transformMatrix * glm::vec4(vertices2[i], 0.f, 1.f);
			vertices2[i] = { vertex.x, vertex.y };
		}

		glm::vec2 axes[4];
		axes[0] = glm::vec2(1.f, 0.f); //axisX for AABB
		axes[1] = glm::vec2(0.f, 1.f); //axisY for AABB
		axes[2] = { glm::cos(glm::radians(hit2.angle)), glm::sin(glm::radians(hit2.angle)) }; //axisX for OBB
		axes[3] = { -axes[2].y, axes[2].x }; //axisY for OBB

		for (int i = 0; i < 4; i++) {
			if (!checkOverlapOnAxis(vertices1, vertices2, axes[i]))
				return false;
		}
		return true;
	}

	inline bool checkCollisionOBB(Hitbox_2D_OBB& hit1, Hitbox_2D_OBB& hit2) {
		glm::vec2 vertices1[4], vertices2[4];

		float leftX = -hit1.size.x * 0.5f;
		float bottomY = -hit1.size.y * 0.5f;

		vertices1[0] = { leftX, bottomY };
		vertices1[1] = { leftX, bottomY + hit1.size.y };
		vertices1[2] = { leftX + hit1.size.x, bottomY + hit1.size.y };
		vertices1[3] = { leftX + hit1.size.x, bottomY };

		leftX = -hit2.size.x * 0.5f;
		bottomY = -hit2.size.y * 0.5f;

		vertices2[0] = { leftX, bottomY };
		vertices2[1] = { leftX, bottomY + hit2.size.y };
		vertices2[2] = { leftX + hit2.size.x, bottomY + hit2.size.y };
		vertices2[3] = { leftX + hit2.size.x, bottomY };

		for (int i = 0; i < 4; i++) {
			glm::vec4 vertex = hit1.transformMatrix * glm::vec4(vertices1[i], 0.f, 1.f);
			vertices1[i] = { vertex.x, vertex.y };
		}

		for (int i = 0; i < 4; i++) {
			glm::vec4 vertex = hit2.transformMatrix * glm::vec4(vertices2[i], 0.f, 1.f);
			vertices2[i] = { vertex.x, vertex.y };
		}

		glm::vec2 axes[4];
		axes[0] = { glm::cos(glm::radians(hit1.angle)), glm::sin(glm::radians(hit1.angle)) }; //axisX for first
		axes[1] = { -axes[0].y, axes[0].x }; //axisY for first
		axes[2] = { glm::cos(glm::radians(hit2.angle)), glm::sin(glm::radians(hit2.angle)) }; //axisX for socond
		axes[3] = { -axes[2].y, axes[2].x }; //axisY for second

		for (int i = 0; i < 4; i++) {
			if (!checkOverlapOnAxis(vertices1, vertices2, axes[i]))
				return false;
		}
		return true;
	}
};