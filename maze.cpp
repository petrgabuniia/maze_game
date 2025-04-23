#include "maze.h"
#include <random>
#include <stack>
#include <algorithm>

Maze::Maze(unsigned w, unsigned h, unsigned cell)
: w_(w), h_(h), cell_(cell)
{
    grid_.assign(h_, std::vector<Cell>(w_, Cell::Wall));

    std::mt19937 rng(std::random_device{}());
    std::stack<sf::Vector2i> stk;
    auto push = [&](sf::Vector2i c)
    {
        if (grid_[c.y][c.x] == Cell::Wall)
        {
            grid_[c.y][c.x] = Cell::Floor;
            stk.push(c);
        }
    };
    push({1,1});

    const sf::Vector2i dir[4] = {{2,0},{-2,0},{0,2},{0,-2}};
    while (!stk.empty())
    {
        sf::Vector2i cur = stk.top();
        std::array<int,4> order{0,1,2,3};
        std::shuffle(order.begin(), order.end(), rng);

        bool carved = false;
        for (int k : order)
        {
            sf::Vector2i nxt = cur + dir[k];
            if (nxt.x<=0||nxt.y<=0||nxt.x>=int(w_-1)||nxt.y>=int(h_-1)) continue;
            if (grid_[nxt.y][nxt.x] == Cell::Wall)
            {
                grid_[cur.y+dir[k].y/2][cur.x+dir[k].x/2] = Cell::Floor;
                push(nxt);
                carved = true;
                break;
            }
        }
        if (!carved) stk.pop();
    }

    grid_[1][1]       = Cell::Floor;
    grid_[h_-2][w_-2] = Cell::Exit;
    exit_             = {float((w_-2)*cell_), float((h_-2)*cell_)};

    for (unsigned y=0;y<h_;++y)
        for (unsigned x=0;x<w_;++x)
            if (grid_[y][x] == Cell::Wall)
                walls_.emplace_back(sf::Vector2f{float(x*cell_), float(y*cell_)},
                                    sf::Vector2f{float(cell_),  float(cell_)});
}
// -----------------------------------------------------------------
bool Maze::isWallPixel(float px, float py) const
{
    unsigned cx = px / cell_, cy = py / cell_;
    if (cx>=w_ || cy>=h_) return true;
    return grid_[cy][cx] == Cell::Wall;
}
// -----------------------------------------------------------------
void Maze::draw(sf::RenderWindow& w) const
{
    sf::RectangleShape brick({float(cell_), float(cell_)});
    brick.setFillColor(sf::Color(100,100,100));

    for (const auto& box : walls_)
    {
        brick.setPosition(box.position);
        w.draw(brick);
    }
}
