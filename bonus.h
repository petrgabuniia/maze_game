#pragma once
#include <SFML/Graphics.hpp>

class Player;

inline const sf::Font& coinFont()
{
    static const sf::Font* f = []{
        auto* fp = new sf::Font;
        fp->openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
        return fp;               
    }();
    return *f;
}

class Bonus
{
public:
    virtual ~Bonus() = default;

    virtual void apply(Player&) = 0;   
    virtual int  points() const = 0;   

    bool          collected() const { return collected_; }
    sf::FloatRect bounds()    const { return shape_.getGlobalBounds(); }

    void draw(sf::RenderWindow& w) const
    {
        if (!collected_) { w.draw(shape_); w.draw(label_); }
    }

protected:
    Bonus(const sf::Vector2f& pos, char letter, sf::Color fill)
        : label_(coinFont(), std::string(1, letter), 14)  
    {
        shape_.setRadius(12.f);
        shape_.setOrigin({12.f, 12.f});
        shape_.setFillColor(fill);
        shape_.setPosition(pos);

        label_.setFillColor(sf::Color::Black);
        auto r = label_.getLocalBounds();
        label_.setOrigin({ r.size.x / 2.f, r.size.y / 2.f + 3});
        label_.setPosition(pos);
    }

    sf::CircleShape shape_;
    sf::Text        label_;
    bool            collected_{false};
};
