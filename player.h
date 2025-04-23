#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

class Maze;
class Monster;

inline float dist(sf::Vector2f a, sf::Vector2f b)
{
    float dx=a.x-b.x, dy=a.y-b.y;
    return std::sqrt(dx*dx+dy*dy);
}

// --------------------------------------------------------------
class Player
{
public:
    explicit Player(const sf::Vector2f& pos);

    void update(float dt, const Maze&);
    bool tryHit(Monster&);

    void boostSpeed(float m)  { speed_  *= m; }
    void boostAttack(int n)   { attack_ += n; }
    void hurt(int);           // gets red flash

    sf::Vector2f           position() const { return pos_; }
    sf::FloatRect          bounds()   const { return shape_.getGlobalBounds(); }
    const sf::CircleShape& shape()    const { return shape_; }

private:
    sf::CircleShape shape_;
    sf::Vector2f    pos_;
    float           speed_{120.f};
    int             attack_{1};
    float           flash_{0.f};
};
