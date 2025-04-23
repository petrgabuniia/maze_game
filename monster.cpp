#include "monsters.h"
#include "player.h"
#include "maze.h"
#include <cmath>

Monster::Monster(const sf::Vector2f& pos,
                 float spd, int hp, int dmg, sf::Color c)
: baseColor_(c), speed_(spd), hp_(hp), maxHp_(hp), damage_(dmg)
{
    shape_.setRadius(14.f);
    shape_.setOrigin({14.f,14.f});
    shape_.setFillColor(baseColor_);
    shape_.setPosition(pos);
}

void Monster::hurt(int dmg)
{
    if (!alive_) return;
    hp_ -= dmg;
    if(hp_<0) hp_=0;
    flash_ = .25f;
    shape_.setFillColor(sf::Color::Yellow);
    if (hp_ <= 0) { alive_ = false; respawn_ = 5.f; }
}

void Monster::update(float dt, const Player& p, const Maze& maze)
{
    if (!alive_) { respawn_ -= dt;
        if (respawn_ <= 0) { alive_=true; hp_=maxHp_; shape_.setFillColor(baseColor_); }
        return;
    }
    if (flash_>0) { flash_ -= dt; if (flash_<=0) shape_.setFillColor(baseColor_); }

    sf::Vector2f dir = p.position() - position();
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len>1.f) dir/=len;

    sf::Vector2f step = dir * speed_ * dt;
    sf::Vector2f nxt  = position() + step;
    if (!maze.isWallPixel(nxt.x,nxt.y)) shape_.setPosition(nxt);

    atkCd_ -= dt;
    if (len < 28.f && atkCd_ <= 0.f) {
        const_cast<Player&>(p).hurt(damage_);
        flash_ = .25f;
        shape_.setFillColor(sf::Color::Magenta);
        atkCd_ = .8f;
    }
}
