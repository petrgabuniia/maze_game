#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

struct Weapon
{
    std::string name;
    int         damage;
};

// ----------------------------------------------------------------------
class Inventory
{
public:
    Inventory();

    void toggle()                 { open_ = !open_; }
    bool isOpen()           const { return open_;   }

    void handleKey(sf::Keyboard::Key);
    void draw(sf::RenderWindow&) const;

    const Weapon& current() const { return weapons_[current_]; }

private:
    std::vector<Weapon> weapons_;
    unsigned            current_{0};
    bool                open_{false};
};
