// zombie.cpp
#include "zombie.h"

Zombie::Zombie(const sf::Vector2f& pos, int hp, int dmg)
: Monster(pos, 80.f, hp, dmg, {100,255,100}) {}
