// MazeGame.cpp  —  compact SFML-3 version, event-loop bug fixed
// build: g++ -std=c++17 MazeGame.cpp -I/path/sfml/include -L/path/sfml/lib \
//        -lsfml-graphics -lsfml-window -lsfml-system -o MazeGame
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <random>
#include <stack>
#include <string>
#include <variant>
#include <vector>

namespace util {
    inline const sf::Font& font() {
        static const sf::Font* fp = [] {
            auto* f = new sf::Font;
            (void)f->openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
            return f;
        }();
        return *fp;
    }
    inline float len(sf::Vector2f v) { return std::hypot(v.x, v.y); }
    inline void centre(sf::Text& t, const sf::RectangleShape& box) {
        auto b = t.getLocalBounds();
        t.setOrigin({ b.size.x / 2.f, b.size.y / 2.f });
        t.setPosition(box.getPosition() + box.getSize() / 2.f);
    }
    // ------- RNG helpers -----------------------------------------------------
    inline std::mt19937& rng() {
        static std::mt19937 eng{ std::random_device{}() };
        return eng;
    }
    inline float randF(float a, float b) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(rng());
    }
}
// ───────────────────────────── Maze ─────────────────────────────
class Maze {
public:
    enum class Cell { Wall, Floor, Exit };
    Maze(unsigned w, unsigned h, unsigned c);

    bool isWallPixel(float x, float y) const;
    sf::Vector2f entrance()  const { return { float(cell_),float(cell_) }; }
    const sf::Vector2f& exitPos() const { return exit_; }
    sf::FloatRect exitRect() const {
        return { { exit_.x - cell_ * .5f, exit_.y - cell_ * .5f },
                 { float(cell_), float(cell_) } };
    }
    void draw(sf::RenderWindow&) const;

private:
    unsigned w_, h_, cell_;
    std::vector<std::vector<Cell>> grid_;
    std::vector<sf::FloatRect> walls_;
    sf::Vector2f exit_;
};

Maze::Maze(unsigned w,unsigned h,unsigned c):w_(w),h_(h),cell_(c){
    grid_.assign(h_,std::vector<Cell>(w_,Cell::Wall));
    std::stack<sf::Vector2i> st; st.push({1,1}); grid_[1][1]=Cell::Floor;
    const sf::Vector2i dir[4]={{2,0},{-2,0},{0,2},{0,-2}};
    while(!st.empty()){
        auto cur=st.top(); std::array<int,4> ord{0,1,2,3};
        std::shuffle(ord.begin(),ord.end(),util::rng());
        bool carved=false;
        for(int k:ord){
            sf::Vector2i nxt=cur+dir[k];
            if(nxt.x<=0||nxt.y<=0||nxt.x>=int(w_-1)||nxt.y>=int(h_-1))continue;
            if(grid_[nxt.y][nxt.x]==Cell::Wall){
                grid_[cur.y+dir[k].y/2][cur.x+dir[k].x/2]=Cell::Floor;
                grid_[nxt.y][nxt.x]=Cell::Floor; st.push(nxt); carved=true; break;
            }
        }
        if(!carved) st.pop();
    }
    grid_[h_-2][w_-2]=Cell::Exit;
    exit_={float((w_-2)*cell_),float((h_-2)*cell_)};
    for(unsigned y=0;y<h_;++y)
        for(unsigned x=0;x<w_;++x)
            if(grid_[y][x]==Cell::Wall)
                walls_.emplace_back(sf::Vector2f{float(x*cell_),float(y*cell_)},
                                    sf::Vector2f{float(cell_),float(cell_)});
}
bool Maze::isWallPixel(float x,float y)const{
    unsigned cx=x/cell_, cy=y/cell_;
    return cx>=w_||cy>=h_||grid_[cy][cx]==Cell::Wall;
}
void Maze::draw(sf::RenderWindow& w)const{
    sf::RectangleShape brick({float(cell_),float(cell_)});
    brick.setFillColor({100,100,100});
    for(auto& r:walls_){ brick.setPosition({r.position.x,r.position.y}); w.draw(brick);}
    sf::RectangleShape sq({cell_/2.f,cell_/2.f});
    for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx){
        sq.setPosition({exit_.x+dx*cell_/2.f, exit_.y+dy*cell_/2.f});
        sq.setFillColor((dx^dy)?sf::Color::Black:sf::Color::White); w.draw(sq);
    }
}
// ────────────────────────── Player fwd ────────────────────────
class Monster;
// ─────────────────────────── Player ───────────────────────────
class Player{
public:
    explicit Player(sf::Vector2f p);
    void update(float dt,const Maze&);
    bool tryHit(Monster&);
    void hurt(int d);
    void heal(int h){ health_=maxHealth_=h; }
    void setAttack(int a){ attack_=a; }
    bool dead()const{ return health_<=0; }

