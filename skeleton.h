// skeleton.h
#pragma once
#include "monsters.h"

class Skeleton : public Monster
{
public:
    Skeleton(const sf::Vector2f& pos, int hp, int dmg);
    void update(float dt, const Player&, const Maze&) override;
};
