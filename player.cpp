#include "player.h"
#include "maze.h"
#include "monsters.h"
#include <SFML/Window/Keyboard.hpp>

Player::Player(const sf::Vector2f& pos):pos_(pos)
{
    shape_.setRadius(12.f);
    shape_.setOrigin({12.f,12.f});
    shape_.setFillColor(sf::Color::Cyan);
    shape_.setPosition(pos_);
}

void Player::hurt(int dmg)
{
    health_ -= dmg;
    flash_   = .25f;
    shape_.setFillColor(sf::Color::Red);
    if (health_ < 0) health_ = 0;
}

void Player::update(float dt,const Maze& maze)
{
    if(flash_>0){ flash_-=dt; if(flash_<=0) shape_.setFillColor(sf::Color::Cyan); }

    sf::Vector2f v{};
    using K=sf::Keyboard::Key;
    if(sf::Keyboard::isKeyPressed(K::W))v.y-=1;
    if(sf::Keyboard::isKeyPressed(K::Up))v.y-=1;
    if(sf::Keyboard::isKeyPressed(K::S))v.y+=1;
    if(sf::Keyboard::isKeyPressed(K::Down))v.y+=1;
    if(sf::Keyboard::isKeyPressed(K::A))v.x-=1;
    if(sf::Keyboard::isKeyPressed(K::Left))v.x-=1;
    if(sf::Keyboard::isKeyPressed(K::D))v.x+=1;
    if(sf::Keyboard::isKeyPressed(K::Right))v.x+=1;
    if(v==sf::Vector2f{})return;
    float len=std::sqrt(v.x*v.x+v.y*v.y);
    v/=len; v*=speed_*dt;

    sf::Vector2f nxt=pos_+sf::Vector2f{v.x,0};
    if(!maze.isWallPixel(nxt.x,nxt.y))pos_.x=nxt.x;
    nxt=pos_+sf::Vector2f{0,v.y};
    if(!maze.isWallPixel(nxt.x,nxt.y))pos_.y=nxt.y;
    shape_.setPosition(pos_);
}

bool Player::tryHit(Monster& m)
{
    if(dist(pos_,m.position())<32.f){ m.hurt(attack_); return true; }
    return false;
}
