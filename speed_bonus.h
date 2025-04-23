#pragma once
#include "bonus.h"
#include "player.h"

class SpeedBonus : public Bonus
{
public:
    explicit SpeedBonus(const sf::Vector2f& pos)
        : Bonus(pos)                     // base sets radius/origin/pos
    {
        shape_.setFillColor(sf::Color::Yellow);
    }

    void apply(Player&) override;
};
