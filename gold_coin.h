#pragma once
#include "bonus.h"

class GoldCoin : public Bonus
{
public:
    explicit GoldCoin(const sf::Vector2f& pos)
        : Bonus(pos)
    {
        shape_.setFillColor({255,215,  0});    // gold
    }
    void apply(Player&) override { collected_ = true; }
    int  points() const override { return 50; }
};
