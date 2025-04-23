#pragma once
#include <SFML/Graphics.hpp>

class Player;
class Maze;

// ---------------------------------------------------------------
class Monster
{
public:
    Monster(const sf::Vector2f& pos, float speed, int hp,
            sf::Color base);

    virtual void update(float dt, const Player&, const Maze&);
    virtual void attack(Player&) {}                // simple melee stub
    void         hurt(int dmg);

    bool  alive() const { return alive_; }

    sf::Vector2f           position() const { return shape_.getPosition(); }
    const sf::CircleShape& shape()    const { return shape_; }

protected:
    sf::CircleShape shape_;
    sf::Color       baseColor_;
    float           speed_;
    int             hp_, maxHp_;
    bool            alive_{true};

    float           respawn_{0.f};
    float           flash_{0.f};
    float           atkCd_{0.f};
};
