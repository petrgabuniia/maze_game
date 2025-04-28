#include <SFML/Graphics.hpp>
#include <vector>
#include "game.h"
#include "util.hpp"          

constexpr int cellsW = 25, cellsH = 18, cell = 32;

static const Params EASY   {10,3,120,  3,2,1,80,  3,2,1,60};
static const Params MEDIUM {10,2,120,  6,3,2,90,  6,3,2,70};
static const Params HARD   {10,1,120, 10,4,3,100,10,4,3,80};

enum Pick {None, Easy, Med, Hard, Custom};


static Pick menu(sf::RenderWindow& win)
{
    sf::Text title(gFont(),"Select difficulty",32); title.setPosition({50,30});

    const char* names[] = {"Easy","Medium","Hard","Custom"};
    const Pick  vals [] = { Easy,   Med,     Hard,  Custom };

    const float W = 180, H = 50, left = 70, top = 110, gap = 70;

    struct Btn              
    {
        sf::RectangleShape box;
        sf::Text           txt;
        Pick               val;
        Btn(const char* name, Pick p, float x, float y)
        : txt(gFont(), name, 20), val(p)
        {
            box.setSize({180.f, 50.f});         
            box.setFillColor({70,70,70});
            box.setPosition({x,y});
            centre(txt, box);
        }

    };

    std::vector<Btn> b;
    for(int i=0;i<4;++i)
        b.emplace_back(names[i], vals[i], left, top + i*gap);

    while(win.isOpen())
    {
        while(auto ev = win.pollEvent())
        {
            if(ev->is<sf::Event::Closed>()) win.close();

            if(ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                     ev->getIf<sf::Event::MouseButtonPressed>()->position);
                for(auto& x : b)
                    if(x.box.getGlobalBounds().contains(m))
                        return x.val;
            }
        }

        win.clear({20,20,20});
        win.draw(title);
        for(auto& x : b){ win.draw(x.box); win.draw(x.txt); }
        win.display();
    }
    return None;
}



static Params custom(sf::RenderWindow& win)
{
    Params p = EASY;
    struct Field{
        const char* lbl;  int* ptr;
        sf::RectangleShape box{{120,40}};
        sf::Text  txt;
        std::string buf;
        Field(const char* l,int* v,float y):lbl(l),ptr(v),txt(gFont(),"",20)
        {
            box.setFillColor({60,60,60});
            box.setPosition({300,y});
            buf = std::to_string(*v); refresh();
        }
        void refresh(){ txt.setString(buf.empty()?"_":buf); centre(txt,box);}
    };
    const float top=90,row=60;
    std::vector<Field> f={
        {"Player HP",   &p.playerHp,                     top},
        {"Player ATK",  &p.playerAtk,                    top+row},
        {"# Zombies",   reinterpret_cast<int*>(&p.zombies),   top+2*row},
        {"Zombie HP",   &p.zHp,                          top+3*row},
        {"Zombie DMG",  &p.zDmg,                         top+4*row},
        {"# Skeletons", reinterpret_cast<int*>(&p.skeletons), top+5*row},
        {"Skeleton HP", &p.sHp,                          top+6*row},
        {"Skeleton DMG",&p.sDmg,                         top+7*row}
    };
    std::size_t a = 0; f[a].box.setFillColor({90,90,90});

    sf::RectangleShape ok{{160,50}}; ok.setFillColor({70,70,70});
    ok.setPosition({180, top+8*row+30});
    sf::Text okLbl(gFont(),"OK",24); centre(okLbl,ok);

    while(win.isOpen()){
        while(auto ev = win.pollEvent()){
            if(ev->is<sf::Event::Closed>()) win.close();

            if(ev->is<sf::Event::TextEntered>()){
                char c = ev->getIf<sf::Event::TextEntered>()->unicode;
                if(c>='0'&&c<='9'){ f[a].buf.push_back(c); f[a].refresh();}
            }
            if(ev->is<sf::Event::KeyPressed>()){
                auto k = ev->getIf<sf::Event::KeyPressed>()->code;
                if(k==sf::Keyboard::Key::Backspace && !f[a].buf.empty()){
                    f[a].buf.pop_back(); f[a].refresh();
                }
                if(k==sf::Keyboard::Key::Tab){
                    f[a].box.setFillColor({60,60,60});
                    a=(a+1)%f.size(); f[a].box.setFillColor({90,90,90});
                }
                if(k==sf::Keyboard::Key::Enter) goto done;
            }
            if(ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                    ev->getIf<sf::Event::MouseButtonPressed>()->position);
                for(std::size_t i=0;i<f.size();++i)
                    if(f[i].box.getGlobalBounds().contains(m)){
                        f[a].box.setFillColor({60,60,60});
                        a=i; f[a].box.setFillColor({90,90,90});
                    }
                if(ok.getGlobalBounds().contains(m)) goto done;
            }
        }
        win.clear({30,30,30});
        for(auto& x:f){
            win.draw(x.box); win.draw(x.txt);
            sf::Text l(gFont(),x.lbl,20);
            l.setPosition({60, x.box.getPosition().y+8});
            win.draw(l);
        }
        win.draw(ok); win.draw(okLbl); win.display();
    }
done:
    for(auto& x:f) if(!x.buf.empty()) *x.ptr = std::stoi(x.buf);
    return p;
}


int main()
{
    sf::RenderWindow win(
        sf::VideoMode({unsigned(cellsW*cell), unsigned(cellsH*cell)}),
        "Maze Game", sf::State::Windowed);
    win.setVerticalSyncEnabled(true);

restart:
    Pick pick = menu(win);
    if(!win.isOpen()) return 0;
    Params par = pick==Easy?EASY : pick==Med?MEDIUM :
                 pick==Hard?HARD : custom(win);
    if(!win.isOpen()) return 0;

    Game game(cellsW,cellsH,par);
    sf::Clock clk;

    sf::RectangleShape newB{{160,40}}, exitB{{160,40}};
    newB.setFillColor({70,70,70}); exitB.setFillColor({70,70,70});
    sf::Text newT(gFont(),"New Game",20), exitT(gFont(),"Exit",20);

    auto placeBtns=[&](){
        auto s=win.getSize();
        newB .setPosition({s.x/2.f-170.f, s.y/2.f});
        exitB.setPosition({s.x/2.f+10.f , s.y/2.f});
        centre(newT,newB); centre(exitT,exitB);
    };

    while(win.isOpen())
    {
        placeBtns();                          
        while(auto ev=win.pollEvent()){
            if(ev->is<sf::Event::Closed>()) win.close();

            if(game.finished() &&
               ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                    ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(newB .getGlobalBounds().contains(m)) goto restart;
                if(exitB.getGlobalBounds().contains(m)) win.close();
            }
        }

        float dt = clk.restart().asSeconds();
        if(!game.finished()) game.update(dt);

        win.clear(sf::Color::Black);
        game.draw(win);
        if(game.finished()){ win.draw(newB); win.draw(newT); win.draw(exitB); win.draw(exitT); }
        win.display();
    }
}
