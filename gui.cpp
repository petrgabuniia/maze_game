#include <SFML/Graphics.hpp>
#include <optional>
#include "game.h"

constexpr int cellsW=25,cellsH=18,cell=32;

int main()
{
    sf::RenderWindow win({sf::VideoMode({unsigned(cellsW*cell),
                                         unsigned(cellsH*cell)}),
                           "Maze Game",
                           sf::State::Windowed});
    win.setVerticalSyncEnabled(true);

    Game game(cellsW,cellsH);
    sf::Clock clk;

    while(win.isOpen())
    {
        while(auto ev=win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();

            if (ev->is<sf::Event::KeyPressed>())
            {
                auto key=ev->getIf<sf::Event::KeyPressed>()->code;
                if (key==sf::Keyboard::Key::Escape) win.close();
                else if (key==sf::Keyboard::Key::Tab)
                    game.inventory().toggle();
                else if (game.inventory().isOpen())
                    game.inventory().handleKey(key);
            }
        }

        float dt=clk.restart().asSeconds();
        game.update(dt);

        win.clear(sf::Color::Black);
        game.draw(win);
        win.display();
    }
    return 0;
}
