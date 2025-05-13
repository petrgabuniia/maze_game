 #include <SFML/Graphics.hpp>
 #include <algorithm>
 #include <array>
 #include <cmath>
 #include <memory>
 #include <numeric>
 #include <random>
 #include <stack>
 #include <string>
 #include <variant>
 #include <vector>
 
// GUI font setup 
 namespace util {
     inline const sf::Font& font() {
         static const sf::Font* fp = [] {
             auto* f = new sf::Font;
             bool completed = f->openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
             return f;
         }();
         return *fp;
     }
 
    // Calculate the length of a vector
     template<class T>
     inline auto len(const sf::Vector2<T>& v) { 
        return std::hypot(v.x, v.y); 
    }
 
     // Centre text inside an rectangle.
     inline void centre(sf::Text& t, const sf::FloatRect& box) {
         auto bb = t.getLocalBounds();
         t.setOrigin({ bb.size.x * 0.5f, bb.size.y * 0.5f });
         t.setPosition({box.position.x + box.size.x * 0.5f, box.position.y + box.size.y * 0.5f - 5});
     }
 
     //Pseudo-random integer generator
     inline std::mt19937& rng() { 
        static std::mt19937 eng{ 
            std::random_device{}() 
        }; 
        return eng;
    }
     // Generate a pseudo-random float in the range [a, b)
     inline float randF(float a, float b) {
         std::uniform_real_distribution<float> d(a, b);
         return d(rng());
     }
 
     // Check if two rectangles intersect
     inline bool intersects(const sf::FloatRect& a, const sf::FloatRect& b) {
         return a.position.x < b.position.x + b.size.x &&
                a.position.x + a.size.x > b.position.x &&
                a.position.y < b.position.y + b.size.y &&
                a.position.y + a.size.y > b.position.y;
     }
 } 
 


 class Maze {
 public:
     enum class Cell : std::uint8_t { Wall, Floor, Exit };
     Maze(unsigned w, unsigned h, unsigned cell);

     // Wall detection
     bool isWall(float x, float y) const;
     sf::Vector2f entrance() const { 
        return { 
            static_cast<float>(cell_), static_cast<float>(cell_) 
        }; 
    }
    // Exit position
     sf::Vector2f exitPos() const { 
        return exit_; 
    }
     // Exit rectangle (2×2 cells)
     sf::FloatRect exitRect() const {
         const float half = static_cast<float>(cell_) * 0.5f;
         return { 
            exit_ - sf::Vector2f{ 
                half, half 
            }, 
            { 
                static_cast<float>(cell_), static_cast<float>(cell_) 
            } 
        };
     }
 
     // Draw walls 
     void draw(sf::RenderWindow&) const;
 
  
 private: 
     unsigned w_, h_, cell_;
     std::vector<std::vector<Cell>> grid_; // [row][col]
     std::vector<sf::FloatRect> walls_;    // cached wall rectangles for rendering
     sf::Vector2f exit_;                   // pixel position (centre)
 };
 
 // Maze constructor
 Maze::Maze(unsigned w, unsigned h, unsigned c) : w_(w), h_(h), cell_(c), grid_(h, std::vector<Cell>(w, Cell::Wall)) {
     const std::array<sf::Vector2i, 4> dir{ 
        sf::Vector2i
        { 2, 0 }, 
        { -2, 0 },
        { 0, 2 },
        { 0, -2 } 
    }; // directions for maze carving
     std::stack<sf::Vector2i> st;
     st.push({ 1, 1 });
     grid_[1][1] = Cell::Floor;
 
     auto inMaze = [&](sf::Vector2i p) {
         return p.x > 0 && p.y > 0 && p.x < int(w_ - 1) && p.y < int(h_ - 1);
     };
 
     // Maze generation loop
     while (!st.empty()) {
         auto cur = st.top();
         std::array<int, 4> ord{ 0, 1, 2, 3 };
         std::shuffle(ord.begin(), ord.end(), util::rng());
 
         bool carved = false;
         for (int k : ord) {
             auto nxt = cur + dir[k];
             if (!inMaze(nxt) || grid_[nxt.y][nxt.x] != Cell::Wall) continue;
 
             // Knock down the wall between cur → nxt
             grid_[cur.y + dir[k].y / 2][cur.x + dir[k].x / 2] = Cell::Floor;
             grid_[nxt.y][nxt.x] = Cell::Floor;
             st.push(nxt);
             carved = true;
             break;
         }
         if (!carved) st.pop();
     }
 
     // Place exit (bottom-right corner)
     grid_[h_ - 2][w_ - 2] = Cell::Exit;
     exit_ = { static_cast<float>((w_ - 2) * cell_), static_cast<float>((h_ - 2) * cell_) };
 
     // Cache wall rectangles for rendering
     const float cF = static_cast<float>(cell_);
     for (unsigned y = 0; y < h_; ++y)
         for (unsigned x = 0; x < w_; ++x)
             if (grid_[y][x] == Cell::Wall)
                 walls_.push_back({ { cF * x, cF * y }, { cF, cF } });
 }
 
 bool Maze::isWall(float x, float y) const { 
     if (x < 0 || y < 0) return true; // outside viewport counts as wall
     unsigned cx = static_cast<unsigned>(x) / cell_;
     unsigned cy = static_cast<unsigned>(y) / cell_;
     return cx >= w_ || cy >= h_ || grid_[cy][cx] == Cell::Wall;
 }
 
 void Maze::draw(sf::RenderWindow& w) const {
     // Draw all wall
     sf::RectangleShape brick({ static_cast<float>(cell_), static_cast<float>(cell_) });
     brick.setFillColor({ 100, 100, 100 });
     for (const auto& r : walls_) {
         brick.setPosition(r.position);
         w.draw(brick);
     }
 
     // Checkerboard exit square (2×2)
     sf::RectangleShape sq({ cell_ / 2.f, cell_ / 2.f });
     for (int dy = 0; dy < 2; ++dy)
         for (int dx = 0; dx < 2; ++dx) {
             sq.setPosition(exit_ + sf::Vector2f(dx * cell_ / 2.f, dy * cell_ / 2.f));
             sq.setFillColor((dx ^ dy) ? sf::Color::Black : sf::Color::White);
             w.draw(sq);
         }
 }
 

 //  Actor base classes (Player & Monster)

 class Monster;
 

 class Player {
 public:
     explicit Player(sf::Vector2f startPos);
    
     void update(float dt, const Maze&);
     bool tryHit(Monster&);   // Attempt to hit a monster
     // Attribute mutators
     void hurt(int dmg);
     void heal(int h) { 
        health_ = maxHealth_ = h; 
    }
     void setAttack(int a) { 
        attack_ = a; 
    }
 
     // Queries 
     bool dead() const { 
        return health_ <= 0; 
    }
     int  hp()   const { 
        return health_; 
    }
     int  maxHp()const { 
        return maxHealth_; 
    }
 
     // Rendering helpers 
     sf::Vector2f    pos()    const { 
        return shape_.getPosition(); 
    }
     sf::FloatRect   box()    const { 
        return shape_.getGlobalBounds(); 
    }
     const sf::CircleShape& sprite() const { 
        return shape_; 
    }
 
 private:
     sf::CircleShape shape_;
     float speed_{
         120.f 
        }; // Player speed (pixels per second)
 
     int   attack_     { 
        1 
     };
     int   health_     { 
        10 
     };
     int   maxHealth_  { 
        10 
     };
     int range_     { 
        32 
     }; // melee attack range
     float flash_      { 
        0.f 
    }; // red flash when hurt
 };
 
 Player::Player(sf::Vector2f p) : shape_(12.f) { // Player shape
     shape_.setOrigin({ 12.f, 12.f });
     shape_.setFillColor(sf::Color::Cyan);
     shape_.setPosition(p);
 }
 
 void Player::hurt(int dmg) { // Player takes damage
     health_ = std::max(0, health_ - dmg);
     flash_ = 0.25f;
     shape_.setFillColor(sf::Color::Red);
 }
 
 void Player::update(float dt, const Maze& mz) {
     if ((flash_ -= dt) <= 0) shape_.setFillColor(sf::Color::Cyan); // Reset flash
 
     // Player movement
     sf::Vector2f v;
     using K = sf::Keyboard::Key;
     if (sf::Keyboard::isKeyPressed(K::W) || sf::Keyboard::isKeyPressed(K::Up))   v.y -= 1;
     if (sf::Keyboard::isKeyPressed(K::S) || sf::Keyboard::isKeyPressed(K::Down)) v.y += 1;
     if (sf::Keyboard::isKeyPressed(K::A) || sf::Keyboard::isKeyPressed(K::Left)) v.x -= 1;
     if (sf::Keyboard::isKeyPressed(K::D) || sf::Keyboard::isKeyPressed(K::Right))v.x += 1;
 
     if (v == sf::Vector2f{}) return; // no movement
     v /= util::len(v);
     v *= speed_ * dt;
 
     // Ensures player doesn't move through walls when moving diagonally
     auto attempt = [&](sf::Vector2f d) {
         auto next = pos() + d;
         if (!mz.isWall(next.x, next.y)) shape_.move(d);
     };
     attempt({ v.x, 0 });
     attempt({ 0, v.y });
 }
 
 //  Monster (AI controlled)
 class Monster {
 public:
     Monster(sf::Vector2f pos, float speed, int hp, int dmg, sf::Color); 
     void update(float dt, Player&, const Maze&);
     void draw(sf::RenderWindow&) const;
     void hurt(int);
 
     // Queries 
     bool alive() const {
         return alive_; 
        }
     int  hp()    const { 
        return hp_; 
    }
     int  maxHp() const { 
        return maxHp_; 
    }
     int  dmg()   const {
         return dmg_; 
        }
     sf::Vector2f pos() const { 
        return shape_.getPosition(); 
    }
 
 protected: 
     sf::CircleShape shape_; // monster shape
 
 private:
     float speed_;
     int   hp_, maxHp_, dmg_;
     bool  alive_{ true };
 
     // Timers 
     float respawn_{ 5.f }; // seconds after death
     float flash_  { 0.f }; // hit flash
     float atkCD_  { 0.f }; // attack cooldown (seconds)
 
     const sf::Color base_; // default colour
 };
 
 Monster::Monster(sf::Vector2f p, float s, int hp, int dmg, sf::Color c) : shape_(12.f), speed_(s), hp_(hp), maxHp_(hp), dmg_(dmg), base_(c) { // Monster constructor
     shape_.setOrigin({ 12.f, 12.f });
     shape_.setFillColor(c);
     shape_.setPosition(p);
 }
 
 void Monster::hurt(int n) { // Monster takes damage
     if (!alive_) return;
     hp_ = std::max(0, hp_ - n);
     if (!hp_) {
         alive_ = false;
         respawn_ = 5.f;
     }
 }
 
 void Monster::update(float dt, Player& pl, const Maze& mz) {
     if (!alive_) {
         if ((respawn_ -= dt) <= 0) { //countdown to respawn
             alive_ = true;
             hp_ = maxHp_;
             shape_.setFillColor(base_);
         }
         return;
     }
 
     // Hit flash reset
     if ((flash_ -= dt) <= 0) shape_.setFillColor(base_);
 
     // Monster movement towards player
     sf::Vector2f d = pl.pos() - pos();
     float len = util::len(d);
     if (len > 1) d /= len;
 
     auto next = pos() + d * speed_ * dt;
     if (!mz.isWall(next.x, next.y)) shape_.setPosition(next);
 
     // Attack player if in range
     if ((atkCD_ -= dt) <= 0 && len < 10.f) {
         pl.hurt(dmg_);
         atkCD_ = 0.8f;
         flash_ = 0.25f;
         shape_.setFillColor(sf::Color::Magenta);
     }
 }
 
 void Monster::draw(sf::RenderWindow& w) const {
     if (!alive_) return;
     w.draw(shape_);
 
     // Heealt bar above monsters
     float ratio = std::clamp(float(hp_) / maxHp_, 0.f, 1.f);
     sf::RectangleShape bar({ 24.f * ratio, 3.f  });
     bar.setFillColor(sf::Color::Green);
     bar.setOrigin({ 12.f, 14.f + 10});
     bar.setPosition(shape_.getPosition());
     w.draw(bar);
 }
 
 // Derived classes for different monster types
 class Zombie : public Monster {
    public:
        Zombie(sf::Vector2f p, int h, int d, float s)
            : Monster(p, s, h, d, { 100, 255, 100 }) {}
    };
    
    class Skeleton : public Monster {
    public:
        Skeleton(sf::Vector2f p, int h, int d, float s)
            : Monster(p, s, h, d, sf::Color::White) {}
    };
    
 
 inline bool Player::tryHit(Monster& m) { // Attempt to hit a monster
     if (util::len(pos() - m.pos()) < range_) { 
         m.hurt(attack_);
         return true;
     }
     return false;
 }
 
 
 //  Collectables bonuses (coins)

 class Bonus {
 public:
     virtual ~Bonus() = default;
     bool collected() const { 
        return collected_; 
    }
     sf::FloatRect rect() const { 
        return shape_.getGlobalBounds(); 
    }
 
     void draw(sf::RenderWindow& w) const {
         if (!collected_) { 
            w.draw(shape_); w.draw(label_);
         }
     }
 
     virtual void apply(Player&) = 0; // what happens when the player picks it up
     virtual int  points() const = 0; // score value
 
 protected:
     Bonus(sf::Vector2f pos, char glyph, const sf::Color& col) // constructor
         : label_(util::font(), std::string(1, glyph), 14) {
         shape_.setRadius(12.f);
         shape_.setOrigin({ 12.f, 12.f });
         shape_.setFillColor(col);
         shape_.setPosition(pos);
         label_.setFillColor(sf::Color::Black);
         label_.setPosition(pos + sf::Vector2f{ -6.f, -9.f });
     }
 
     sf::CircleShape shape_;
     sf::Text        label_;
     bool            collected_ { 
        false 
    };
 };
 // Derived classes for different coin types
 class BronzeCoin : public Bonus {
 public: explicit BronzeCoin(sf::Vector2f p) : Bonus(p, 'B', { 205, 127,  50 }) {}
     void apply(Player&) override { 
        collected_ = true;
     }
     int  points() const override {
         return 10; 
        }
 };
 class SilverCoin : public Bonus {
 public: explicit SilverCoin(sf::Vector2f p) : Bonus(p, 'S', { 192, 192, 192 }) {}
     void apply(Player&) override {
         collected_ = true; 
        }
     int  points() const override { 
        return 25; 
    }
 };
 class GoldCoin   : public Bonus {
 public: explicit GoldCoin  (sf::Vector2f p) : Bonus(p, 'G', { 
    255, 215,   0 
}) {}
     void apply(Player&) override { 
        collected_ = true; 
    }
     int  points() const override { 
        return 50; 
    }
 };
 
 // Standart level parameters
 struct Params {
     int    playerHp   = 10;
     int    playerAtk  = 3;
     float  playerSpeed= 120.f;
 
     unsigned zombies   = 3; int zHp = 2, zDmg = 1; float zSpeed = 90.f;
     unsigned skeletons = 3; int sHp = 2, sDmg = 1; float sSpeed = 80.f;
 };
 
 constexpr Params EASY  {10, 3, 120.f,  3, 1, 1, 100.f,  3, 1, 1, 80.f };
 constexpr Params MED   { 10, 2, 120.f,  6, 3, 2, 100.f,  6, 3, 2,  90.f };
 constexpr Params HARD  { 10, 1, 120.f, 10, 4, 3, 110.f, 10, 4, 3, 100.f };
 
