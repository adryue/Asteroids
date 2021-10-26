#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <math.h>
#include <vector>
#include <random>

const int WIN_X_LEN = 1280;
const int WIN_Y_LEN = 720;

const int FRAMERATE = 60;

const float SHIP_ACC_SCALE = 0.02;
const float SHIP_ROTATION_SPEED = 5.0;
const float SHIP_WIDTH = 20.0;
const float SHIP_HEIGHT = 40.0;
const float SHIP_RADIUS = SHIP_HEIGHT / 2.0;
const int SHIP_LIVES = 4;
const float SHIP_RESPAWN_MINIMUM_SEPERATION = 150;
const int SHIP_BULLET_LIMIT = 5;
const int SHIP_EXTRA_LIFE = 1000;

const float BULLET_RADIUS = 3.0;
const float BULLET_RELATIVE_SPEED = 4.8;
const int BULLET_DURATION = 80;

const int ASTEROID_INIT_COUNT = 10;
const int ASTEROID_MAX_COUNT = 40;
const float ASTEROID_MAX_SPEED = 2.2;
const int ASTEROID_RADIUS_RANGE = 60;
const int ASTEROID_MIN_RADIUS = 10;
int asteroidSpawnChance = 30;

sf::Font font;
sf::SoundBuffer shipPropelSoundBuffer;
class Bullet;

class Ship : public sf::ConvexShape
{
public:
    sf::Keyboard::Key leftKey;
    sf::Keyboard::Key rightKey;
    sf::Keyboard::Key fwdKey;
    sf::Keyboard::Key fireKey;
    sf::Vector2f velocity;
    std::vector<Bullet> bullets;
    int score;
    sf::Text scoreText;
    int lives;
    sf::Text livesText;
    bool fireKeyPressed;
    bool fwdKeyPressed;
    sf::Sound shipPropelSound;
    int extraLife;

    Ship(float xPos, float yPos, sf::Color color, sf::Keyboard::Key left, sf::Keyboard::Key right, sf::Keyboard::Key fwd, sf::Keyboard::Key fire)
    {
        setPointCount(3);
        setPoint(0, sf::Vector2f(SHIP_WIDTH / 2.0, 0.f));
        setPoint(1, sf::Vector2f(SHIP_WIDTH, SHIP_HEIGHT));
        setPoint(2, sf::Vector2f(0.f, SHIP_HEIGHT));
        setFillColor(color);
        setOrigin(SHIP_WIDTH / 2.0, SHIP_HEIGHT / 2.0);
        setPosition(xPos, yPos);
        leftKey = left;
        rightKey = right;
        fwdKey = fwd;
        fireKey = fire;
        velocity.x = 0.f;
        velocity.y = 0.f;
        score = 0;
        scoreText.setString('0');
        scoreText.setFont(font);
        scoreText.setFillColor(color);
        scoreText.setCharacterSize(40);
        lives = SHIP_LIVES;
        livesText.setFont(font);
        livesText.setFillColor(color);
        livesText.setCharacterSize(40);
        livesText.setString(std::to_string(lives));
        livesText.setOrigin(livesText.getLocalBounds().width + livesText.getLocalBounds().left, 0.0);
        fireKeyPressed = false;
        fwdKeyPressed = false;
        shipPropelSound.setBuffer(shipPropelSoundBuffer);
        shipPropelSound.setLoop(true);
        extraLife = SHIP_EXTRA_LIFE;
    }
    void reset()
    {
        velocity.x = 0.f;
        velocity.y = 0.f;
        setRotation(0.f);
    }
    void resetPoints()
    {
        setPointCount(3);
        setPoint(0, sf::Vector2f(SHIP_WIDTH / 2.0, 0.f));
        setPoint(1, sf::Vector2f(SHIP_WIDTH, SHIP_HEIGHT));
        setPoint(2, sf::Vector2f(0.f, SHIP_HEIGHT));
    }
};
class Bullet : public sf::CircleShape
{
public:
    sf::Vector2f velocity;
    int tick;

