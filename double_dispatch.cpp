#include <iostream>

class Asteroid;
class Spacecraft;
class SpaceStation;

class GameObject
{
public:
    virtual void collide(GameObject & rhs) = 0;
    virtual void collide(Asteroid & rhs) = 0;
    virtual void collide(Spacecraft & rhs) = 0;
    virtual void collide(SpaceStation & rhs) = 0;
};

class Asteroid : public GameObject
{
public:
    virtual void collide(GameObject & rhs) override
    {
        rhs.collide(*this);
    }

    virtual void collide(Asteroid & rhs) override
    {
        std::cout << "Asteroid collide with Asteroid" << std::endl;
    }

    virtual void collide(Spacecraft & rhs) override
    {
        std::cout << "Asteroid collide with Spacecraft" << std::endl;
    }

    virtual void collide(SpaceStation & rhs) override
    {
        std::cout << "Asteroid collide with SpaceStation" << std::endl;
    }
};

class Spacecraft : public GameObject
{
public:
    virtual void collide(GameObject & rhs) override
    {
        rhs.collide(*this);
    }

    virtual void collide(Asteroid & rhs) override
    {
        std::cout << "Spacecraft collide with Asteroid" << std::endl;
    }

    virtual void collide(Spacecraft & rhs) override
    {
        std::cout << "Spacecraft collide with Spacecraft" << std::endl;
    }

    virtual void collide(SpaceStation & rhs) override
    {
        std::cout << "Spacecraft collide with SpaceStation" << std::endl;
    }
};

class SpaceStation : public GameObject
{
public:
    virtual void collide(GameObject & rhs) override
    {
        rhs.collide(*this);
    }

    virtual void collide(Asteroid & rhs) override
    {
        std::cout << "SpaceStation collide with Asteroid" << std::endl;
    }

    virtual void collide(Spacecraft & rhs) override
    {
        std::cout << "SpaceStation collide with Spacecraft" << std::endl;
    }

    virtual void collide(SpaceStation & rhs) override
    {
        std::cout << "SpaceStation collide with SpaceStation" << std::endl;
    }
};

int main()
{
    GameObject * asteroid1 = new Asteroid();
    GameObject * asteroid2 = new Asteroid();
    GameObject * spacecraft1 = new Spacecraft();
    GameObject * spaceStation1 = new SpaceStation();
    asteroid1->collide(*asteroid2);
    asteroid1->collide(*spacecraft1);
    asteroid1->collide(*spaceStation1);
}