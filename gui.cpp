#include <SFML/Graphics.hpp>
#include <optional>
#include "game.h"

constexpr int cellsW = 25;
constexpr int cellsH = 18;
constexpr int cell   = 32;

int main()
{
    sf::RenderWindow win({sf::VideoMode({unsigned(cellsW*cell),
                                         unsigned(cellsH*cell)}),
                           "Maze Game",
                           sf::State::Windowed});
    win.setVerticalSyncEnabled(true);

    Game game(cellsW, cellsH);
    sf::Clock clock;

    while (win.isOpen())
    {
        while (auto ev = win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();
            if (ev->is<sf::Event::KeyPressed>() &&
                ev->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)
                win.close();
        }

        float dt = clock.restart().asSeconds();
        game.update(dt);

        win.clear(sf::Color::Black);
        game.draw(win);
        win.display();
    }
    return 0;
}
