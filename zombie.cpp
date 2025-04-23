#include "zombie.h"

Zombie::Zombie(const sf::Vector2f& pos,int hp,int dmg,float spd)
: Monster(pos, spd, hp, dmg, {100,255,100}) {}

