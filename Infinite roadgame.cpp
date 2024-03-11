#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SelbaWard/src/SelbaWard/Line.hpp>
#include <array>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

float myRandom(float low, float high)
{
    std::uniform_real_distribution<> dist(low, high);
    return dist(gen);
}

class Game
{
private:
    sf::Vector2f vecWindowSize;
    sf::Vector2f vecVanishP;
    sw::Line horizontalLine;
    sw::Line leftTrack;
    sw::Line rightTrack;
    std::vector<sw::Line> middleTrack;
    sf::Vector2f vecVelocity;
    sf::Vector2f vecAcceleration;
    sf::View screenShakeView;
    bool shake;

    // vectors for tree to follow
    sf::Vector2f vecLeftTrackDirectionNorm;
    sf::Vector2f vecRightTrackDirectionNorm;

    // Texture
    // Pink tree
    sf::Texture texPinkTree;
    std::vector<sf::Sprite> sprPinkTrees;

public:

    Game(const sf::RenderWindow& window)
    {
        // Make my life easier variables.
        vecWindowSize = sf::Vector2f(window.getSize());
        vecVanishP = sf::Vector2f(vecWindowSize.x / 2.0f, vecWindowSize.y / 2.0f);

        // Horizontal, Left, and Right tracks.
        horizontalLine.setPoints(sf::Vector2f(0.0f, vecWindowSize.y / 2), sf::Vector2f(vecWindowSize.x, vecWindowSize.y / 2));
        leftTrack.setPoints(sf::Vector2f(0.0f, vecWindowSize.y * 0.8f), vecVanishP);
        rightTrack.setPoints(sf::Vector2f(vecWindowSize.x, vecWindowSize.y * 0.8f), vecVanishP);

        // Middle Track.
        
        sf::Vector2f p1(vecVanishP.x, vecVanishP.y);
        sf::Vector2f p2(vecVanishP.x, vecVanishP.y);
        float lenght;
        float offSet;

        // length of line is relative to it's y location (smaller y location smaller lenght).
        for (int i = 0; i < 16; i++)
        {
            lenght = normalize(p1.y, vecVanishP.y, vecWindowSize.y, 4.0f, 120.0f);
            p2.y += lenght;
            offSet = lenght / 2.0f;
            sw::Line l(p1, p2);
            middleTrack.push_back(l);
            p1.y = p2.y + offSet;
            p2.y = p1.y;
        }

        // Velocity and Acceleration.
        vecVelocity = sf::Vector2f(0.0f, 0.0f);
        vecAcceleration = sf::Vector2f(0.0f, 0.05f);

        // Screen shake
        screenShakeView.reset(sf::FloatRect(0.0f, 0.0f, vecWindowSize.x, vecWindowSize.y));

        // vectors for tree to follow
        // Left track
        sf::Vector2f vecLeftTrackDirection;
        vecLeftTrackDirection = leftTrack.getPoint(0) - vecVanishP; 
        float trackLength = sqrtf(vecLeftTrackDirection.x * vecLeftTrackDirection.x + vecLeftTrackDirection.y * vecLeftTrackDirection.y);
        vecLeftTrackDirectionNorm = vecLeftTrackDirection / trackLength;

        // Right track
        sf::Vector2f vecRightTrackDirection;
        vecRightTrackDirection = rightTrack.getPoint(0) - vecVanishP;
        trackLength = sqrtf(vecRightTrackDirection.x * vecRightTrackDirection.x + vecRightTrackDirection.y * vecRightTrackDirection.y);
        vecRightTrackDirectionNorm = vecRightTrackDirection / trackLength;

        // Tree texture and sprite
        if (!texPinkTree.loadFromFile("Pink tree.png"))
            std::cout << "Error loading tree texture";

        // go twice and add trees for the left track then right track
        for (int i = 0; i < 2; i++)
        {
            float distinceBetweenTrees = 5.0f;
            for (int j = 0; j < 16; j++)
            {
                sf::Sprite sprPinkTree;
                sprPinkTree.setTexture(texPinkTree);

                sf::Vector2f offSet;
                float scale = 0.0f;
                if (i < 1)
                {
                    offSet = vecLeftTrackDirectionNorm * distinceBetweenTrees;
                    // set Origin to buttom right
                    sprPinkTree.setOrigin(sprPinkTree.getGlobalBounds().width, sprPinkTree.getGlobalBounds().height);
                    sprPinkTree.setPosition(vecVanishP);
                    sprPinkTree.move(offSet);

                    // calculate scale relative to y (bigger y bigger scale)
                    scale = normalize(sprPinkTree.getPosition().y, vecVanishP.y, vecWindowSize.y, 0.01f, 0.3f);
                    sprPinkTree.scale(scale, scale);

                    distinceBetweenTrees += sprPinkTree.getGlobalBounds().height;
                }
                else
                {
                    offSet = vecRightTrackDirectionNorm * distinceBetweenTrees;
                    // set Origin to buttom Left
                    sprPinkTree.setOrigin(sprPinkTree.getGlobalBounds().width, sprPinkTree.getGlobalBounds().height);
                    //sprPinkTree.se
                    sprPinkTree.setPosition(vecVanishP);
                    sprPinkTree.move(offSet);

                    // calculate scale relative to y (bigger y bigger scale)
                    scale = normalize(sprPinkTree.getPosition().y, vecVanishP.y, vecWindowSize.y, 0.01f, 0.3f);
                    sprPinkTree.scale(-scale, scale);

                    distinceBetweenTrees += sprPinkTree.getGlobalBounds().height;
                }

                sprPinkTrees.push_back(sprPinkTree);
            }
        }
    }

