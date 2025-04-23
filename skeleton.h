// skeleton.h
#pragma once
#include "monsters.h"

class Skeleton : public Monster
{
public:
    Skeleton(const sf::Vector2f& pos, int hp, int dmg, float spd);
    void update(float dt,const Player&p,const Maze&m) override;
};
