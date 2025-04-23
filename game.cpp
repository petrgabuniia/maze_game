#include "game.h"
#include "zombie.h"
#include "skeleton.h"
#include "bronze_coin.h"
#include "silver_coin.h"
#include "gold_coin.h"
#include <SFML/Window/Keyboard.hpp>

const DiffParams Game::DIFF[3]={
/*Easy  */ { 3,2,1,3 },
/*Medium*/ { 6,3,2,2 },
/*Hard  */ {10,4,3,1 }
};

// ------------------------------------------------------------------
Game::Game(unsigned mw,unsigned mh,Difficulty d)
: diff_(d)
, params_(DIFF[int(d)])
, maze_(mw,mh,cell_)
, player_(maze_.entrance())
, winText_{font_}
, scoreText_{font_}
{
    (void)font_.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");

    winText_.setString("You escaped!");
    winText_.setCharacterSize(48);
    winText_.setFillColor(sf::Color::Green);
    winText_.setPosition({50.f,50.f});

    scoreText_.setCharacterSize(24);
    scoreText_.setFillColor(sf::Color::White);
    scoreText_.setString("Score: 0");

    player_.setAttack(params_.playerAtk);

    std::mt19937 rng(std::random_device{}());
    for(unsigned i=0;i<params_.monsters;++i) monsters_.push_back(spawnMonster(rng));
    for(unsigned i=0;i<25;++i)               coins_.push_back(spawnCoin(rng));
}

Game::~Game()=default;

// ------------------------------------------------------------------
std::unique_ptr<Monster> Game::spawnMonster(std::mt19937& rng)
{
    std::uniform_real_distribution<float> fx(cell_,maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_,maze_.exitPos().y);
    sf::Vector2f p{fx(rng),fy(rng)};
    if(rng()&1) return std::make_unique<Zombie>(p,params_.monsterHp,params_.monsterDmg);
    return std::make_unique<Skeleton>(p,params_.monsterHp,params_.monsterDmg);
}

std::unique_ptr<Bonus> Game::spawnCoin(std::mt19937& rng)
{
    std::uniform_real_distribution<float> fx(cell_,maze_.exitPos().x);
    std::uniform_real_distribution<float> fy(cell_,maze_.exitPos().y);
    sf::Vector2f p{fx(rng),fy(rng)};
    switch(rng()%3){
        case 0:return std::make_unique<BronzeCoin>(p);
        case 1:return std::make_unique<SilverCoin>(p);
        default:return std::make_unique<GoldCoin>(p);
    }
}

// ------------------------------------------------------------------
void Game::update(float dt)
{
    if(won_)return;

    player_.update(dt,maze_);
    for(auto& m:monsters_) m->update(dt,player_,maze_);

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        for(auto& m:monsters_) if(m->alive()) player_.tryHit(*m);

    for(auto& c:coins_) if(!c->collected()&&player_.bounds().findIntersection(c->bounds()))
    { c->apply(player_); score_+=c->points(); scoreText_.setString("Score: "+std::to_string(score_)); }

    if(player_.dead()) { winText_.setString("You died!"); winText_.setFillColor(sf::Color::Red); won_=true; }
    if(maze_.exitRect().contains(player_.position())) won_=true;
}

// ------------------------------------------------------------------
void Game::draw(sf::RenderWindow& w) const
{
    maze_.draw(w);
    for(const auto& c:coins_) c->draw(w);
    w.draw(player_.shape());
    for(const auto& m:monsters_) if(m->alive()) w.draw(m->shape());

    auto b=scoreText_.getLocalBounds();
    scoreText_.setPosition({ w.getSize().x-b.size.x-20.f, 10.f });
    w.draw(scoreText_);

    if(won_) w.draw(winText_);

    for(const auto& m:monsters_) if(m->alive()) w.draw(m->shape());

    auto drawBar = [&](sf::Vector2f pos, float ratio)
    {
        constexpr float W = 30.f, H = 4.f;
        sf::RectangleShape back({W, H});
        back.setFillColor(sf::Color(50,50,50));
        back.setPosition(pos);
        sf::RectangleShape fill({W*ratio, H});
        fill.setFillColor(sf::Color(0,200,0));
        fill.setPosition(pos);
        w.draw(back); w.draw(fill);
    };
    
    drawBar(player_.position() + sf::Vector2f{-15.f, -28.f}, float(player_.hp())/player_.maxHp());

    for(const auto& m:monsters_) if(m->alive())
        drawBar(m->position() + sf::Vector2f{-15.f, -30.f}, float(m->hp())/m->maxHp());

}
