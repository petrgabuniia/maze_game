#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>

#include "maze.h"
#include "player.h"
#include "bonus.h"

enum class Difficulty{Easy,Medium,Hard};

struct DiffParams{
    unsigned monsters;
    int  monsterHp;
    int  monsterDmg;
    int  playerAtk;
};

class Monster;
class Game
{
public:
    Game(unsigned w,unsigned h,Difficulty d=Difficulty::Medium);
    ~Game();

    void update(float dt);
    void draw(sf::RenderWindow&) const;

private:
    std::unique_ptr<Monster> spawnMonster(std::mt19937&);
    std::unique_ptr<Bonus>   spawnCoin   (std::mt19937&);

    static constexpr unsigned cell_=32;
    static const DiffParams   DIFF[3];

    Difficulty                       diff_;
    DiffParams                       params_;
    Maze                             maze_;
    Player                           player_;
    std::vector<std::unique_ptr<Monster>> monsters_;
    std::vector<std::unique_ptr<Bonus>>   coins_;

    sf::Font  font_;
    sf::Text  winText_;
    mutable sf::Text  scoreText_;
    int       score_{0};
    bool      won_{false};
};
