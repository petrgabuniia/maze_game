// skeleton.cpp
#include "skeleton.h"
#include "player.h"
#include "maze.h"
#include <cmath>

Skeleton::Skeleton(const sf::Vector2f& pos)
: Monster(pos, 60.f, 2, sf::Color::White) {}

void Skeleton::update(float dt, const Player& p, const Maze& m)
{
    shape_.move({20.f*std::sin(speed_*dt*.1f)*dt, 0.f});
    Monster::update(dt, p, m);
}