    int hp()const{return health_;}
    int maxHp()const{return maxHealth_;}
    sf::Vector2f position()const{return pos_;}
    sf::FloatRect bounds()const{return shape_.getGlobalBounds();}
    const sf::CircleShape& shape()const{return shape_;}
private:
    sf::CircleShape shape_; sf::Vector2f pos_;
    float speed_{120}; int attack_{1},health_{10},maxHealth_{10};
    float flash_{0};
};
Player::Player(sf::Vector2f p):pos_(p){
    shape_.setRadius(12); shape_.setOrigin({12,12});
    shape_.setFillColor(sf::Color::Cyan); shape_.setPosition(p);
}
void Player::hurt(int d){ health_=std::max(0,health_-d); flash_=.25f; shape_.setFillColor(sf::Color::Red);}
void Player::update(float dt,const Maze& maze){
    if(flash_>0&&(flash_-=dt)<=0) shape_.setFillColor(sf::Color::Cyan);
    sf::Vector2f v; using K=sf::Keyboard::Key;
    if(sf::Keyboard::isKeyPressed(K::W)||sf::Keyboard::isKeyPressed(K::Up))   v.y-=1;
    if(sf::Keyboard::isKeyPressed(K::S)||sf::Keyboard::isKeyPressed(K::Down)) v.y+=1;
    if(sf::Keyboard::isKeyPressed(K::A)||sf::Keyboard::isKeyPressed(K::Left)) v.x-=1;
    if(sf::Keyboard::isKeyPressed(K::D)||sf::Keyboard::isKeyPressed(K::Right))v.x+=1;
    if(v==sf::Vector2f{}) return;
    v/=util::len(v); v*=speed_*dt;
    auto tryMove=[&](sf::Vector2f d){ auto nxt=pos_+d; if(!maze.isWallPixel(nxt.x,nxt.y)) pos_=nxt; };
    tryMove({v.x,0}); tryMove({0,v.y}); shape_.setPosition(pos_);
}
// ───────────────────────── Bonus / Coins ─────────────────────
class Bonus{
public:
    virtual ~Bonus()=default;
    bool collected()const{ return collected_; }
    sf::FloatRect bounds()const{ return shape_.getGlobalBounds(); }
    void draw(sf::RenderWindow& w)const{ if(!collected_){ w.draw(shape_); w.draw(label_);} }
    virtual void apply(Player&)=0; virtual int points()const=0;
protected:
    Bonus(sf::Vector2f pos,char ch,sf::Color fill)
        :label_(util::font(),std::string(1,ch),14){
        shape_.setRadius(12); shape_.setOrigin({12,12});
        shape_.setFillColor(fill); shape_.setPosition(pos);
        label_.setFillColor(sf::Color::Black);
        auto bb=label_.getLocalBounds();
        label_.setOrigin({bb.size.x/2.f,bb.size.y/2.f});
        label_.setPosition(shape_.getPosition());
    }
    sf::CircleShape shape_; sf::Text label_; bool collected_{false};
};
template<int Pts,char Ch,uint8_t R,uint8_t G,uint8_t B>
class Coin:public Bonus{
public: explicit Coin(sf::Vector2f p):Bonus(p,Ch,{R,G,B}){}
    void apply(Player&)override{ collected_=true; }
    int points()const override{ return Pts; }
};
using BronzeCoin=Coin<10,'B',205,127,50>;
using SilverCoin=Coin<25,'S',192,192,192>;
using GoldCoin  =Coin<50,'G',255,215,0>;
// ───────────────────────── Monster ───────────────────────────
class Monster{
public:
    Monster(sf::Vector2f p,float s,int hp,int dmg,sf::Color);
    void update(float dt,const Player&,const Maze&);
    void draw(sf::RenderWindow&)const;

