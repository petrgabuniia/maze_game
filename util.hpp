#pragma once
#include <SFML/Graphics.hpp>

inline const sf::Font& gFont()
{
    static const sf::Font* fp = []{
        auto* f = new sf::Font;
        (void)f->openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
        return f;
    }();
    return *fp;
}

inline void centre(sf::Text& t, const sf::RectangleShape& box)
{
    auto r = t.getLocalBounds();
    t.setOrigin({r.size.x/2.f, r.size.y/2.f});
    t.setPosition(box.getPosition() + box.getSize()/2.f);
}
