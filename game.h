#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>
#include "maze.h"
#include "player.h"
#include "inventory.h"
#include "bonus.h"

class Monster;          // fwd
enum class Difficulty { Easy=3, Normal=6, Hard=10 };

class Game
{
public:
    Game(unsigned mw,unsigned mh, Difficulty d=Difficulty::Normal);
    ~Game();

    void update(float dt);
    void draw(sf::RenderWindow&) const;

    Inventory& inventory()       { return inv_; }

private:
    std::unique_ptr<Monster> spawnMonster(std::mt19937&);
    std::unique_ptr<Bonus>   spawnBonus(std::mt19937&);

    static constexpr unsigned cell_ = 32;

    Maze                                   maze_;
    Player                                 player_;
    Inventory                              inv_;
    std::vector<std::unique_ptr<Monster>>  monsters_;
    std::vector<std::unique_ptr<Bonus>>    bonuses_;

    sf::Font  font_;
    sf::Text  winText_;
    bool      won_{false};
};
