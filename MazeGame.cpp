// MazeGame.cpp  —  single-file edition, SFML-3-ready  ------------------------
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <random>
#include <stack>
#include <array>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// ----------------------------------------------------------------- utilities
namespace util {
    inline const sf::Font& font() {
        static const sf::Font* fp = []{
            auto* f = new sf::Font;
            f->openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
            return f;
        }();
        return *fp;
    }
    inline float length(sf::Vector2f v){ return std::hypot(v.x,v.y); }
    inline void centre(sf::Text& t,const sf::RectangleShape& box){
        auto b=t.getLocalBounds();
        t.setOrigin({b.size.x/2.f,b.size.y/2.f});
        t.setPosition(box.getPosition()+box.getSize()/2.f);
    }
} // namespace util

// --------------------------------------------------------------------- Maze
class Maze{
public:
    enum class Cell{Wall,Floor,Exit};
    Maze(unsigned w,unsigned h,unsigned cell);

    bool isWallPixel(float x,float y) const;
    sf::Vector2f entrance()const{ return {float(cell_),float(cell_)}; }
    const sf::Vector2f& exitPos()const{ return exit_; }
    sf::FloatRect exitRect()const{
        return {{exit_.x-cell_*.5f,exit_.y-cell_*.5f},{float(cell_),float(cell_)}};
    }
    void draw(sf::RenderWindow&)const;
private:
    unsigned w_,h_,cell_;
    std::vector<std::vector<Cell>> grid_;
    std::vector<sf::FloatRect> walls_;
    sf::Vector2f exit_;
};

Maze::Maze(unsigned w,unsigned h,unsigned cell):w_(w),h_(h),cell_(cell){
    grid_.assign(h_,std::vector<Cell>(w_,Cell::Wall));
    std::mt19937 rng(std::random_device{}());
    std::stack<sf::Vector2i> st;
    auto push=[&](sf::Vector2i c){
        if(grid_[c.y][c.x]==Cell::Wall){grid_[c.y][c.x]=Cell::Floor;st.push(c);}
    };
    push({1,1});
    const sf::Vector2i dir[4]={{2,0},{-2,0},{0,2},{0,-2}};
    while(!st.empty()){
        auto cur=st.top();
        std::array<int,4> ord{0,1,2,3};
        std::shuffle(ord.begin(),ord.end(),rng);
        bool carved=false;
        for(int k:ord){
            sf::Vector2i nxt=cur+dir[k];
            if(nxt.x<=0||nxt.y<=0||nxt.x>=int(w_-1)||nxt.y>=int(h_-1)) continue;
            if(grid_[nxt.y][nxt.x]==Cell::Wall){
                grid_[cur.y+dir[k].y/2][cur.x+dir[k].x/2]=Cell::Floor;
                push(nxt); carved=true; break;
            }
        }
        if(!carved) st.pop();
    }
    grid_[1][1]=Cell::Floor;
    grid_[h_-2][w_-2]=Cell::Exit;
    exit_={float((w_-2)*cell_),float((h_-2)*cell_)};

    for(unsigned y=0;y<h_;++y)
        for(unsigned x=0;x<w_;++x)
            if(grid_[y][x]==Cell::Wall)
                walls_.emplace_back(sf::Vector2f{float(x*cell_),float(y*cell_)},
                                    sf::Vector2f{float(cell_),float(cell_)});
}
bool Maze::isWallPixel(float x,float y)const{
    unsigned cx=x/cell_,cy=y/cell_;
    if(cx>=w_||cy>=h_) return true;
    return grid_[cy][cx]==Cell::Wall;
}
void Maze::draw(sf::RenderWindow& w)const{
    sf::RectangleShape brick({float(cell_),float(cell_)});
    brick.setFillColor({100,100,100});
    for(const auto& r:walls_){
        brick.setPosition({r.left,r.top});
        w.draw(brick);
    }
    sf::RectangleShape sq({cell_/2.f,cell_/2.f});
    for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx){
        sq.setPosition({exit_.x+dx*cell_/2.f,exit_.y+dy*cell_/2.f});
        sq.setFillColor((dx^dy)?sf::Color::Black:sf::Color::White);
        w.draw(sq);
    }
}

// ------------------------------------------------------------------- Player
class Monster; // fwd

