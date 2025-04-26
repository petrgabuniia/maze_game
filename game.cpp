#include "game.h"
#include "zombie.h"
#include "skeleton.h"
#include "bronze_coin.h"
#include "silver_coin.h"
#include "gold_coin.h"
#include <SFML/Window/Keyboard.hpp>

Game::Game(unsigned w, unsigned h, const Params& p)
: maze_(w,h,cell_)
, player_(maze_.entrance())
, params_(p)
, banner_{font_}
, scoreTxt_{font_}
{
    (void)font_.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");

    banner_.setCharacterSize(48);
    banner_.setFillColor(sf::Color::Green);
    banner_.setPosition({50,50});
    banner_.setString("You escaped!");

    scoreTxt_.setCharacterSize(24);
    scoreTxt_.setFillColor(sf::Color::White);
    scoreTxt_.setString("Score: 0");

    player_.setAttack(params_.playerAtk);
    player_.hurt(0);       
    player_.heal(params_.playerHp); 
    player_.heal(params_.playerHp);

    std::mt19937 rng(std::random_device{}());
    for(unsigned i=0;i<params_.zombies;   ++i) monsters_.push_back(makeZombie  (rng,p));
    for(unsigned i=0;i<params_.skeletons; ++i) monsters_.push_back(makeSkeleton(rng,p));
    for(unsigned i=0;i<25;++i) coins_.push_back(spawnCoin(rng));
}


std::unique_ptr<Monster> Game::makeZombie(std::mt19937& r,const Params& p)
{
    std::uniform_real_distribution<float> fx(cell_,maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_,maze_.exitPos().y);
    return std::make_unique<Zombie>(sf::Vector2f{fx(r),fy(r)},p.zHp,p.zDmg,p.zSpeed);
}
std::unique_ptr<Monster> Game::makeSkeleton(std::mt19937& r,const Params& p)
{
    std::uniform_real_distribution<float> fx(cell_,maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_,maze_.exitPos().y);
    return std::make_unique<Skeleton>(sf::Vector2f{fx(r),fy(r)},p.sHp,p.sDmg,p.sSpeed);
}
std::unique_ptr<Bonus> Game::spawnCoin(std::mt19937& r)
{
    std::uniform_real_distribution<float> fx(cell_, maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_, maze_.exitPos().y);
    sf::Vector2f p{fx(r), fy(r)};
    switch (r() % 3)
    {
        case 0:  return std::make_unique<BronzeCoin>(p);
        case 1:  return std::make_unique<SilverCoin>(p);
        default: return std::make_unique<GoldCoin>(p);
    }
}


void Game::update(float dt)
{
    if (done_) return;

    player_.update(dt, maze_);
    for (auto& m : monsters_) m->update(dt, player_, maze_);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        for (auto& m : monsters_) if (m->alive()) player_.tryHit(*m);

    for (auto& c : coins_)
        if (!c->collected() &&
            player_.bounds().findIntersection(c->bounds()))
        {
            c->apply(player_);
            score_ += c->points();
            scoreTxt_.setString("Score: " + std::to_string(score_));
        }

    if (player_.dead())
    {
        banner_.setString("You died!");
        banner_.setFillColor(sf::Color::Red);
        done_ = true;
    }
    if (maze_.exitRect().contains(player_.position()))
        done_ = true;
}

void Game::draw(sf::RenderWindow& w) const
{
    maze_.draw(w);

    for (const auto& c : coins_)   c->draw(w);
    w.draw(player_.shape());
    for (const auto& m : monsters_) if (m->alive()) w.draw(m->shape());

    auto bar = [&](sf::Vector2f pos, float ratio)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        constexpr float W = 30.f, H = 4.f;
        sf::RectangleShape back({W, H});
        back.setFillColor(sf::Color(50,50,50));
        back.setPosition(pos);
        w.draw(back);
        if (ratio > 0.f)
        {
            sf::RectangleShape fill({W * ratio, H});
            fill.setFillColor(sf::Color(0,200,0));
            fill.setPosition(pos);
            w.draw(fill);
        }
    };
    bar(player_.position() + sf::Vector2f{-15.f,-28.f},
        float(player_.hp())/player_.maxHp());
    for (const auto& m : monsters_) if (m->alive())
        bar(m->position() + sf::Vector2f{-15.f,-30.f},
            float(m->hp())/m->maxHp());

    auto b = scoreTxt_.getLocalBounds();
    scoreTxt_.setPosition({ w.getSize().x - b.size.x - 20.f, 10.f });
    w.draw(scoreTxt_);

    if (done_) w.draw(banner_);
}