    Bullet(sf::Vector2f pos, sf::Vector2f vel, sf::Color color)
    {
        setRadius(BULLET_RADIUS);
        setFillColor(sf::Color(200, 200, 0));
        setOutlineThickness(-BULLET_RADIUS / 2.0);
        setOutlineColor(color);
        setOrigin(BULLET_RADIUS, BULLET_RADIUS);
        setPosition(pos);
        velocity = vel;
        tick = 0;
    }
};
class Asteroid : public sf::CircleShape
{
public:
    sf::Vector2f velocity;
    Asteroid(float radius, sf::Vector2f position, sf::Vector2f vel)
    {
        setRadius(radius);
        setOrigin(radius, radius);
        setFillColor(sf::Color(50, 50, 50));
        setOutlineThickness(-2.f);
        setOutlineColor(sf::Color::White);
        setPosition(position);
        velocity = vel;
    }
};
void checkWrapScreen(sf::Shape &s)
{
    if (s.getPosition().x > (float)WIN_X_LEN - 1.f)
    {
        s.setPosition(0.f, s.getPosition().y);
    }
    if (s.getPosition().x < 0.f)
    {
        s.setPosition((float)WIN_X_LEN - 1.f, s.getPosition().y);
    }
    if (s.getPosition().y > (float)WIN_Y_LEN - 1.f)
    {
        s.setPosition(s.getPosition().x, 0.f);
    }
    if (s.getPosition().y < 0.f)
    {
        s.setPosition(s.getPosition().x, (float)WIN_Y_LEN - 1.f);
    }
}
float checkCollision(sf::Shape &obj, float objRadius, std::vector<Asteroid> &asteroids)
{
    for (unsigned int a = 0; a < asteroids.size(); a++)
    {
        float xDistance = obj.getPosition().x - asteroids[a].getPosition().x;
        float yDistance = obj.getPosition().y - asteroids[a].getPosition().y;
        float distance = std::sqrt(xDistance * xDistance + yDistance * yDistance);
        if (distance <= asteroids[a].getRadius() + objRadius)
        {
            float radius = asteroids[a].getRadius();
            asteroids.erase(asteroids.begin() + a);
            return radius;
        }
    }
    return 0.0;
}
int main()
{
    std::srand(std::time(nullptr));

    int numShips;
    std::cout << "How many players? ";
    std::cin >> numShips;

    sf::RenderWindow window(sf::VideoMode(WIN_X_LEN, WIN_Y_LEN), "Asteroids");
    window.setFramerateLimit(FRAMERATE);
    font.loadFromFile("Hyperspace-JvEM.ttf");

    sf::Text loseScreen;
    loseScreen.setCharacterSize(150);
    loseScreen.setFont(font);
    loseScreen.setFillColor(sf::Color::White);
    loseScreen.setString("GAME OVER");
    loseScreen.setOrigin(loseScreen.getLocalBounds().width / 2.0 + loseScreen.getLocalBounds().left, loseScreen.getLocalBounds().height / 2.0 + loseScreen.getLocalBounds().top);
    loseScreen.setPosition((float)WIN_X_LEN / 2, (float)WIN_Y_LEN / 2);
    bool gameLost = false;

    //load all the sound effects

    //play a sound when a ship is destroyed
    sf::SoundBuffer shipDestroySoundBuffer;
    shipDestroySoundBuffer.loadFromFile("shipDestroy.wav");
    sf::Sound shipDestroySound;
    shipDestroySound.setBuffer(shipDestroySoundBuffer);
    //play a sound when a ship fires a bullet
    sf::SoundBuffer shipFireSoundBuffer;
    shipFireSoundBuffer.loadFromFile("shipFire.wav");
    sf::Sound shipFireSound;
    shipFireSound.setBuffer(shipFireSoundBuffer);
    //play a sound when a ship propels itself
    shipPropelSoundBuffer.loadFromFile("shipPropel.wav");
    //play a sound when an asteroid gets destroyed
    sf::SoundBuffer asteroidDestroySoundBuffer;
    asteroidDestroySoundBuffer.loadFromFile("asteroidDestroy.wav");
    sf::Sound asteroidDestroySound;
    asteroidDestroySound.setBuffer(asteroidDestroySoundBuffer);
    //play a sound when a ship gets an extra life
    sf::SoundBuffer shipExtraLifeSoundBuffer;
    shipExtraLifeSoundBuffer.loadFromFile("shipExtralife.wav");
    sf::Sound shipExtraLifeSound;
    shipExtraLifeSound.setBuffer(shipExtraLifeSoundBuffer);
    //play super epic awesome sound track
    sf::Music music;
    music.openFromFile("SoundTrack.wav");
    music.setLoop(true);
    music.play();


    std::vector<Ship> ships;
    ships.push_back(Ship((int)WIN_X_LEN * 0.75, WIN_Y_LEN / 2, sf::Color::Cyan, sf::Keyboard::Left, sf::Keyboard::Right, sf::Keyboard::Up, sf::Keyboard::Down));
    ships.push_back(Ship((int)WIN_X_LEN * 0.25, WIN_Y_LEN / 2, sf::Color::Red, sf::Keyboard::A, sf::Keyboard::D, sf::Keyboard::W, sf::Keyboard::S));
    ships.push_back(Ship((int)WIN_X_LEN * 0.5, WIN_Y_LEN / 2, sf::Color::Yellow, sf::Keyboard::H, sf::Keyboard::K, sf::Keyboard::U, sf::Keyboard::J));

    for (int i = 3; i > numShips; i--)
    {
        ships.pop_back();
        asteroidSpawnChance += 40;
    }

    std::vector<Asteroid> asteroids;

    // create all asteroids
    for (int i  = 0; i < ASTEROID_INIT_COUNT; i++)
    {
        int radius = std::rand() % ASTEROID_RADIUS_RANGE + ASTEROID_MIN_RADIUS;
        sf::Vector2f position;
        sf::Vector2f velocity(((float)std::rand() / (float)RAND_MAX - 0.5) * 2 * ASTEROID_MAX_SPEED,
                              ((float)std::rand() / (float)RAND_MAX - 0.5) * 2 * ASTEROID_MAX_SPEED);

        switch (std::rand() % 4)
        {
        case 0:
            position.x = std::rand() % WIN_X_LEN;
            position.y = 0;
            break;
        case 1:
            position.x = std::rand() % WIN_X_LEN;
            position.y = WIN_Y_LEN - 1;
            break;
        case 2:
            position.x = 0;
            position.y = std::rand() % WIN_Y_LEN;
            break;
        case 3:
            position.x = WIN_X_LEN - 1;
            position.y = std::rand() % WIN_Y_LEN;
            break;
        }
        asteroids.push_back(Asteroid(radius, position, velocity));
    }
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();

            }
        }
        window.clear();
        // handle ships
        for (Ship& s : ships)
        {
            if (!gameLost && checkCollision(s, SHIP_RADIUS, asteroids) != 0.0)
            {
                shipDestroySound.play();
                if (--s.lives <= 0)
                {
                    gameLost = true;
                    loseScreen.setFillColor(s.getFillColor());
                }
                else
                {
                    // you can't check if the location is valid if there are no asteroids to check it with
                    if (asteroids.size() != 0)
                    {
                        // respawn the ship in a location that is far enough away from all the asteroids
                        bool validCoord = false;
                        float xCoord;
                        float yCoord;
                        float xDistance;
                        float yDistance;
                        float distance;
                        while (!validCoord)
                        {
                            xCoord = std::rand() % WIN_X_LEN;
                            yCoord = std::rand() % WIN_Y_LEN;
                            for (unsigned int a = 0; a < asteroids.size(); a++)
                            {
                                xDistance = xCoord - asteroids[a].getPosition().x;
                                yDistance = yCoord - asteroids[a].getPosition().y;
                                distance = std::sqrt(xDistance * xDistance + yDistance * yDistance);
                                if (distance <= asteroids[a].getRadius() + SHIP_RESPAWN_MINIMUM_SEPERATION)
                                {
                                    validCoord = false;
                                    break;
                                }
                                validCoord = true;
                            }
                        }
                        s.setPosition(xCoord, yCoord);
                        s.reset();
                    }
                }
                s.livesText.setString(std::to_string(s.lives));
                s.livesText.setOrigin(s.livesText.getLocalBounds().width + s.livesText.getLocalBounds().left, 0.0);

            }
            else
            {
                // don't allow the player to move or shoot if the game is already over
                if (!gameLost)
                {
                    //all the movement and firing keys
                    if (sf::Keyboard::isKeyPressed(s.leftKey))
                    {
                        s.rotate(-SHIP_ROTATION_SPEED);
                    }
                    if (sf::Keyboard::isKeyPressed(s.rightKey))
                    {
                        s.rotate(SHIP_ROTATION_SPEED);
                    }
                    if (sf::Keyboard::isKeyPressed(s.fwdKey))
                    {
                        if (!s.fwdKeyPressed)
                        {
                            s.shipPropelSound.play();
                            s.setPointCount(6);
                            s.setPoint(2, sf::Vector2f(SHIP_WIDTH / 2.0 + SHIP_WIDTH / 4.0, SHIP_HEIGHT));
                            s.setPoint(3, sf::Vector2f(SHIP_WIDTH / 2.0, SHIP_HEIGHT + SHIP_HEIGHT / 3.0));
                            s.setPoint(4, sf::Vector2f(SHIP_WIDTH / 2.0 - SHIP_WIDTH / 4.0, SHIP_HEIGHT));
                            s.setPoint(5, sf::Vector2f(0.0, SHIP_HEIGHT));
                        }
                        s.fwdKeyPressed = true;
                        sf::Vector2f accel(SHIP_ACC_SCALE * sin(s.getRotation() / 360 * 2 * M_PI), -SHIP_ACC_SCALE * cos(s.getRotation() / 360 * 2 * M_PI));
                        s.velocity += accel;
                    }
                    else
                    {
                        s.fwdKeyPressed = false;
                        s.shipPropelSound.stop();
                        s.resetPoints();
                    }
                    if (sf::Keyboard::isKeyPressed(s.fireKey))
                    {
                        if (!s.fireKeyPressed && s.bullets.size() < SHIP_BULLET_LIMIT)
                        {
                            // allow the player to only be able to fire one bullet when pressing down
                            // doesn't allow the player to shoot too many bullets
                            s.fireKeyPressed = true;
                            sf::Vector2f relativeVelocity(BULLET_RELATIVE_SPEED * sin(s.getRotation() / 360 * 2 * M_PI), -BULLET_RELATIVE_SPEED * cos(s.getRotation() / 360 * 2 * M_PI));
                            sf::Vector2f finalVelocity(s.velocity + relativeVelocity);
                            s.bullets.push_back(Bullet(s.getPosition(), finalVelocity, s.getFillColor()));
                            shipFireSound.play();
                        }
                    }
                    else
                    {
                        s.fireKeyPressed = false;
                    }
                }
                else
                {
                    s.resetPoints();
                    s.shipPropelSound.stop();
                    s.fwdKeyPressed = false;
                }
                s.move(s.velocity);
                checkWrapScreen(s);
                window.draw(s);

            }
        }

        // handle bullets
        for (Ship& s : ships)
        {
            for (unsigned int b = 0; b < s.bullets.size(); )
            {
                float radius = checkCollision(s.bullets[b], BULLET_RADIUS, asteroids);
                if (++s.bullets[b].tick > BULLET_DURATION)
                {
                    s.bullets.erase(s.bullets.begin() + b);
                }
                else if (radius != 0.0)
                {
                    s.bullets.erase(s.bullets.begin() + b);
                    s.score += 1000 / (int)radius;
                    // when updating the score, reward the player with a life if the score reaches a multiple of 1000
                    if (s.score > s.extraLife)
                    {
                        shipExtraLifeSound.play();
                        s.extraLife += SHIP_EXTRA_LIFE;
                        s.lives++;
                        s.livesText.setString(std::to_string(s.lives));
                        s.livesText.setOrigin(s.livesText.getLocalBounds().width + s.livesText.getLocalBounds().left, 0.0);
                    }
                    s.scoreText.setString(std::to_string(s.score));
                    asteroidDestroySound.play();
                }
                else
                {
                    s.bullets[b].move(s.bullets[b].velocity);
                    checkWrapScreen(s.bullets[b]);
                    window.draw(s.bullets[b]);
                    b++;
                }
            }
        }

        // handle asteroids
        // randomly spawn an asteroid every once in a while, but only if there aren't too many asteroids
        if (rand() % asteroidSpawnChance == 0 && asteroids.size() < ASTEROID_MAX_COUNT)
        {
            int radius = std::rand() % ASTEROID_RADIUS_RANGE + ASTEROID_MIN_RADIUS;
            sf::Vector2f position;
            sf::Vector2f velocity(((float)std::rand() / (float)RAND_MAX - 0.5) * 2 * ASTEROID_MAX_SPEED,
                                  ((float)std::rand() / (float)RAND_MAX - 0.5) * 2 * ASTEROID_MAX_SPEED);

            switch (std::rand() % 4)
            {
            case 0:
                position.x = std::rand() % WIN_X_LEN;
                position.y = 0;
                break;
            case 1:
                position.x = std::rand() % WIN_X_LEN;
                position.y = WIN_Y_LEN - 1;
                break;
            case 2:
                position.x = 0;
                position.y = std::rand() % WIN_Y_LEN;
                break;
            case 3:
                position.x = WIN_X_LEN - 1;
                position.y = std::rand() % WIN_Y_LEN;
                break;
            }
            asteroids.push_back(Asteroid(radius, position, velocity));
        }
        // move asteroids
        for (Asteroid& a : asteroids)
        {
            a.move(a.velocity);
            checkWrapScreen(a);
            window.draw(a);
        }
        // handle drawing the score for all the ships
        float scorePosition = 0.0;
        for (Ship& s : ships)
        {
            s.scoreText.setPosition(5.0, scorePosition);
            window.draw(s.scoreText);
            scorePosition += 50.0;
        }
        // handle drawing the lives for all the ships
        float livesPosition = 0.0;
        for (Ship& s : ships)
        {
            s.livesText.setPosition(WIN_X_LEN - 6.0, livesPosition);
            window.draw(s.livesText);
            livesPosition += 50.0;
        }
        // show the "GAME OVER" text over the window
        if (gameLost)
        {
            window.draw(loseScreen);
        }
        // display the rendered window
        window.display();
    }
    return 0;
}
