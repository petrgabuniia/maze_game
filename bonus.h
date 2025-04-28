#pragma once
#include "util.hpp"

class Player;

class Bonus
{
public:
    virtual ~Bonus() = default;
    virtual void apply(Player&) = 0;
    virtual int  points() const = 0;

    bool          collected() const { return collected_; }
    sf::FloatRect bounds()    const { return shape_.getGlobalBounds(); }

    void draw(sf::RenderWindow& w) const
    { if (!collected_) { w.draw(shape_); w.draw(label_); } }

protected:
    Bonus(const sf::Vector2f& pos, char ch, sf::Color fill)
      : label_(gFont(), std::string(1,ch), 14)
    {
        shape_.setRadius(12.f);  shape_.setOrigin({12,12});
        shape_.setFillColor(fill);
        shape_.setPosition(pos);

        label_.setFillColor(sf::Color::Black);
        auto bb = label_.getLocalBounds();
        label_.setOrigin({bb.size.x/2.f, bb.size.y/2.f});
        label_.setPosition(shape_.getPosition());
    }

    sf::CircleShape shape_;
    sf::Text        label_;
    bool            collected_{false};
};

class BronzeCoin : public Bonus { public:
    explicit BronzeCoin(const sf::Vector2f& p):Bonus(p,'B',{205,127,50}){}
    void apply(Player&)override{collected_=true;} int points()const override{return 10;}
};
class SilverCoin : public Bonus { public:
    explicit SilverCoin(const sf::Vector2f& p):Bonus(p,'S',{192,192,192}){}
    void apply(Player&)override{collected_=true;} int points()const override{return 25;}
};
class GoldCoin   : public Bonus { public:
    explicit GoldCoin  (const sf::Vector2f& p):Bonus(p,'G',{255,215,0}){}
    void apply(Player&)override{collected_=true;} int points()const override{return 50;}
};
