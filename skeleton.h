// skeleton.h
#pragma once
#include "monsters.h"

class Skeleton : public Monster
{
public:
    explicit Skeleton(const sf::Vector2f& pos);
    void update(float dt, const Player&, const Maze&) override;
};
