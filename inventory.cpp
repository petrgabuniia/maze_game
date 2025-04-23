#include "inventory.h"

Inventory::Inventory()
{
    weapons_.push_back({"Fists",1});
    weapons_.push_back({"Sword",3});
}

void Inventory::handleKey(sf::Keyboard::Key k)
{
    using K = sf::Keyboard::Key;
    if (k>=K::Num1 && k<=K::Num9)
    {
        unsigned idx = static_cast<unsigned>(k)-static_cast<unsigned>(K::Num1);
        if (idx < weapons_.size()) current_=idx;
    }
}
void Inventory::draw(sf::RenderWindow& w) const
{
    if (!open_) return;
    sf::RectangleShape box({200.f,100.f});
    box.setPosition({10.f,10.f});              // Vector2f overload is valid
    box.setFillColor(sf::Color(0,0,0,180));
    w.draw(box);
}

