#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>

#include "maze.h"
#include "player.h"
#include "bonus.h"

class Monster;

enum class Difficulty { Easy = 3, Normal = 6, Hard = 10 };

class Game
{
public:
    Game(unsigned mazeW, unsigned mazeH, Difficulty d = Difficulty::Normal);
    ~Game();

    void update(float dt);
    void draw(sf::RenderWindow&) const;

private:
    std::unique_ptr<Monster> spawnMonster(std::mt19937&);
    std::unique_ptr<Bonus>   spawnCoin   (std::mt19937&);

    static constexpr unsigned cell_ = 32;

    Maze                                   maze_;
    Player                                 player_;
    std::vector<std::unique_ptr<Monster>>  monsters_;
    std::vector<std::unique_ptr<Bonus>>    coins_;

    // UI
    sf::Font  font_;
    sf::Text  winText_;
    mutable sf::Text  scoreText_;
    int       score_{0};
    bool      won_{false};
};
