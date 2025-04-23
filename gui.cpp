#include <SFML/Graphics.hpp>
#include <optional>
#include "game.h"

constexpr int cellsW=25,cellsH=18,cell=32;

static sf::Font loadFont(){
    sf::Font f; f.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
    return f;
}

Difficulty showMenu(sf::RenderWindow& win)
{
    const sf::Font font=loadFont();

    sf::Text title(font);  title.setString("Choose difficulty"); title.setCharacterSize(36);
    sf::Text easy (font), med(font), hard(font);
    easy.setString("1  -  Easy");   med.setString("2  -  Medium"); hard.setString("3  -  Hard");
    easy .setCharacterSize(28);
    med  .setCharacterSize(28);
    hard .setCharacterSize(28);

    title.setPosition({60,40});
    easy .setPosition({80,120});
    med  .setPosition({80,170});
    hard .setPosition({80,220});

    while(win.isOpen()){
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>())win.close();
            if(ev->is<sf::Event::KeyPressed>()){
                auto k=ev->getIf<sf::Event::KeyPressed>()->code;
                if(k==sf::Keyboard::Key::Num1)return Difficulty::Easy;
                if(k==sf::Keyboard::Key::Num2)return Difficulty::Medium;
                if(k==sf::Keyboard::Key::Num3)return Difficulty::Hard;
            }
            if(ev->is<sf::Event::MouseButtonPressed>()){
                sf::Vector2f m=static_cast<sf::Vector2f>(
                   ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(easy.getGlobalBounds().contains(m)) return Difficulty::Easy;
                if(med .getGlobalBounds().contains(m)) return Difficulty::Medium;
                if(hard.getGlobalBounds().contains(m)) return Difficulty::Hard;
            }
        }
        win.clear(sf::Color(30,30,30));
        win.draw(title);win.draw(easy);win.draw(med);win.draw(hard);
        win.display();
    }
    return Difficulty::Medium;
}

int main()
{
    sf::RenderWindow win({sf::VideoMode({unsigned(cellsW*cell),unsigned(cellsH*cell)}),
                          "Maze Game", sf::State::Windowed});
    win.setVerticalSyncEnabled(true);

    Difficulty diff=showMenu(win);
    if(!win.isOpen())return 0;

    Game game(cellsW,cellsH,diff);
    sf::Clock clock;

    while(win.isOpen()){
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>())win.close();
            if(ev->is<sf::Event::KeyPressed>()&&
               ev->getIf<sf::Event::KeyPressed>()->code==sf::Keyboard::Key::Escape)win.close();
        }
        float dt=clock.restart().asSeconds();
        game.update(dt);
        win.clear(sf::Color::Black);
        game.draw(win);
        win.display();
    }
    return 0;
}
