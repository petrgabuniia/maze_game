#pragma once
#include "bonus.h"

class GoldCoin : public Bonus
{
public:
    explicit GoldCoin(const sf::Vector2f& pos)
        : Bonus(pos, 'G', {255,215,0}) {}

    void apply(Player&) override { collected_ = true; }
    int  points()       const override { return 50; }
};