class Player{
public:
    explicit Player(sf::Vector2f pos):pos_(pos){
        shape_.setRadius(12); shape_.setOrigin({12,12});
        shape_.setFillColor(sf::Color::Cyan); shape_.setPosition(pos_);
    }
    void update(float dt,const Maze&);
    bool tryHit(Monster&);
    void hurt(int d){ health_-=(d); flash_=.25f; shape_.setFillColor(sf::Color::Red); }
    void heal(int h){ health_=maxHealth_=h; }
    void setAttack(int a){ attack_=a; }
    bool dead()const{ return health_<=0; }
    int  hp()const{return health_;}
    int  maxHp()const{return maxHealth_;}
    sf::Vector2f position()const{return pos_;}
    sf::FloatRect bounds()const{return shape_.getGlobalBounds();}
    const sf::CircleShape& shape()const{return shape_;}
private:
    sf::CircleShape shape_;
    sf::Vector2f pos_;
    float speed_{120.f};
    int attack_{1},health_{10},maxHealth_{10};
    float flash_{0.f};
};

void Player::update(float dt,const Maze& maze){
    if(flash_>0){ flash_-=dt; if(flash_<=0) shape_.setFillColor(sf::Color::Cyan);}
    sf::Vector2f v{};
    using K=sf::Keyboard::Key;
    if(sf::Keyboard::isKeyPressed(K::W)||sf::Keyboard::isKeyPressed(K::Up))   v.y-=1;
    if(sf::Keyboard::isKeyPressed(K::S)||sf::Keyboard::isKeyPressed(K::Down)) v.y+=1;
    if(sf::Keyboard::isKeyPressed(K::A)||sf::Keyboard::isKeyPressed(K::Left)) v.x-=1;
    if(sf::Keyboard::isKeyPressed(K::D)||sf::Keyboard::isKeyPressed(K::Right))v.x+=1;
    if(v==sf::Vector2f{}) return;
    v/=util::length(v); v*=speed_*dt;

    auto tryMove=[&](sf::Vector2f d){
        auto nxt=pos_+d;
        if(!maze.isWallPixel(nxt.x,nxt.y)) pos_=nxt;
    };
    tryMove({v.x,0}); tryMove({0,v.y});
    shape_.setPosition(pos_);
}

// ------------------------------------------------------------------ Bonus &
class Bonus{
public:
    virtual ~Bonus()=default;
    bool collected()const{return collected_;}
    sf::FloatRect bounds()const{return shape_.getGlobalBounds();}
    void draw(sf::RenderWindow& w)const{
        if(!collected_){ w.draw(shape_); w.draw(label_); }
    }
    virtual void apply(Player&)=0;
    virtual int  points()const=0;
protected:
    Bonus(sf::Vector2f pos,char ch,sf::Color fill):label_(util::font(),std::string(1,ch),14){
        shape_.setRadius(12); shape_.setOrigin({12,12});
        shape_.setFillColor(fill); shape_.setPosition(pos);
        label_.setFillColor(sf::Color::Black);
        auto bb=label_.getLocalBounds();
        label_.setOrigin({bb.size.x/2.f,bb.size.y/2.f});
        label_.setPosition(shape_.getPosition());
    }
    sf::CircleShape shape_;
    sf::Text label_;
    bool collected_{false};
};

// tiny macro to create the 3 coin classes
#define COIN(NAME,CH,COL,PTS) \
class NAME:public Bonus{\
public: explicit NAME(sf::Vector2f p):Bonus(p,CH,COL){}\
void apply(Player&)override{collected_=true;}\
int points()const override{return PTS;} }

COIN(BronzeCoin,'B',sf::Color(205,127,50),10);
COIN(SilverCoin,'S',sf::Color(192,192,192),25);
COIN(GoldCoin  ,'G',sf::Color(255,215,0),50);

// ---------------------------------------------------------------- Monster(s)
class Monster{
public:
    Monster(sf::Vector2f pos,float speed,int hp,int dmg,sf::Color body);

    void update(float dt,const Player&,const Maze&);
    void draw(sf::RenderWindow& w)const{
        if(!alive_) return;
        w.draw(shape_);
        float r=float(hp_)/maxHp_;
        sf::RectangleShape back({24,3}),bar({24*r,3});
        back.setFillColor(sf::Color::Black); bar.setFillColor(sf::Color::Green);
        back.setOrigin({12,14}); bar.setOrigin({12,14});
        back.setPosition(shape_.getPosition()); bar.setPosition(shape_.getPosition());
        w.draw(back); w.draw(bar);
    }
    void  hurt(int n){ hp_=std::max(0,hp_-n); if(!hp_) {alive_=false; respawn_=5;} }
    bool  alive()const{return alive_;}
    int   hp()const{return hp_;}
    int   maxHp()const{return maxHp_;}
    int   damage()const{return damage_;}
    sf::Vector2f position()const{return shape_.getPosition();}
    const sf::CircleShape& shape()const{return shape_;}
private:
    sf::CircleShape shape_;
    float speed_;
    int hp_,maxHp_,damage_;
    bool alive_{true};
    float respawn_{5.f},flash_{0.f},atkCd_{0.f};
    sf::Color base_;
};