//  Game class (main game logic)
 class Game {
     static constexpr unsigned CELL = 32; // maze cell size (pixels)
 public:
     Game(unsigned cols, unsigned rows, const Params&);
     void update(float dt);
     void draw(sf::RenderWindow&) const;
 
     bool finished() const { return done_; }
 
 private:
     // Monster spawnwer
     template<class M>
     void spawn(unsigned n, int hp, int dmg, float sp) {
         std::generate_n(std::back_inserter(monsters_), n, [&] {
             return std::make_unique<M>(sf::Vector2f{
                 util::randF(CELL, maze_.exitPos().x - CELL),
                 util::randF(CELL, maze_.exitPos().y - CELL)
             }, hp, dmg, sp);
         });
     }
 
     // Maze & player 
     Maze maze_;
     Player player_;
     std::vector<std::unique_ptr<Monster>> monsters_;
     std::vector<std::unique_ptr<Bonus>>   coins_;
 
     // UI elements
     sf::Text banner_{ 
        util::font(), "You escaped!", 48 
    };
     mutable sf::Text score_{
         util::font(), "Score: 0", 24 
        };
     int  scr_  { 
        0 
    };
     bool done_ { 
        false
     };
 };
 
 Game::Game(unsigned w, unsigned h, const Params& p) // constructor
     : maze_(w, h, CELL), player_(maze_.entrance()) {
     // Player stats
     player_.setAttack(p.playerAtk);
     player_.heal(p.playerHp);
 
     // Enemies
     spawn<Zombie  >(p.zombies  , p.zHp, p.zDmg, p.zSpeed);
     spawn<Skeleton>(p.skeletons, p.sHp, p.sDmg, p.sSpeed);
 
     // Random coins within maze bounds (not too close to exit)
     for (int i = 0; i < 25; ++i) {
         sf::Vector2f pos{ util::randF(CELL, maze_.exitPos().x - CELL),
                           util::randF(CELL, maze_.exitPos().y - CELL) };
         switch (i % 3) {
         case 0: coins_.push_back(std::make_unique<BronzeCoin>(pos)); break;
         case 1: coins_.push_back(std::make_unique<SilverCoin>(pos)); break;
         default:coins_.push_back(std::make_unique<GoldCoin  >(pos)); break;
         }
     }
 
     banner_.setFillColor(sf::Color::Green);
     banner_.setPosition({ 50.f, 50.f });
 }
 
 void Game::update(float dt) { // game logic
     if (done_) return;
 
     player_.update(dt, maze_);
     for (auto& m : monsters_) m->update(dt, player_, maze_);
 
     // Player attack 
     if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
         for (auto& m : monsters_) if (m->alive()) player_.tryHit(*m);
 
     // Coin collection 
     for (auto& c : coins_)
         if (!c->collected() && util::intersects(player_.box(), c->rect())) {
             c->apply(player_);
             scr_ += c->points();
             score_.setString("Score: " + std::to_string(scr_));
         }
 
     // Win / lose conditions 
     if (player_.dead()) {
         banner_.setString("You died!");
         banner_.setFillColor(sf::Color::Red);
         done_ = true;
     }
     if (maze_.exitRect().contains(player_.pos())) done_ = true;
 }
 
 void Game::draw(sf::RenderWindow& w) const {
     maze_.draw(w);
     for (auto& c : coins_) c->draw(w);
 
     w.draw(player_.sprite());
     for (auto& m : monsters_) if (m->alive()) m->draw(w);
 
     // Player health bar (30 × 4 px above head)
     auto bar = [&](sf::Vector2f p, float ratio) {
         ratio = std::clamp(ratio, 0.f, 1.f);
         sf::RectangleShape fill({ 30.f * ratio, 4.f });
         fill.setFillColor({ 0, 200, 0 });
         fill.setPosition(p);
         w.draw(fill);
     };
     bar(player_.pos() + sf::Vector2f{ -15.f, -28.f },
         float(player_.hp()) / player_.maxHp());
 
     // Right‑top corner score label
     auto bb = score_.getLocalBounds();
     score_.setPosition({ w.getSize().x - bb.size.x - 20.f, 10.f });
     w.draw(score_);
 
     if (done_) w.draw(banner_);
 }

 //  UI helpers 
 
 static Params customForm(sf::RenderWindow& win) {
     Params p{};
 
     // Lightweight form field wrapper (variant targets int|float in Params).
     struct Field {
         sf::RectangleShape box{ { 120.f, 40.f } };
         sf::Text           txt;
         std::variant<int*, float*> ptr;
         std::string        buf;
         const char*        label;
 
         Field(const char* l, int* pi, float* pf, float y)
             : txt(util::font(), "", 20), ptr(pi ? std::variant<int*, float*>{ pi } : pf), label(l) {
             box.setFillColor({ 60, 60, 60 });
             box.setPosition({ 310.f, y });
             buf = pi ? std::to_string(*pi) : std::to_string(int(*pf));
             txt.setString(buf);
             util::centre(txt, box.getGlobalBounds());
         }
 
         void refresh() {
             txt.setString(buf.empty() ? "_" : buf);
             util::centre(txt, box.getGlobalBounds());
         }
         void commit() {
             if (std::holds_alternative<int*>(ptr))
                 *std::get<int*>(ptr) = std::stoi(buf.empty() ? "0" : buf);
             else
                 *std::get<float*>(ptr) = std::stof(buf.empty() ? "0" : buf);
         }
     };
 
     // Lay out form – one row per attribute.
     const float top = 90.f, row = 48.f;
     std::vector<Field> f = {
         { "Player HP",             &p.playerHp,          nullptr,           top },
         { "Player ATK",            &p.playerAtk,         nullptr,           top + row },
         { "Player Spd",            nullptr,             &p.playerSpeed,    top + 2 * row },
         { "Number of zombies",     reinterpret_cast<int*>(&p.zombies), nullptr, top + 3 * row },
         { "Zombie HP",             &p.zHp,               nullptr,           top + 4 * row },
         { "Zombie DMG",            &p.zDmg,              nullptr,           top + 5 * row },
         { "Zombie Spd",            nullptr,             &p.zSpeed,         top + 6 * row },
         { "Number of skeletons",   reinterpret_cast<int*>(&p.skeletons), nullptr, top + 7 * row },
         { "Skeleton HP",           &p.sHp,               nullptr,           top + 8 * row },
         { "Skeleton DMG",          &p.sDmg,              nullptr,           top + 9 * row },
         { "Skeleton Spd",          nullptr,             &p.sSpeed,         top + 10 * row }
     };
 
     size_t idx = 0; // focused field
     f[idx].box.setFillColor({ 90, 90, 90 });
 
     // OK button 
     sf::RectangleShape ok{ { 160.f, 50.f } }; ok.setFillColor({ 70, 70, 70 });
     ok.setPosition({ 150.f, top + 11.5f * row });
     sf::Text okLbl(util::font(), "OK", 24); util::centre(okLbl, ok.getGlobalBounds());
 
     // Event loop (modal) 
     while (win.isOpen()) {
         while (auto ev = win.pollEvent()) {
             if (ev->is<sf::Event::Closed>()) win.close();
 
             // Text input (digits only)
             if (ev->is<sf::Event::TextEntered>()) {
                 char c = char(ev->getIf<sf::Event::TextEntered>()->unicode);
                 if (std::isdigit(c)) { f[idx].buf.push_back(c); f[idx].refresh(); }
             }
 
             if (ev->is<sf::Event::KeyPressed>()) {
                 auto k = ev->getIf<sf::Event::KeyPressed>()->code;
                 if (k == sf::Keyboard::Key::Backspace && !f[idx].buf.empty()) {
                     f[idx].buf.pop_back(); f[idx].refresh();
                 }
                 if (k == sf::Keyboard::Key::Tab) {
                     f[idx].box.setFillColor({ 60, 60, 60 });
                     idx = (idx + 1) % f.size();
                     f[idx].box.setFillColor({ 90, 90, 90 });
                 }
                 if (k == sf::Keyboard::Key::Enter) goto done;
             }
 
             // Mouse clicks processing
             if (ev->is<sf::Event::MouseButtonPressed>() &&
                 ev->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left) {
                 sf::Vector2f p(ev->getIf<sf::Event::MouseButtonPressed>()->position);
 
                 for (size_t i = 0; i < f.size(); ++i)
                     if (f[i].box.getGlobalBounds().contains(p)) {
                         f[idx].box.setFillColor({ 60, 60, 60 });
                         idx = i;
                         f[idx].box.setFillColor({ 90, 90, 90 });
                     }
                 if (ok.getGlobalBounds().contains(p)) goto done;
             }
         }
 
         // Draw frame 
         win.clear({ 30, 30, 30 });
         for (auto& x : f) {
             win.draw(x.box); win.draw(x.txt);
             sf::Text l(util::font(), x.label, 20);
             l.setPosition({ 60.f, x.box.getPosition().y + 8 });
             win.draw(l);
         }
         win.draw(ok); win.draw(okLbl);
         win.display();
     }
 
 done:
     // Commit inserted text field values to Params
     for (auto& x : f) x.commit();
     return p;
 }
 

 //  Main menu (difficulty selection)

 
 enum class Pick { None, Easy, Medium, Hard, Custom };
 
 struct Button {
     sf::RectangleShape box;
     sf::Text           txt;
     Pick               val;
 
     Button(const char* s, Pick v, float y)
         : box({ 180.f, 50.f }), txt(util::font(), s, 20), val(v) {
         box.setFillColor({ 70, 70, 70 });
         box.setPosition({ 70.f, y });
         util::centre(txt, box.getGlobalBounds());
     }
 };
 
 static Pick menu(sf::RenderWindow& win) {
     sf::Text title(util::font(), "Select difficulty", 32);
     title.setPosition({ 50.f, 30.f });
 
     std::array<Button, 4> b{
         Button("Easy",   Pick::Easy,   110.f),
         Button("Medium", Pick::Medium, 180.f),
         Button("Hard",   Pick::Hard,   250.f),
         Button("Custom", Pick::Custom, 320.f)
     };
 
     while (win.isOpen()) {
         while (auto ev = win.pollEvent()) {
             if (ev->is<sf::Event::Closed>()) win.close();
             if (ev->is<sf::Event::MouseButtonPressed>() &&
                 ev->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left) {
                 sf::Vector2f p(ev->getIf<sf::Event::MouseButtonPressed>()->position);
                 for (auto& x : b) if (x.box.getGlobalBounds().contains(p)) return x.val;
             }
         }
 
         win.clear({ 20, 20, 20 });
         win.draw(title);
         for (auto& x : b) { win.draw(x.box); win.draw(x.txt); }
         win.display();
     }
     return Pick::None;
 }
  //  Main function
 int main() {
     constexpr int COLS = 25, ROWS = 18, CELL = 32;
 
     sf::RenderWindow win(
         sf::VideoMode({ unsigned(COLS * CELL), unsigned(ROWS * CELL) }),
         "Maze Game",
         sf::Style::Titlebar | sf::Style::Close);
 
     win.setVerticalSyncEnabled(true);
 
 restart:
     Pick pick = menu(win);
     if (!win.isOpen()) return 0;
 
     Params par  = pick == Pick::Easy   ? EASY  :
                   pick == Pick::Medium ? MED   :
                   pick == Pick::Hard   ? HARD  : customForm(win);
 
     if (!win.isOpen()) return 0;
 
     Game game(COLS, ROWS, par);
     sf::Clock clk;
 
     // Post‑game buttons (drawn only when game.finished())
     sf::RectangleShape newB({ 160.f, 40.f }), exitB({ 160.f, 40.f });
     newB.setFillColor({ 70, 70, 70 });
     exitB.setFillColor({ 70, 70, 70 });
     sf::Text newT(util::font(), "New Game", 20), exitT(util::font(), "Exit", 20);
 
     auto place = [&] {
         auto s = win.getSize();
         newB.setPosition({ s.x / 2.f - 170.f, s.y / 2.f });
         exitB.setPosition({ s.x / 2.f +  10.f, s.y / 2.f });
         util::centre(newT,  newB.getGlobalBounds());
         util::centre(exitT, exitB.getGlobalBounds());
     };
 
     // Main loop 
     while (win.isOpen()) {
         place(); // keep buttons centred on resize
 
         while (auto ev = win.pollEvent()) {
             if (ev->is<sf::Event::Closed>()) win.close();
             if (game.finished() && ev->is<sf::Event::MouseButtonPressed>() &&
                 ev->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left) {
                 sf::Vector2f p(ev->getIf<sf::Event::MouseButtonPressed>()->position);
                 if (newB.getGlobalBounds().contains(p)) goto restart;
                 if (exitB.getGlobalBounds().contains(p)) win.close();
             }
         }
 
         // Fixed‑timestep update 
         float dt = clk.restart().asSeconds();
         if (!game.finished()) game.update(dt);
 
         // Draw 
         win.clear();
         game.draw(win);
         if (game.finished()) { win.draw(newB); win.draw(newT); win.draw(exitB); win.draw(exitT); }
         win.display();
     }
 }
 