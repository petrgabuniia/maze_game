// zombie.h
#pragma once
#include "monsters.h"

class Zombie : public Monster
{
public:
    explicit Zombie(const sf::Vector2f& pos);
    void update(float dt, const Player& p, const Maze& m) override
    { Monster::update(dt, p, m); }
};
