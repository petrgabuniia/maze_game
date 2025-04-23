#pragma once
#include "bonus.h"
#include "player.h"

class AttackBonus : public Bonus
{
public:
    explicit AttackBonus(const sf::Vector2f& pos)
        : Bonus(pos)
    {
        shape_.setFillColor(sf::Color::Blue);
    }

    void apply(Player&) override;
};
