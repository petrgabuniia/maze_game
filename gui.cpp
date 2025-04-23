// gui.cpp -------------------------------------------------------------
#include <SFML/Graphics.hpp>
#include <optional>
#include "game.h"

constexpr int cellsW = 25;
constexpr int cellsH = 18;
constexpr int cell   = 32;

// ---------------------------------------------------------------------
// tiny font helper; swap the path for any TTF you like
static sf::Font loadFont()
{
    sf::Font f;
    f.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
    return f;
}

// ---------------------------------------------------------------------
// simple difficulty picker (keyboard 1/2/3 or mouse click)
Difficulty showMenu(sf::RenderWindow& win)
{
    const sf::Font font = loadFont();

    sf::Text title(font);  title.setString("Choose difficulty");
    title.setCharacterSize(36);
    sf::Text easy(font), med(font), hard(font);
    easy .setString("1  -  Easy");
    med  .setString("2  -  Medium");
    hard .setString("3  -  Hard");
    easy .setCharacterSize(28);
    med  .setCharacterSize(28);
    hard .setCharacterSize(28);

    title.setPosition({60.f, 40.f});
    easy .setPosition({80.f,120.f});
    med  .setPosition({80.f,170.f});
    hard .setPosition({80.f,220.f});

    while (win.isOpen())
    {
        while (auto ev = win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();

            if (ev->is<sf::Event::KeyPressed>())
            {
                auto k = ev->getIf<sf::Event::KeyPressed>()->code;
                if (k == sf::Keyboard::Key::Num1) return Difficulty::Easy;
                if (k == sf::Keyboard::Key::Num2) return Difficulty::Medium;
                if (k == sf::Keyboard::Key::Num3) return Difficulty::Hard;
                if (k == sf::Keyboard::Key::Escape) win.close();
            }

            if (ev->is<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                    ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if (easy.getGlobalBounds().contains(m))  return Difficulty::Easy;
                if (med .getGlobalBounds().contains(m))  return Difficulty::Medium;
                if (hard.getGlobalBounds().contains(m)) return Difficulty::Hard;
            }
        }

        win.clear(sf::Color(30,30,30));
        win.draw(title); win.draw(easy); win.draw(med); win.draw(hard);
        win.display();
    }
    return Difficulty::Medium;   // window closed
}

// ---------------------------------------------------------------------
// main
int main()
{
    sf::RenderWindow win({sf::VideoMode({unsigned(cellsW*cell),
                                         unsigned(cellsH*cell)}),
                          "Maze Game",
                          sf::State::Windowed});
    win.setVerticalSyncEnabled(true);

restart_game:                      // label for easy restart

    Difficulty diff = showMenu(win);
    if (!win.isOpen()) return 0;   // user closed during menu

    Game game(cellsW, cellsH, diff);
    sf::Clock clock;

    while (win.isOpen())
    {
        while (auto ev = win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();

            if (ev->is<sf::Event::KeyPressed>())
            {
                auto k = ev->getIf<sf::Event::KeyPressed>()->code;

                if (k == sf::Keyboard::Key::Escape) win.close();

                // ---- press R to restart after win OR death -------------
                if (k == sf::Keyboard::Key::R && game.finished())
                {
                    goto restart_game;        // jump back to menu
                }
            }
        }

        float dt = clock.restart().asSeconds();
        game.update(dt);

        win.clear(sf::Color::Black);
        game.draw(win);
        win.display();
    }
    return 0;
}
