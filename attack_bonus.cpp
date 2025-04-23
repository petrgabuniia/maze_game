// attack_bonus.cpp
#include "attack_bonus.h"
#include "player.h"

void AttackBonus::apply(Player& p)
{
    if (collected_) return;
    collected_ = true;
    shape_.setFillColor(sf::Color::Transparent);
    p.boostAttack(2);
}
