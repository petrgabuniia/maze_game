#include "game.h"
#include "zombie.h"
#include "skeleton.h"
#include "bronze_coin.h"
#include "silver_coin.h"
#include "gold_coin.h"
#include <SFML/Window/Keyboard.hpp>

// ------------------------------------------------------------------
Game::Game(unsigned mw, unsigned mh, Difficulty diff)
: maze_(mw, mh, cell_)
, player_(maze_.entrance())
, winText_{font_}
, scoreText_{font_}
{
    font_.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");

    // Win banner
    winText_.setString("You escaped!");
    winText_.setCharacterSize(48);
    winText_.setFillColor(sf::Color::Green);
    winText_.setPosition({50.f, 50.f});

    // Scoreboard
    scoreText_.setCharacterSize(24);
    scoreText_.setFillColor(sf::Color::White);
    scoreText_.setString("Score: 0");

    std::mt19937 rng(std::random_device{}());

    for (unsigned i = 0; i < static_cast<unsigned>(diff); ++i)
        monsters_.push_back(spawnMonster(rng));

    for (unsigned i = 0; i < 25; ++i)          // 25 random coins
        coins_.push_back(spawnCoin(rng));
}

Game::~Game() = default;

std::unique_ptr<Monster> Game::spawnMonster(std::mt19937& rng)
{
    std::uniform_real_distribution<float> fx(cell_, maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_, maze_.exitPos().y);
    sf::Vector2f pos{fx(rng), fy(rng)};

    if (rng() & 1)                       // ← same behaviour, no type clash
        return std::make_unique<Zombie>(pos);
    else
        return std::make_unique<Skeleton>(pos);
}


std::unique_ptr<Bonus> Game::spawnCoin(std::mt19937& rng)
{
    std::uniform_real_distribution<float> fx(cell_, maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_, maze_.exitPos().y);
    sf::Vector2f p{fx(rng), fy(rng)};

    switch (rng() % 3)
    {
        case 0: return std::make_unique<BronzeCoin>(p);
        case 1: return std::make_unique<SilverCoin>(p);
        default: return std::make_unique<GoldCoin>(p);
    }
}

// ------------------------- update ----------------------------------
void Game::update(float dt)
{
    if (won_) return;

    player_.update(dt, maze_);
    for (auto& m : monsters_) m->update(dt, player_, maze_);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        for (auto& m : monsters_) if (m->alive()) player_.tryHit(*m);

    // coin pickup
    for (auto& c : coins_)
        if (!c->collected() &&
            player_.bounds().findIntersection(c->bounds()))
        {
            c->apply(player_);
            score_ += c->points();
            scoreText_.setString("Score: " + std::to_string(score_));
        }

    // exit?
    if (maze_.exitRect().contains(player_.position()))
        won_ = true;
}

// --------------------------- draw ----------------------------------
void Game::draw(sf::RenderWindow& w) const
{
    maze_.draw(w);
    for (const auto& c : coins_) c->draw(w);
    w.draw(player_.shape());
    for (const auto& m : monsters_) if (m->alive()) w.draw(m->shape());

    /// inside Game::draw()
    auto bounds = scoreText_.getLocalBounds();
    scoreText_.setPosition(
        {w.getSize().x - bounds.size.x - 20.f,   // bounds.size.x is SFML-3
        10.f});
    w.draw(scoreText_);



    if (won_) w.draw(winText_);
}