    void hurt(int n){ hp_=std::max(0,hp_-n); if(!hp_){alive_=false; respawn_=5;} }
    bool alive()const{ return alive_; }
    int hp()const{return hp_;} int maxHp()const{return maxHp_;}
    int damage()const{return dmg_;}
    sf::Vector2f position()const{return shape_.getPosition();}
    const sf::CircleShape& shape()const{return shape_;}
private:
    sf::CircleShape shape_; float speed_;
    int hp_,maxHp_,dmg_; bool alive_{true};
    float respawn_{5},flash_{0},atkCd_{0}; sf::Color base_;
};
Monster::Monster(sf::Vector2f p,float s,int hp,int dmg,sf::Color c)
    :speed_(s),hp_(hp),maxHp_(hp),dmg_(dmg),base_(c){
    shape_.setRadius(12); shape_.setOrigin({12,12});
    shape_.setFillColor(c); shape_.setPosition(p);
}
void Monster::update(float dt,const Player& pl,const Maze& maze){
    if(!alive_){ if((respawn_-=dt)<=0){alive_=true;hp_=maxHp_;shape_.setFillColor(base_);} return; }
    if(flash_>0&&(flash_-=dt)<=0) shape_.setFillColor(base_);
    sf::Vector2f d=pl.position()-position(); float l=util::len(d);
    if(l>1) d/=l;
    auto nxt=position()+d*speed_*dt;
    if(!maze.isWallPixel(nxt.x,nxt.y)) shape_.setPosition(nxt);
    if((atkCd_-=dt)<=0 && l<10){
        const_cast<Player&>(pl).hurt(dmg_); atkCd_=.8f; flash_=.25f; shape_.setFillColor(sf::Color::Magenta);
    }
}
void Monster::draw(sf::RenderWindow& w)const{
    if(!alive_) return; w.draw(shape_);
    float r=float(hp_)/maxHp_;
    sf::RectangleShape bar({24*r,3}); bar.setFillColor(sf::Color::Green);
    bar.setOrigin({12,14}); bar.setPosition(shape_.getPosition()); w.draw(bar);
}
class Zombie:public Monster{public:Zombie(sf::Vector2f p,int h,int d,float s):Monster(p,s,h,d,{100,255,100}){}};
class Skeleton:public Monster{public:Skeleton(sf::Vector2f p,int h,int d,float s):Monster(p,s,h,d,sf::Color::White){}};
// Player melee
bool Player::tryHit(Monster& m){ if(util::len(position()-m.position())<32){ m.hurt(attack_); return true;} return false; }
// ─────────────── Parameters & presets ───────────────
struct Params{
    int playerHp=10,playerAtk=3; float playerSpeed=120;
    unsigned zombies=3; int zHp=2,zDmg=1; float zSpeed=90;
    unsigned skeletons=3; int sHp=2,sDmg=1; float sSpeed=80;
};
const Params EASY{}, MED{10,2,120,6,3,2,100,6,3,2,90},
              HARD{10,1,120,10,4,3,110,10,4,3,100};
// ───────────────────────── Game ─────────────────────
class Game{
    static constexpr unsigned cell_=32;
public:
    Game(unsigned,unsigned,const Params&);
    void update(float); void draw(sf::RenderWindow&)const;
    bool finished()const{ return done_; }
private:
    template<class M>
    void spawnMany(unsigned n,int hp,int dmg,float sp){
        for(unsigned i=0;i<n;++i)
            monsters_.push_back(std::make_unique<M>(
                sf::Vector2f{util::randF(cell_,maze_.exitPos().x),
                             util::randF(cell_,maze_.exitPos().y)},hp,dmg,sp));
    }
    Maze maze_; Player player_;
    std::vector<std::unique_ptr<Monster>> monsters_;
    std::vector<std::unique_ptr<Bonus>>   coins_;

    sf::Text banner_{util::font(),"You escaped!",48};
    mutable sf::Text score_{util::font(),"Score: 0",24};
    int scr_{0}; bool done_{false};
};

