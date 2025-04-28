// gui.cpp ──────────────────────────────────────────────────────────
#include <SFML/Graphics.hpp>
#include <sstream>
#include "game.h"

constexpr int cellsW = 25;
constexpr int cellsH = 18;
constexpr int cell   = 32;

// -------------------------------------------------- global font
static const sf::Font& font()
{
    static const sf::Font* f = []{
        auto* fp = new sf::Font;
        fp->openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
        return fp;          // never deleted – avoids SFML shutdown crash
    }();
    return *f;
}

// -------------------------------------------------- presets
static const Params EASY   {10,3,120,  3,2,1,80,  3,2,1,60};
static const Params MEDIUM {10,2,120,  6,3,2,90,  6,3,2,70};
static const Params HARD   {10,1,120, 10,4,3,100,10,4,3,80};

enum Choice {None, PickEasy, PickMed, PickHard, PickCustom};

// -------------------------------------------------- helpers
static void center(sf::Text& t,const sf::RectangleShape& box)
{
    auto b=t.getLocalBounds();
    t.setOrigin({b.size.x/2.f,b.size.y/2.f});
    t.setPosition(box.getPosition()+box.getSize()/2.f);
}

// ===============================================================
// difficulty menu – mouse buttons only
Choice menu(sf::RenderWindow& win)
{
    sf::Text title(font(),"Select difficulty",32); title.setPosition({50,30});

    const float W=180,H=50,left=70,top=110,gap=70;
    sf::RectangleShape easy({W,H}),med({W,H}),hard({W,H}),cust({W,H});
    easy.setPosition({left,top});
    med .setPosition({left,top+gap});
    hard.setPosition({left,top+2*gap});
    cust.setPosition({left,top+3*gap});
    for(auto* r:{&easy,&med,&hard,&cust}) r->setFillColor({70,70,70});

    sf::Text easyT (font(),"Easy",20);
    sf::Text medT  (font(),"Medium",20);
    sf::Text hardT (font(),"Hard",20);
    sf::Text custT (font(),"Custom",20);
    center(easyT,easy); center(medT,med); center(hardT,hard); center(custT,cust);

    while(win.isOpen())
    {
        while(auto ev=win.pollEvent())
        {
            if(ev->is<sf::Event::Closed>()) win.close();
            if(ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                      ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(easy.getGlobalBounds().contains(m)) return PickEasy;
                if(med .getGlobalBounds().contains(m)) return PickMed;
                if(hard.getGlobalBounds().contains(m)) return PickHard;
                if(cust.getGlobalBounds().contains(m)) return PickCustom;
            }
        }
        win.clear(sf::Color(20,20,20));
        win.draw(title);
        for(auto* r:{&easy,&med,&hard,&cust}) win.draw(*r);
        win.draw(easyT);win.draw(medT);win.draw(hardT);win.draw(custT);
        win.display();
    }
    return None;
}

