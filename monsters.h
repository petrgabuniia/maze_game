#pragma once
#include <SFML/Graphics.hpp>

class Player;
class Maze;

class Monster {
public:
    Monster(sf::Vector2f pos, float speed, int hp, int dmg, sf::Color body);
    virtual ~Monster() = default;

    virtual void update(float dt, const Player& player, const Maze&);
    virtual void draw(sf::RenderWindow& w) const;

    void  hurt(int n)           { hp_ = std::max(0, hp_ - n); }
    bool  alive()    const      { return hp_ > 0; }
    int   hp()       const      { return hp_; }
    int   maxHp()    const      { return maxHp_; }
    int   damage()   const      { return damage_; }
    sf::Vector2f position() const { return shape_.getPosition(); }
    const sf::CircleShape& shape()    const { return shape_; }
    sf::FloatRect bounds()  const { return shape_.getGlobalBounds(); }

protected:
    sf::CircleShape shape_;
    float           speed_;
    int             hp_, maxHp_;
    int             damage_;
    bool           alive_{true};
    float           respawn_{5.f};
    sf::Color       baseColor_;
    float           flash_{0.f};
    float           atkCd_{0.f};
};


class Zombie : public Monster {
public:
    Zombie(sf::Vector2f pos, int hp, int dmg, float speed)
    : Monster(pos, speed, hp, dmg, sf::Color(100,255,100))
    {}
};

class Skeleton : public Monster {
public:
    Skeleton(sf::Vector2f pos, int hp, int dmg, float speed)
    : Monster(pos, speed, hp, dmg, sf::Color::White)
    {}
};
