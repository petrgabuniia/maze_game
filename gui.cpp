#include <SFML/Graphics.hpp>
#include <sstream>
#include "game.h"

constexpr int cellsW = 25;
constexpr int cellsH = 18;
constexpr int cell   = 32;


static const sf::Font& font()
{
    static const sf::Font* f = []{
        auto* fp = new sf::Font;
        fp->openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
        return fp;
    }();
    return *f;
}


static const Params EASY   {10,3,120, 3,2,1,80, 3,2,1,60};
static const Params MEDIUM {10,2,120, 6,3,2,80, 6,3,2,60};
static const Params HARD   {10,1,120,10,4,3,80,10,4,3,60};

static sf::Text block(const char* title,const Params& p,float y)
{
    std::ostringstream ss;
    ss<<"Player HP "<<p.playerHp<<" ATK "<<p.playerAtk<<'\n'
      <<"Zombies "<<p.zombies<<" HP "<<p.zHp<<" DMG "<<p.zDmg<<" SPD "<<p.zSpeed<<'\n'
      <<"Skeletons "<<p.skeletons<<" HP "<<p.sHp<<" DMG "<<p.sDmg<<" SPD "<<p.sSpeed;
    sf::Text t(font(), std::string(title)+"\n"+ss.str(), 18);
    t.setPosition({40.f,y});
    return t;
}

enum Choice{None,PickEasy,PickMed,PickHard,PickCustom};


Choice menu(sf::RenderWindow& win)
{
    sf::Text title(font(), "Select difficulty", 32);
    title.setPosition({50, 30});

    const float W = 180, H = 50;
    const float left = 70, top = 110, gap = 70;

    sf::RectangleShape easyBox  ({W, H});
    sf::RectangleShape medBox   ({W, H});
    sf::RectangleShape hardBox  ({W, H});
    sf::RectangleShape custBox  ({W, H});

    easyBox .setPosition({left,      top});
    medBox  .setPosition({left,      top + gap});
    hardBox .setPosition({left,      top + 2*gap});
    custBox .setPosition({left,      top + 3*gap});

    for(auto* r : {&easyBox,&medBox,&hardBox,&custBox})
        r->setFillColor({70,70,70});

    sf::Text easyLbl (font(),"Easy",   20);
    sf::Text medLbl  (font(),"Medium", 20);
    sf::Text hardLbl (font(),"Hard",   20);
    sf::Text custLbl (font(),"Custom",  20);

    easyLbl .setPosition({left+1+W/2.f - 28, top+H/2.f - 13});
    medLbl  .setPosition({left+W/2.f -33, top+H/2.f+gap - 13});
    hardLbl .setPosition({left+W/2.f-26, top+H/2.f+2*gap - 13});
    custLbl .setPosition({left+W/2.f-30, top+H/2.f+3*gap - 13});


    while (win.isOpen())
    {
        while (auto ev = win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();

            if (ev->is<sf::Event::MouseButtonPressed>() &&
                ev->getIf<sf::Event::MouseButtonPressed>()->button ==
                     sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                    ev->getIf<sf::Event::MouseButtonPressed>()->position);

                if (easyBox .getGlobalBounds().contains(m)) return PickEasy;
                if (medBox  .getGlobalBounds().contains(m)) return PickMed;
                if (hardBox .getGlobalBounds().contains(m)) return PickHard;
                if (custBox .getGlobalBounds().contains(m)) return PickCustom;
            }
        }

        win.clear(sf::Color(20,20,20));
        win.draw(title);
        win.draw(easyBox);  win.draw(easyLbl);
        win.draw(medBox);   win.draw(medLbl);
        win.draw(hardBox);  win.draw(hardLbl);
        win.draw(custBox);  win.draw(custLbl);
        win.display();
    }
    return None;
}



