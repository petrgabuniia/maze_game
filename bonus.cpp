#include "bonus.h"

Bonus::Bonus(const sf::Vector2f& pos)
{
    shape_.setRadius(10.f);
    shape_.setOrigin({10.f,10.f});
    shape_.setPosition(pos);
}
Bonus::~Bonus() = default;
