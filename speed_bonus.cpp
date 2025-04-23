// speed_bonus.cpp
#include "speed_bonus.h"
#include "player.h"

void SpeedBonus::apply(Player& p)
{
    if (collected_) return;
    collected_ = true;
    shape_.setFillColor(sf::Color::Transparent);
    p.boostSpeed(1.5f);
}
