#pragma once

class Object;
struct Player;

enum CollisionType { Left, Right, Top, Bottom };

class Phycics {
public:
	CollisionType check_collision_type(Object* object_p, Player& player);
private:
	
};