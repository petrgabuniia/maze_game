#pragma once
#include <SFML/Graphics.hpp>

class Player;
class Maze;

class Monster
{
public:
    Monster(const sf::Vector2f& pos,
            float               speed,
            int                 hp,
            int                 damage,
            sf::Color           base);

    virtual void update(float dt, const Player&, const Maze&);
    void         hurt(int dmg);
    bool         alive() const { return alive_; }

    sf::Vector2f           position() const { return shape_.getPosition(); }
    const sf::CircleShape& shape()    const { return shape_; }
    int  hp()      const { return hp_;  }
    int  maxHp()   const { return maxHp_; }

protected:
    sf::CircleShape shape_;
    sf::Color       baseColor_;
    float           speed_;
    int             hp_, maxHp_;
    int             damage_;          // ← how much this monster hurts player
    bool            alive_{true};
    float           respawn_{0.f};
    float           flash_{0.f};
    float           atkCd_{0.f};
};