Monster::Monster(sf::Vector2f pos,float speed,int hp,int dmg,sf::Color body)
:shape_(12),speed_(speed),hp_(hp),maxHp_(hp),damage_(dmg),base_(body){
    shape_.setOrigin({12,12}); shape_.setFillColor(body);
    shape_.setPosition(pos);
}
void Monster::update(float dt,const Player& player,const Maze& maze){
    if(!alive_){
        if((respawn_-=dt)<=0){ alive_=true; hp_=maxHp_; shape_.setFillColor(base_); }
        return;
    }
    if(flash_>0&&(flash_-=dt)<=0) shape_.setFillColor(base_);

    sf::Vector2f dir=player.position()-position();
    float len=util::length(dir);
    if(len>1) dir/=len;
    sf::Vector2f step=dir*speed_*dt, nxt=position()+step;
    if(!maze.isWallPixel(nxt.x,nxt.y)) shape_.setPosition(nxt);

    if((atkCd_-=dt)<=0 && len<10){
        const_cast<Player&>(player).hurt(damage_);
        shape_.setFillColor(sf::Color::Magenta);
        flash_=.25f; atkCd_=.8f;
    }
}

class Zombie   :public Monster{public:Zombie  (sf::Vector2f p,int h,int d,float s):Monster(p,s,h,d,{100,255,100}){}};
class Skeleton :public Monster{public:Skeleton(sf::Vector2f p,int h,int d,float s):Monster(p,s,h,d, sf::Color::White){}};

// --------------------------------------------------------------------- Game
struct Params{
    int playerHp,playerAtk; float playerSpeed;
    unsigned zombies; int zHp,zDmg; float zSpeed;
    unsigned skeletons; int sHp,sDmg; float sSpeed;
};

class Game{
public:
    Game(unsigned w,unsigned h,const Params&);
    void update(float dt);
    void draw(sf::RenderWindow&)const;
    bool finished()const{return done_;}
private:
    static constexpr unsigned cell_=32;
    Maze maze_;
    Player player_;
    std::vector<std::unique_ptr<Monster>> monsters_;
    std::vector<std::unique_ptr<Bonus>>   coins_;
    Params params_;
    sf::Text banner_,scoreTxt_;
    int score_{0}; bool done_{false};

    sf::Vector2f randomPos(std::mt19937& r){
        std::uniform_real_distribution<float> fx(cell_,maze_.exitPos().x);
        std::uniform_real_distribution<float> fy(cell_,maze_.exitPos().y);
        return {fx(r),fy(r)};
    }
};

Game::Game(unsigned w,unsigned h,const Params& p)
:maze_(w,h,cell_),player_(maze_.entrance()),params_(p),
 banner_(util::font(),"You escaped!",48),scoreTxt_(util::font(),"Score: 0",24){
    banner_.setFillColor(sf::Color::Green); banner_.setPosition({50,50});
    std::mt19937 rng(std::random_device{}());
    auto mz=[&](sf::Vector2f pos){return std::make_unique<Zombie>  (pos,p.zHp,p.zDmg,p.zSpeed);};
    auto ms=[&](sf::Vector2f pos){return std::make_unique<Skeleton>(pos,p.sHp,p.sDmg,p.sSpeed);};
    for(unsigned i=0;i<p.zombies;++i)   monsters_.push_back(mz(randomPos(rng)));
    for(unsigned i=0;i<p.skeletons;++i) monsters_.push_back(ms(randomPos(rng)));
    for(int i=0;i<25;++i){
        switch(rng()%3){
            case 0: coins_.push_back(std::make_unique<BronzeCoin>(randomPos(rng)));break;
            case 1: coins_.push_back(std::make_unique<SilverCoin>(randomPos(rng)));break;
            default:coins_.push_back(std::make_unique<GoldCoin  >(randomPos(rng)));
        }
    }
    player_.setAttack(p.playerAtk); player_.heal(p.playerHp);
}

