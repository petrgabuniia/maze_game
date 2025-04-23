#pragma once
#include "bonus.h"

class SilverCoin : public Bonus
{
public:
    explicit SilverCoin(const sf::Vector2f& pos)
        : Bonus(pos)
    {
        shape_.setFillColor({192,192,192});    // silver
    }
    void apply(Player&) override { collected_ = true; }
    int  points() const override { return 25; }
};
