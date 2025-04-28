#include "monsters.h"
#include "player.h"
#include "maze.h"

Monster::Monster(sf::Vector2f pos, float speed, int hp, int dmg, sf::Color body): shape_(12.f), speed_(speed), hp_(hp), maxHp_(hp), damage_(dmg), alive_(true), respawn_(5.f), baseColor_(body), flash_(0.f), atkCd_(0.f)
{
    shape_.setFillColor(body);
    shape_.setOrigin({12,12});
    shape_.setPosition(pos);
}


void Monster::update(float dt, const Player& player, const Maze& maze)
{
    if (!alive_) { respawn_ -= dt;
        if (respawn_ <= 0) { alive_=true; hp_=maxHp_; shape_.setFillColor(baseColor_); }
        return;
    }
    if (flash_>0) { flash_ -= dt; if (flash_<=0) shape_.setFillColor(baseColor_); }

    sf::Vector2f dir = player.position() - position();
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len>1.f) dir/=len;

    sf::Vector2f step = dir * speed_ * dt;
    sf::Vector2f nxt  = position() + step;
    if (!maze.isWallPixel(nxt.x,nxt.y)) shape_.setPosition(nxt);

    atkCd_ -= dt;
    if (alive() && len < 10.f && atkCd_ <= 0.f) {
        const_cast<Player&>(player).hurt(damage_);
        flash_ = .25f;
        shape_.setFillColor(sf::Color::Magenta);
        atkCd_ = .8f;
    }
}

void Monster::draw(sf::RenderWindow& w) const
{
    if (!alive()) return;
    w.draw(shape_);
    float ratio = float(hp_)/maxHp_;
    sf::RectangleShape back({24,3}), bar({24*ratio,3});
    back.setFillColor(sf::Color::Black);
    bar.setFillColor(sf::Color::Green);
    back.setOrigin({12,14}); bar.setOrigin({12,14});
    back.setPosition(shape_.getPosition());
    bar.setPosition(shape_.getPosition());
    w.draw(back); w.draw(bar);
}
