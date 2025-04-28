#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Maze
{
public:
    enum class Cell { Wall, Floor, Exit };
    unsigned w_, h_, cell_;

    Maze(unsigned w_, unsigned h_, unsigned cell_);

    bool  isWallPixel(float px, float py) const;
    const std::vector<sf::FloatRect>& walls()   const { return walls_; }
    const sf::Vector2f&               exitPos() const { return exit_; }

    sf::Vector2f entrance() const
    {
        return {static_cast<float>(cell_), static_cast<float>(cell_)};
    }
    sf::FloatRect exitRect() const
    {
        return {{exit_.x - cell_ * .5f, exit_.y - cell_ * .5f},
                {static_cast<float>(cell_), static_cast<float>(cell_)}};
    }

    void draw(sf::RenderWindow&) const;

private:
    
    std::vector<std::vector<Cell>>  grid_;
    std::vector<sf::FloatRect>      walls_;
    sf::Vector2f                    exit_;
};