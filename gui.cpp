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



static const Params EASY   {10,3,120,   3,3,4,60,   3,2,1,79};
static const Params MEDIUM {10,2,120,   5,6,5,70,   5,3,2,99};
static const Params HARD   {10,1,120,   7,9,6,80,   7,4,3,119};

static sf::Text block(const char* title, const Params& p, float y)
{
    std::ostringstream ss;
    ss << "Player's  HP: " << p.playerHp  << ", player's damage per hit: " << p.playerAtk  << '\n'
       << "Number of zombies: "    << p.zombies   << ",  zombie's HP: "  << p.zHp << ", zombie's  damage per hit: " << p.zDmg << ", zombie's speed: " << p.zSpeed << '\n'
       << "Number of skeletons: "  << p.skeletons << ", skeleton's HP: "  << p.sHp << ", skeleton's  damage per hit " << p.sDmg << ", skeleton's speed: " << p.sSpeed;

    sf::Text t(font());          
    t.setString(std::string(title) + "\n" + ss.str());
    t.setCharacterSize(18);
    t.setPosition({40.f, y});
    return t;
}

enum Choice { None, PickEasy, PickMed, PickHard, PickCustom };


Choice menu(sf::RenderWindow& win)
{
    sf::Text prompt(font());
    prompt.setString("Click or press 1-4");
    prompt.setPosition({60, 30});

    auto e = block("1) Easy",   EASY,   90);
    auto m = block("2) Medium", MEDIUM, 180);
    auto h = block("3) Hard",   HARD,   270);

    sf::Text c(font());
    c.setString("4) Custom …");
    c.setCharacterSize(18);
    c.setPosition({40, 360});

    while (win.isOpen())
    {
        while (auto ev = win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();

            if (ev->is<sf::Event::KeyPressed>())
            {
                auto k = ev->getIf<sf::Event::KeyPressed>()->code;
                if (k == sf::Keyboard::Key::Num1) return PickEasy;
                if (k == sf::Keyboard::Key::Num2) return PickMed;
                if (k == sf::Keyboard::Key::Num3) return PickHard;
                if (k == sf::Keyboard::Key::Num4) return PickCustom;
            }
            if (ev->is<sf::Event::MouseButtonPressed>())
            {
                sf::Vector2f p = static_cast<sf::Vector2f>(
                    ev->getIf<sf::Event::MouseButtonPressed>()->position);
                if (e.getGlobalBounds().contains(p)) return PickEasy;
                if (m.getGlobalBounds().contains(p)) return PickMed;
                if (h.getGlobalBounds().contains(p)) return PickHard;
                if (c.getGlobalBounds().contains(p)) return PickCustom;
            }
        }
        win.clear(sf::Color(20,20,20));
        win.draw(prompt); win.draw(e); win.draw(m); win.draw(h); win.draw(c);
        win.display();
    }
    return None;
}


static int getNumber(sf::RenderWindow& win,
                     const std::string& label, int def)
{
    sf::Text t(font());
    t.setCharacterSize(28);
    t.setPosition({40, 150});
    std::string buf = std::to_string(def);

    while (win.isOpen())
    {
        t.setString(label + ": " + buf + " (Enter)");
        while (auto ev = win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();

            if (ev->is<sf::Event::KeyPressed>())
            {
                auto k = ev->getIf<sf::Event::KeyPressed>()->code;
                if (k >= sf::Keyboard::Key::Num0 && k <= sf::Keyboard::Key::Num9)
                    buf.push_back(char('0' + (int(k) - int(sf::Keyboard::Key::Num0))));
                if (k == sf::Keyboard::Key::Backspace && !buf.empty()) buf.pop_back();
                if (k == sf::Keyboard::Key::Enter && !buf.empty()) return std::stoi(buf);
            }
        }
        win.clear(); win.draw(t); win.display();
    }
    return def;
}


Params customWizard(sf::RenderWindow& win)
{
    Params p = EASY; 
    p.playerHp  = getNumber(win, "Player HP",        10);
    p.playerAtk = getNumber(win, "Player attack",     2);
    p.zombies   = getNumber(win, "Number of zombies", 5);
    p.zHp       = getNumber(win, "Zombie HP",         3);
    p.zDmg      = getNumber(win, "Zombie DMG",        2);
    p.skeletons = getNumber(win, "Number of skeletons", 5);
    p.sHp       = getNumber(win, "Skeleton HP",         3);
    p.sDmg      = getNumber(win, "Skeleton DMG",        2);
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
    if (!win.isOpen()) return 0;

    Params params = (ch == PickEasy)  ? EASY
                  : (ch == PickMed)   ? MEDIUM
                  : (ch == PickHard)  ? HARD
                  :                     customWizard(win);

    if (!win.isOpen()) return 0;

    Game game(cellsW, cellsH, params);
    sf::Clock clk;

    while (win.isOpen())
    {
        while (auto ev = win.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) win.close();

            if (ev->is<sf::Event::KeyPressed>())
            {
                auto k = ev->getIf<sf::Event::KeyPressed>()->code;
                if (k == sf::Keyboard::Key::Escape) win.close();
                if (k == sf::Keyboard::Key::R && game.finished()) goto restart;
            }
        }

        float dt = clk.restart().asSeconds();
        game.update(dt);

        win.clear(sf::Color::Black);
        game.draw(win);
        win.display();
    }
    return 0;
}
