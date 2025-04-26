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
    sf::Text prompt(font(),"Click or press 1-4",24);
    prompt.setPosition({60,30});

    auto e=block("1) Easy",  EASY,   90);
    auto m=block("2) Medium",MEDIUM, 180);
    auto h=block("3) Hard",  HARD,   270);
    sf::Text c(font(),"4) Custom …",18); c.setPosition({40,360});

    while(win.isOpen())
    {
        while(auto ev=win.pollEvent())
        {
            if(ev->is<sf::Event::Closed>()) win.close();
            if(ev->is<sf::Event::KeyPressed>())
            {
                auto k=ev->getIf<sf::Event::KeyPressed>()->code;
                if(k==sf::Keyboard::Key::Num1) return PickEasy;
                if(k==sf::Keyboard::Key::Num2) return PickMed;
                if(k==sf::Keyboard::Key::Num3) return PickHard;
                if(k==sf::Keyboard::Key::Num4) return PickCustom;
            }
            if(ev->is<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2f p = static_cast<sf::Vector2f>(
                    ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if(e.getGlobalBounds().contains(p)) return PickEasy;
                if(m.getGlobalBounds().contains(p)) return PickMed;
                if(h.getGlobalBounds().contains(p)) return PickHard;
                if(c.getGlobalBounds().contains(p)) return PickCustom;
            }
        }
        win.clear(sf::Color(20,20,20));
        win.draw(prompt); win.draw(e); win.draw(m); win.draw(h); win.draw(c);
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
