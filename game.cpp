#include "game.h"
#include "zombie.h"
#include "skeleton.h"
#include "speed_bonus.h"
#include "attack_bonus.h"
#include <SFML/Window/Keyboard.hpp>

// -----------------------------------------------------------------------------
// ctor
Game::Game(unsigned mw, unsigned mh, Difficulty diff)
: maze_(mw, mh, cell_)                 // perfect maze
, player_(maze_.entrance())            // spawn at (1,1)
, winText_{font_}                      // sf::Text needs a Font ref
{
    // load a font that sits next to the executable
    font_.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");


    winText_.setString("You escaped!");
    winText_.setCharacterSize(48);
    winText_.setFillColor(sf::Color::Green);
    winText_.setPosition({50.f, 50.f});

    // --------------------------------- RNG helpers
    std::mt19937 rng(std::random_device{}());

    for (unsigned i = 0; i < static_cast<unsigned>(diff); ++i)
        monsters_.push_back(spawnMonster(rng));

    for (unsigned i = 0; i < 5; ++i)
        bonuses_.push_back(spawnBonus(rng));
}

// -----------------------------------------------------------------------------
Game::~Game() = default;

// -----------------------------------------------------------------------------
std::unique_ptr<Monster> Game::spawnMonster(std::mt19937& rng)
{
    std::uniform_real_distribution<float> fx(cell_, maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_, maze_.exitPos().y);
    sf::Vector2f pos{fx(rng), fy(rng)};

    if (rng() & 1)
        return std::make_unique<Zombie>(pos);
    else
        return std::make_unique<Skeleton>(pos);
}

// -----------------------------------------------------------------------------
std::unique_ptr<Bonus> Game::spawnBonus(std::mt19937& rng)
{
    std::uniform_real_distribution<float> fx(cell_, maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_, maze_.exitPos().y);
    sf::Vector2f pos{fx(rng), fy(rng)};

    if (rng() & 1)
        return std::make_unique<SpeedBonus>(pos);
    else
        return std::make_unique<AttackBonus>(pos);
}

// -----------------------------------------------------------------------------
// main game step
void Game::update(float dt)
{
    if (won_) return;

    player_.update(dt, maze_);

    for (auto& m : monsters_) m->update(dt, player_, maze_);

    // melee (Space)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        for (auto& m : monsters_) if (m->alive()) player_.tryHit(*m);

    // bonuses
    for (auto& b : bonuses_)
        if (!b->collected() &&
            player_.bounds().findIntersection(b->bounds()))
            b->apply(player_);

    // reached exit?
    if (maze_.exitRect().contains(player_.position()))
        won_ = true;
}

// -----------------------------------------------------------------------------
// render everything
void Game::draw(sf::RenderWindow& w) const
{
    maze_.draw(w);

    for (const auto& b : bonuses_) b->draw(w);

    w.draw(player_.shape());

    for (const auto& m : monsters_)
        if (m->alive()) w.draw(m->shape());

    inv_.draw(w);

    if (won_) w.draw(winText_);
}
