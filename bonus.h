#pragma once
#include <SFML/Graphics.hpp>

class Player;

class Bonus
{
public:
    explicit Bonus(const sf::Vector2f& pos);
    virtual ~Bonus();                          // v-table anchor

    virtual void apply(Player&) = 0;

    bool          collected() const { return collected_; }
    sf::FloatRect bounds()    const { return shape_.getGlobalBounds(); }

    void draw(sf::RenderWindow& w) const
    {
        if (!collected_) w.draw(shape_);
    }

protected:
    sf::CircleShape shape_;
    bool            collected_{false};
};