static int getNumber(sf::RenderWindow& win,const std::string& label,int def)
{
    std::string buf = std::to_string(def);
    sf::Text t(font(), "", 28); t.setPosition({40,150});

    while(win.isOpen())
    {
        t.setString(label+": "+buf+" (Enter)");
        while(auto ev=win.pollEvent())
        {
            if(ev->is<sf::Event::Closed>()) win.close();
            if(ev->is<sf::Event::KeyPressed>())
            {
                auto k=ev->getIf<sf::Event::KeyPressed>()->code;
                if(k>=sf::Keyboard::Key::Num0&&k<=sf::Keyboard::Key::Num9)
                    buf.push_back(char('0'+(int(k)-int(sf::Keyboard::Key::Num0))));
                if(k==sf::Keyboard::Key::Backspace && !buf.empty()) buf.pop_back();
                if(k==sf::Keyboard::Key::Enter && !buf.empty()) return std::stoi(buf);
            }
        }
        win.clear(); win.draw(t); win.display();
    }
    return def;
}


Params customWizard(sf::RenderWindow& win)
{
    Params p=EASY;
    p.playerHp  = getNumber(win,"Player HP",10);
    p.playerAtk = getNumber(win,"Player ATK",2);
    p.zombies   = getNumber(win,"# Zombies",5);
    p.zHp       = getNumber(win,"Zombie HP",3);
    p.zDmg      = getNumber(win,"Zombie DMG",2);
    p.skeletons = getNumber(win,"# Skeletons",5);
    p.sHp       = getNumber(win,"Skeleton HP",3);
    p.sDmg      = getNumber(win,"Skeleton DMG",2);
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

    Params params = (ch==PickEasy)?EASY:(ch==PickMed)?MEDIUM:(ch==PickHard)?HARD:customWizard(win);
    if(!win.isOpen()) return 0;

    Game game(cellsW,cellsH,params);
    sf::Clock clk;

    sf::RectangleShape newBtn({160,40});
    sf::RectangleShape exitBtn({160,40});
    newBtn .setFillColor({70,70,70});
    exitBtn.setFillColor({70,70,70});

    sf::Text newLbl(font(),"New Game",20);
    sf::Text exitLbl(font(),"Exit",20);

    auto center=[&](sf::Text& txt,sf::RectangleShape& box)
    {
        auto r=txt.getLocalBounds();
        txt.setOrigin({r.size.x/2.f,r.size.y/2.f});
        txt.setPosition(box.getPosition()+box.getSize()/2.f);
    };

    while(win.isOpen())
    {
        sf::Vector2u s=win.getSize();
        newBtn .setPosition({float(s.x)/2.f-170.f,float(s.y)/2.f});
        exitBtn.setPosition({float(s.x)/2.f+10.f, float(s.y)/2.f});
        center(newLbl,newBtn);
        center(exitLbl,exitBtn);

        while(auto ev=win.pollEvent())
        {
            if(ev->is<sf::Event::Closed>()) win.close();

            if(ev->is<sf::Event::KeyPressed>())
            {
                auto k=ev->getIf<sf::Event::KeyPressed>()->code;
                if(k==sf::Keyboard::Key::Escape) win.close();
            }

            if(game.finished() && ev->is<sf::Event::MouseButtonPressed>() &&
               ev->getIf<sf::Event::MouseButtonPressed>()->button==sf::Mouse::Button::Left)
            {
                sf::Vector2f m = static_cast<sf::Vector2f>(
                     ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(newBtn.getGlobalBounds().contains(m)) goto restart;
                if(exitBtn.getGlobalBounds().contains(m)) win.close();
            }
        }

        float dt=clk.restart().asSeconds();
        if(!game.finished()) game.update(dt);

        win.clear(sf::Color::Black);
        game.draw(win);

        if(game.finished())
        {
            win.draw(newBtn);  win.draw(newLbl);
            win.draw(exitBtn); win.draw(exitLbl);
        }
        win.display();
    }
    return 0;
}
