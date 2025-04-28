#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>
#include "maze.h"
#include "player.h"
#include "bonus.h"
#include "monsters.h"

enum class Difficulty { Easy, Medium, Hard, Custom };

struct Params {
    int   playerHp;
    int   playerAtk;
    float playerSpeed;

    unsigned zombies;
    int      zHp;
    int      zDmg;
    float    zSpeed;

    unsigned skeletons;
    int      sHp;
    int      sDmg;
    float    sSpeed;
};

class Monster;
class Game {
public:
    Game(unsigned w, unsigned h, const Params& p);

    void update(float dt);
    void draw(sf::RenderWindow&) const;
    bool finished() const { return done_; }

private:
    std::unique_ptr<Monster> spawnRandomMonster(std::mt19937&);
    std::unique_ptr<Monster> makeZombie(std::mt19937& r, const Params& p);
    std::unique_ptr<Monster> makeSkeleton(std::mt19937& r, const Params& p);
    std::unique_ptr<Bonus>   spawnCoin(std::mt19937& r);

    static constexpr unsigned cell_ = 32;

    Maze    maze_;
    Player  player_;
    std::vector<std::unique_ptr<Monster>> monsters_;
    std::vector<std::unique_ptr<Bonus>>   coins_;
    Params  params_;

    // UI
    sf::Font  font_;
    sf::Text  banner_;
    mutable sf::Text scoreTxt_;
    int   score_{0};
    bool  done_{false};
};