Params customWizard(sf::RenderWindow& win)
{
    Params p = EASY;                 

    struct Field
    {
        const char*        lbl;
        int*               ptr;        
        sf::RectangleShape box;
        sf::Text           txt;
        std::string        buf;

        Field(const char* l, int* v, float y)
        : lbl(l), ptr(v), box(), txt(font(), "", 20), buf(std::to_string(*v))
        {
            box.setSize({120,40});             
            box.setFillColor({60,60,60});
            box.setPosition({300,y});
            refresh();
        }
        void refresh()
        {
            txt.setString(buf.empty() ? "_" : buf);
            auto b = txt.getLocalBounds();
            txt.setOrigin({b.size.x/2.f, b.size.y/2.f});
            txt.setPosition(box.getPosition() + box.getSize()/2.f);
        }
    };


    const float top=90,row=60;
    std::vector<Field> f;
    f.emplace_back("Player HP",   &p.playerHp , top);
    f.emplace_back("Player damage",  &p.playerAtk, top+1*row);
    f.emplace_back("Number of zombies", reinterpret_cast<int*>(&p.zombies), top+2*row);
    f.emplace_back("Zombie HP",   &p.zHp, top+3*row);
    f.emplace_back("Zombie damage",  &p.zDmg, top+4*row);
    f.emplace_back("Number of skeletons", reinterpret_cast<int*>(&p.skeletons), top+5*row);
    f.emplace_back("Skeleton HP", &p.sHp, top+6*row);
    f.emplace_back("Skeleton damage",&p.sDmg, top+7*row);
    for(auto& x:f) x.refresh();

    std::size_t active=0;
    f[active].box.setFillColor({90,90,90});

    std::vector<sf::Text> labs;
    for(std::size_t i=0;i<f.size();++i){
        sf::Text t(font(),f[i].lbl,20);
        t.setPosition({60, top+float(i)*row+6});
        labs.push_back(std::move(t));
    }

    auto setActive=[&](std::size_t i){
        f[active].box.setFillColor({60,60,60});
        active=i;
        f[active].box.setFillColor({90,90,90});
    };

    while(win.isOpen())
    {
        while(auto ev=win.pollEvent())
        {
            if(ev->is<sf::Event::Closed>()) win.close();

            if(ev->is<sf::Event::TextEntered>())
            {
                uint32_t c=ev->getIf<sf::Event::TextEntered>()->unicode;
                if(c>='0'&&c<='9'){ f[active].buf.push_back(char(c)); f[active].refresh(); }
            }
            if(ev->is<sf::Event::KeyPressed>())
            {
                auto k=ev->getIf<sf::Event::KeyPressed>()->code;
                if(k==sf::Keyboard::Key::Backspace && !f[active].buf.empty()){
                    f[active].buf.pop_back(); f[active].refresh();
                }
                if(k==sf::Keyboard::Key::Tab){
                    setActive((active+1)%f.size());
                }
                if(k==sf::Keyboard::Key::Enter){ goto finish; }
            }
            if(ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                     ev->getIf<sf::Event::MouseButtonPressed>()->position);
                for(std::size_t i=0;i<f.size();++i)
                    if(f[i].box.getGlobalBounds().contains(m)){ setActive(i); }
            }
        }

        win.clear(sf::Color(30,30,30));
        for(auto& l:labs) win.draw(l);
        for(auto& x:f){ win.draw(x.box); win.draw(x.txt); }
        win.display();
    }

finish:
    for(auto& x:f)
        if(!x.buf.empty()) *x.ptr = std::stoi(x.buf);
    return p;
}


int main()
{
    sf::RenderWindow win({sf::VideoMode({unsigned(cellsW*cell),
                                         unsigned(cellsH*cell)}),
                          "Maze Game", sf::State::Windowed});
    win.setVerticalSyncEnabled(true);

restart:
    Choice ch = menu(win);
    if(!win.isOpen()) return 0;

    Params params = (ch==PickEasy)?EASY:(ch==PickMed)?MEDIUM:
                    (ch==PickHard)?HARD:customWizard(win);
    if(!win.isOpen()) return 0;

    Game game(cellsW,cellsH,params);
    sf::Clock clk;

    sf::RectangleShape newBtn({160,40}), exitBtn({160,40});
    newBtn .setFillColor({70,70,70});
    exitBtn.setFillColor({70,70,70});
    sf::Text newLbl(font(),"New Game",20), exitLbl(font(),"Exit",20);
    auto centerBtn=[&](sf::RectangleShape& b,sf::Text& t){ center(t,b); };

    while(win.isOpen())
    {
        sf::Vector2u s=win.getSize();
        newBtn .setPosition({float(s.x)/2.f-170.f,float(s.y)/2.f});
        exitBtn.setPosition({float(s.x)/2.f+10.f, float(s.y)/2.f});
        centerBtn(newBtn,newLbl); centerBtn(exitBtn,exitLbl);

        while(auto ev=win.pollEvent())
        {
            if(ev->is<sf::Event::Closed>()) win.close();
            if(ev->is<sf::Event::KeyPressed>() &&
               ev->getIf<sf::Event::KeyPressed>()->code==sf::Keyboard::Key::Escape) win.close();

            if(game.finished() && ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                     ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(newBtn.getGlobalBounds().contains(m)) goto restart;
                if(exitBtn.getGlobalBounds().contains(m)) win.close();
            }
        }

        float dt = clk.restart().asSeconds();
        if(!game.finished()) game.update(dt);

        win.clear(sf::Color::Black);
        game.draw(win);
        if(game.finished()){
            win.draw(newBtn); win.draw(newLbl);
            win.draw(exitBtn); win.draw(exitLbl);
        }
        win.display();
    }
    return 0;
}