Game::Game(unsigned w,unsigned h,const Params& p)
: maze_(w,h,cell_), player_(maze_.entrance()) {
    player_.setAttack(p.playerAtk); player_.heal(p.playerHp);

    spawnMany<Zombie>(p.zombies   ,p.zHp,p.zDmg,p.zSpeed);
    spawnMany<Skeleton>(p.skeletons,p.sHp,p.sDmg,p.sSpeed);

    for (int i = 0; i < 25; ++i) {
        sf::Vector2f pos{ util::randF(cell_, maze_.exitPos().x),
                          util::randF(cell_, maze_.exitPos().y) };
        switch (i % 3) {
            case 0: coins_.push_back(std::make_unique<BronzeCoin>(pos)); break;
            case 1: coins_.push_back(std::make_unique<SilverCoin>(pos)); break;
            default:coins_.push_back(std::make_unique<GoldCoin  >(pos)); break;
        }
    }

    banner_.setFillColor(sf::Color::Green);
    banner_.setPosition({50, 50});
}
void Game::update(float dt){
    if(done_) return;
    player_.update(dt,maze_); for(auto& m:monsters_) m->update(dt,player_,maze_);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        for(auto& m:monsters_) if(m->alive()) player_.tryHit(*m);
    for(auto& c:coins_) if(!c->collected() && player_.bounds().findIntersection(c->bounds())){
        c->apply(player_); scr_+=c->points(); score_.setString("Score: "+std::to_string(scr_));
    }
    if(player_.dead()){ banner_.setString("You died!"); banner_.setFillColor(sf::Color::Red); done_=true; }
    if(maze_.exitRect().contains(player_.position())) done_=true;
}
void Game::draw(sf::RenderWindow& w)const{
    maze_.draw(w); for(auto& c:coins_) c->draw(w);
    w.draw(player_.shape()); for(auto& m:monsters_) if(m->alive()) w.draw(m->shape());
    auto bar=[&](sf::Vector2f p,float r){ r=std::clamp(r,0.f,1.f);
        sf::RectangleShape fill({30*r,4}); fill.setFillColor({0,200,0});
        fill.setPosition(p); w.draw(fill); };
    bar(player_.position()+sf::Vector2f{-15,-28},float(player_.hp())/player_.maxHp());
    for(auto& m:monsters_) if(m->alive())
        bar(m->position()+sf::Vector2f{-15,-30},float(m->hp())/m->maxHp());
    auto b=score_.getLocalBounds(); score_.setPosition({w.getSize().x-b.size.x-20,10}); w.draw(score_);
    if(done_) w.draw(banner_);
}
// ─────────────────────── UI helpers ─────────────────────────
enum Pick{None,EasyPick,MedPick,HardPick,CustomPick};
struct Button{ sf::RectangleShape box; sf::Text txt; Pick val;
    Button(const char* s,Pick v,float y):box({180,50}),txt(util::font(),s,20),val(v){
        box.setFillColor({70,70,70}); box.setPosition({70,y}); util::centre(txt,box);} };