    float normalize(float x, float min, float max, float r1, float r2)
    {
        return (x - min) / (max - min) * (r2 - r1) + r1;
    }

    void addScreenShake(sf::RenderWindow& window)
    {
        if (shake)
        {
            if (vecVelocity.y > 0.0f && vecVelocity.y < 100.0f)
                screenShakeView.reset(sf::FloatRect(0.0f, 0.0f, vecWindowSize.x + 4.0f, vecWindowSize.y + 3.0f));
            else if (vecVelocity.y > 100.0f)
                screenShakeView.reset(sf::FloatRect(0.0f, 0.0f, vecWindowSize.x + 6.0f, vecWindowSize.y + 4.0f));

            shake = false;
        }
        else
        {
            screenShakeView.reset(sf::FloatRect(0.0f, 0.0f, vecWindowSize.x, vecWindowSize.y));

            shake = true;
        }

        window.setView(screenShakeView);
    }
    
    void update(sf::RenderWindow& window)
    {
        updateInput();
        updateMiddleTrack();
        updateTrees();
        //addScreenShake(window);
    }

    void updateInput()
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {

            vecVelocity.y += vecAcceleration.y;
            std::cout << vecVelocity.y << "\n";

        }
        else
        {
            if (vecVelocity.y > 0.000001f)
                vecVelocity.y -= vecAcceleration.y;

        }
    }

    void updateMiddleTrack()
    {
        for (int i = 0; i < middleTrack.size(); i++)
        {
            // update each p1 (starting point of each line) to velocity.
            // veloctiy is relative to p1's y location (smaller y value smaller speed).
            float lenght = normalize(middleTrack[i].getPoint(0).y, vecVanishP.y, vecWindowSize.y, 4.0f, 120.0f);
            float speedRelativeToY = vecVelocity.y * (lenght / 32.0f);
            sf::Vector2f p1(vecVanishP.x, middleTrack[i].getPoint(0).y + speedRelativeToY);
            sf::Vector2f p2(vecVanishP.x, vecVanishP.y);
            p2.y = p1.y + lenght;
            middleTrack[i].setPoints(p1, p2);

            // Check if space is available for resetting the line at vanishing point.
            bool noCollision = true;
            for (int i = 0; i < middleTrack.size(); i++)
            {
                if (middleTrack[i].getPoint(0).y <= vecVanishP.y + 4.0f + ( 4.0f / 2.0f))
                    noCollision = false;
            }
            // If there's space, set lines that are out of bounds to vanishing point.
            if (p1.y + speedRelativeToY > vecWindowSize.y && noCollision)
            {
                p1 = vecVanishP;
                p2 = vecVanishP;
                lenght = 4.0f;
                p2.y += lenght;
                middleTrack[i].setPoints(p1, p2);
            }

        }

        
    }

    void updateTrees()
    {
        for (int i = 0; i < sprPinkTrees.size(); i++)
        {
            float speedRelativeToY = vecVelocity.y * (normalize(sprPinkTrees[i].getPosition().y, vecVanishP.y, vecWindowSize.y, 4.0f, 120.0f) / 32.0f);
            
            float scale = normalize(sprPinkTrees[i].getPosition().y, vecVanishP.y, vecWindowSize.y, 0.01f, 0.3f);

            // Left trees
            if (i < sprPinkTrees.size() / 2)
            {
                sprPinkTrees[i].move(vecLeftTrackDirectionNorm * speedRelativeToY);
                sprPinkTrees[i].setScale(scale, scale);

            }
            // Right trees
            else
            {
                sprPinkTrees[i].move(vecRightTrackDirectionNorm * speedRelativeToY);
                sprPinkTrees[i].setScale(-scale, scale);
            }

            // Check if space is available for resetting the tree at vanishing point.
            bool noCollision = true;
            sf::Sprite smallestPinkTree = sprPinkTrees[0];
            smallestPinkTree.setScale(0.01f, 0.01f);
            for (int i = 0; i < sprPinkTrees.size(); i++)
            {
                // Left trees
                if (i < sprPinkTrees.size() / 2)
                {
                    if (sprPinkTrees[i].getPosition().y <= vecVanishP.y * vecLeftTrackDirectionNorm.y + smallestPinkTree.getGlobalBounds().height + 5.0f)
                        noCollision = false;

                }
                // Right trees
                else
                {
                    if (sprPinkTrees[i].getPosition().y <= vecVanishP.y * vecRightTrackDirectionNorm.y + smallestPinkTree.getGlobalBounds().height + 5.0f)
                        noCollision = false;
                }
            }

            // Reset if out of bounds
            if ((sprPinkTrees[i].getPosition() + vecLeftTrackDirectionNorm * speedRelativeToY).x <= 0.0f
                || (sprPinkTrees[i].getPosition() + vecRightTrackDirectionNorm * speedRelativeToY).x >= vecWindowSize.x)
            {
                if (noCollision)
                {
                    sprPinkTrees[i].setPosition(vecVanishP);
                    sprPinkTrees[i].setScale(0.01f, 0.01f);
                }
            }
        }
        

    }

    void draw(sf::RenderWindow& window)
    {
      
        window.draw(horizontalLine);
        window.draw(leftTrack);
        window.draw(rightTrack);

        for (int i = 0; i < middleTrack.size(); i++)
            window.draw(middleTrack[i]);
        

        for (int i = 0; i < sprPinkTrees.size(); i++)
            window.draw(sprPinkTrees[i]);
        
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "My window", sf::Style::Fullscreen);
    window.setFramerateLimit(60);
    
    Game game(window);

    while (window.isOpen())
    {
        window.clear();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || event.key.code == sf::Keyboard::Escape)
                window.close();

        }

        game.update(window);
        game.draw(window);
        window.display();   
    }



    return 0;
}
