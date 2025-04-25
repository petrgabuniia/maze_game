#pragma once
#include "bonus.h"

class BronzeCoin : public Bonus
{
public:
    explicit BronzeCoin(const sf::Vector2f& pos)
        : Bonus(pos, 'B', {205,127,50}) {}

    void apply(Player&) override { collected_ = true; }
    int  points()       const override { return 10; }
};
