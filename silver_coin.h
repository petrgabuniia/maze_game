#pragma once
#include "bonus.h"

class SilverCoin : public Bonus
{
public:
    explicit SilverCoin(const sf::Vector2f& pos)
        : Bonus(pos, 'S', {192,192,192}) {}

    void apply(Player&) override { collected_ = true; }
    int  points()       const override { return 25; }
};