void Game::update(float dt){
    if(done_) return;
    player_.update(dt,maze_);
    for(auto& m:monsters_) m->update(dt,player_,maze_);
    using K=sf::Keyboard::Key;
    if(sf::Keyboard::isKeyPressed(K::Space))
        for(auto& m:monsters_) if(m->alive()) player_.tryHit(*m);
    for(auto& c:coins_) if(!c->collected() && player_.bounds().findIntersection(c->bounds())){
        c->apply(player_); score_+=c->points();
        scoreTxt_.setString("Score: "+std::to_string(score_));
    }
    if(player_.dead()){ banner_.setString("You died!"); banner_.setFillColor(sf::Color::Red); done_=true;}
    if(maze_.exitRect().contains(player_.position())) done_=true;
}
void Game::draw(sf::RenderWindow& w)const{
    maze_.draw(w);
    for(const auto& c:coins_) c->draw(w);
    w.draw(player_.shape());
    for(const auto& m:monsters_) if(m->alive()) w.draw(m->shape());

    auto bar=[&](sf::Vector2f pos,float r){
        r=std::clamp(r,0.f,1.f);
        constexpr float W=30,H=4;
        sf::RectangleShape back({W,H}),fill({W*r,H});
        back.setFillColor({50,50,50}); fill.setFillColor({0,200,0});
        back.setPosition(pos); fill.setPosition(pos);
        w.draw(back); if(r>0) w.draw(fill);
    };
    bar(player_.position()+sf::Vector2f{-15,-28},float(player_.hp())/player_.maxHp());
    for(const auto& m:monsters_) if(m->alive())
        bar(m->position()+sf::Vector2f{-15,-30},float(m->hp())/m->maxHp());

    auto b=scoreTxt_.getLocalBounds();
    scoreTxt_.setPosition({w.getSize().x-b.size.x-20,10});
    w.draw(scoreTxt_); if(done_) w.draw(banner_);
}

// -------------------------------------------------------------- UI helpers
constexpr int cellsW=25,cellsH=18,cell=32;
const Params EASY{10,3,120,  3,2,1,90,  3,2,1,80};
const Params MED {10,2,120,  6,3,2,100, 6,3,2,90};
const Params HARD{10,1,120, 10,4,3,110,10,4,3,100};

enum Pick{None,Easy,Med,Hard,Custom};

struct Button{
    sf::RectangleShape box;
    sf::Text txt;
    Pick val;
    Button(const char* label,Pick v,float x,float y):txt(util::font(),label,20),val(v){
        box.setSize({180,50}); box.setFillColor({70,70,70}); box.setPosition({x,y});
        util::centre(txt,box);
    }
};

static Pick menu(sf::RenderWindow& win){
    sf::Text title(util::font(),"Select difficulty",32); title.setPosition({50,30});
    const char* names[]={"Easy","Medium","Hard","Custom"};
    const Pick  vals []={ Easy ,  Med   , Hard , Custom };
    std::vector<Button> b;
    for(int i=0;i<4;++i) b.emplace_back(names[i],vals[i],70,110+i*70);

    while(win.isOpen()){
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>()) win.close();
            if(ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Left){
                sf::Vector2f m=sf::Vector2f(ev->getIf<sf::Event::MouseButtonPressed>()->position);
                for(auto& x:b) if(x.box.getGlobalBounds().contains(m)) return x.val;
            }
        }
        win.clear({20,20,20});
        win.draw(title); for(auto& x:b){ win.draw(x.box); win.draw(x.txt); } win.display();
    }
    return None;
}

// -------------------------------------------------------------- main loop
int main(){
    sf::RenderWindow win(sf::VideoMode({unsigned(cellsW*cell),unsigned(cellsH*cell)}),
                         "Maze Game",sf::Style::Titlebar|sf::Style::Close);
    win.setVerticalSyncEnabled(true);
restart:
    Pick pick=menu(win); if(!win.isOpen()) return 0;
    Params par=pick==Easy?EASY:pick==Med?MED:pick==Hard?HARD:EASY; // custom removed for brevity
    Game game(cellsW,cellsH,par); sf::Clock clk;

    sf::RectangleShape newB({160,40}),exitB({160,40});
    newB.setFillColor({70,70,70}); exitB.setFillColor({70,70,70});
    sf::Text newT(util::font(),"New Game",20),exitT(util::font(),"Exit",20);
    auto place=[&]{auto s=win.getSize(); newB.setPosition({s.x/2.f-170,s.y/2.f});
                   exitB.setPosition({s.x/2.f+10,s.y/2.f});
                   util::centre(newT,newB); util::centre(exitT,exitB);};
    while(win.isOpen()){
        place();
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>()) win.close();
            if(game.finished()&&ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Left){
                sf::Vector2f m=sf::Vector2f(ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(newB.getGlobalBounds().contains(m)) goto restart;
                if(exitB.getGlobalBounds().contains(m)) win.close();
            }
        }
        float dt=clk.restart().asSeconds();
        if(!game.finished()) game.update(dt);

        win.clear(); game.draw(win);
        if(game.finished()){ win.draw(newB); win.draw(newT); win.draw(exitB); win.draw(exitT); }
        win.display();
    }
}