static Pick menu(sf::RenderWindow& win){
    sf::Text title(util::font(),"Select difficulty",32); title.setPosition({50,30});
    std::array<Button,4> b{ Button("Easy",EasyPick,110),Button("Medium",MedPick,180),
                            Button("Hard",HardPick,250),Button("Custom",CustomPick,320)};
    while(win.isOpen()){
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>()) win.close();
            if(ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left){
                sf::Vector2f p(ev->getIf<sf::Event::MouseButtonPressed>()->position);
                for(auto& x:b) if(x.box.getGlobalBounds().contains(p)) return x.val;
            }
        }
        win.clear({20,20,20}); win.draw(title);
        for(auto& x:b){ win.draw(x.box); win.draw(x.txt);} win.display();
    }
    return None;
}
// ---- custom parameter form (event loops fixed too) -----------
static Params customForm(sf::RenderWindow& win){
    Params p{};
    struct Field{
        sf::RectangleShape box{{120,40}}; sf::Text txt;
        std::variant<int*,float*> ptr; std::string buf; const char* label;
        Field(const char* l,int* pi,float* pf,float y):txt(util::font(),"",20),ptr(pi?std::variant<int*,float*>{pi}:pf),label(l){
            box.setFillColor({60,60,60}); box.setPosition({310,y});
            buf=pi?std::to_string(*pi):std::to_string(int(*pf)); txt.setString(buf); util::centre(txt,box);
        }
        void refresh(){ txt.setString(buf.empty()?"_":buf); util::centre(txt,box);}
        void commit(){
            if(std::holds_alternative<int*>(ptr)) *std::get<int*>(ptr)=std::stoi(buf.empty()?"0":buf);
            else *std::get<float*>(ptr)=std::stof(buf.empty()?"0":buf);
        }
    };
    float top=90,row=48;
    std::vector<Field> f={
        {"Player HP",&p.playerHp,nullptr,top},
        {"Player ATK",&p.playerAtk,nullptr,top+row},
        {"Player Spd",nullptr,&p.playerSpeed,top+2*row},
        {"# Zombies",reinterpret_cast<int*>(&p.zombies),nullptr,top+3*row},
        {"Zombie HP",&p.zHp,nullptr,top+4*row},
        {"Zombie DMG",&p.zDmg,nullptr,top+5*row},
        {"Zombie Spd",nullptr,&p.zSpeed,top+6*row},
        {"# Skeletons",reinterpret_cast<int*>(&p.skeletons),nullptr,top+7*row},
        {"Skel HP",&p.sHp,nullptr,top+8*row},
        {"Skel DMG",&p.sDmg,nullptr,top+9*row},
        {"Skel Spd",nullptr,&p.sSpeed,top+10*row}
    };
    size_t idx=0; f[idx].box.setFillColor({90,90,90});
    sf::RectangleShape ok({160,50}); ok.setFillColor({70,70,70}); ok.setPosition({150,top+11.5f*row});
    sf::Text okLbl(util::font(),"OK",24); util::centre(okLbl,ok);

    while(win.isOpen()){
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>()) win.close();
            if(ev->is<sf::Event::TextEntered>()){
                char c=char(ev->getIf<sf::Event::TextEntered>()->unicode);
                if(std::isdigit(c)){ f[idx].buf.push_back(c); f[idx].refresh();}
            }
            if(ev->is<sf::Event::KeyPressed>()){
                auto k=ev->getIf<sf::Event::KeyPressed>()->code;
                if(k==sf::Keyboard::Key::Backspace&&!f[idx].buf.empty()){ f[idx].buf.pop_back(); f[idx].refresh();}
                if(k==sf::Keyboard::Key::Tab){ f[idx].box.setFillColor({60,60,60}); idx=(idx+1)%f.size(); f[idx].box.setFillColor({90,90,90}); }
                if(k==sf::Keyboard::Key::Enter) goto done;
            }
            if(ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left){
                sf::Vector2f p(ev->getIf<sf::Event::MouseButtonPressed>()->position);
                for(size_t i=0;i<f.size();++i) if(f[i].box.getGlobalBounds().contains(p)){
                    f[idx].box.setFillColor({60,60,60}); idx=i; f[idx].box.setFillColor({90,90,90});
                }
                if(ok.getGlobalBounds().contains(p)) goto done;
            }
        }
        win.clear({30,30,30});
        for(auto& x:f){
            win.draw(x.box); win.draw(x.txt);
            sf::Text l(util::font(),x.label,20); l.setPosition({60,x.box.getPosition().y+8}); win.draw(l);
        }
        win.draw(ok); win.draw(okLbl); win.display();
    }
done:
    for(auto& x:f) x.commit();
    return p;
}
// ─────────────────────────── main ────────────────────────────
int main(){
    constexpr int W=25,H=18,C=32;
    sf::RenderWindow win(
        sf::VideoMode({unsigned(W*C),unsigned(H*C)}),
        "Maze Game",sf::Style::Titlebar|sf::Style::Close);
    win.setVerticalSyncEnabled(true);

restart:
    Pick pick=menu(win); if(!win.isOpen()) return 0;
    Params par = pick==EasyPick?EASY : pick==MedPick?MED :
                 pick==HardPick?HARD : customForm(win);
    if(!win.isOpen()) return 0;

    Game game(W,H,par); sf::Clock clk;
    sf::RectangleShape newB({160,40}), exitB({160,40});
    newB.setFillColor({70,70,70}); exitB.setFillColor({70,70,70});
    sf::Text newT(util::font(),"New Game",20), exitT(util::font(),"Exit",20);
    auto place=[&]{ auto s=win.getSize();
        newB.setPosition({s.x/2.f-170,s.y/2.f}); exitB.setPosition({s.x/2.f+10,s.y/2.f});
        util::centre(newT,newB); util::centre(exitT,exitB); };

    while(win.isOpen()){
        place();
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>()) win.close();
            if(game.finished() && ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left){
                sf::Vector2f p(ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(newB.getGlobalBounds().contains(p)) goto restart;
                if(exitB.getGlobalBounds().contains(p)) win.close();
            }
        }
        float dt=clk.restart().asSeconds();
        if(!game.finished()) game.update(dt);
        win.clear(); game.draw(win);
        if(game.finished()){ win.draw(newB); win.draw(newT); win.draw(exitB); win.draw(exitT); }
        win.display();
    }
}
