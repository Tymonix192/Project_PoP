//contributors: 399554 397957
#ifndef MOBILE_H
#define MOBILE_H

#include <string>
#include <vector>
#include <memory>
#include "tools.h"
using namespace std;

// Base class for mobile entities
class Mobile {
protected:
    S2d position;
    double alpha;
    double displacement;

public:
    // Constructors
    Mobile(const S2d& pos, double alpha, double disp);
    Mobile(); // Default constructor
    
    // Virtual destructor for proper inheritance
    virtual ~Mobile() = default;
    
    // Accessors
    S2d getPosition() const;
    double getAlpha() const;
    double getDisplacement() const;
    
    // Mutators
    void setPosition(const S2d& pos);
    void setAlpha(double a);
    void setDisplacement(double d);
    
    // Common functionality
    S2d calculateNextPosition() const;
    
    // Pure virtual methods
    virtual void move() = 0; // Each entity type implements its own movement logic
    virtual bool isValid() const = 0;
    virtual bool isInArena() const = 0;
    virtual int draw() const = 0;
    virtual void print() const = 0;  // For testing/debugging
    
    // Type identification
    virtual bool isParticule() const { return false; }
    virtual bool isFaiseur() const { return false; }
};

class Particule : public Mobile {
private:
    unsigned int counter;

public:
    //Constructors
    Particule(const S2d& pos, double alpha, double disp, unsigned int count = 0);
    Particule(); // Default constructor

    // Accessors
    unsigned int getCounter() const;

    // Mutators
    void setCounter(unsigned int c);
    void incrementCounter();

    // Particle-specific behavior
    void move() override;
    bool shouldSplit() const;
    void createSplitParticles(std::vector<std::unique_ptr<Particule>>& newParticles) const;

    // Overridden methods
    bool isValid() const override;
    bool isInArena() const override;
    int draw() const override;
    void print() const override;
    bool isParticule() const override { return true; }
};

class Faiseur : public Mobile {
private:
    double radius;
    unsigned int numElements; // Number of elements in the maker
    vector<S2d> elements; // Positions of all elements
    
public:
    // Constructors
    Faiseur();
    Faiseur(const S2d& pos, double alpha, double disp, double rad, unsigned int nbe);
    
    // Accessors
    double getRadius() const;
    unsigned int getNumElements() const;
    const vector<S2d>& getElements() const;
    
    // Mutators
    void setRadius(double r);
    void setNumElements(unsigned int nbe);
    
    // Faiseur-specific behavior
    void calculateElements();
    void move() override;
    bool collidesWithPoint(const S2d& point) const;
    bool collidesWithFaiseur(const Faiseur& other, 
                            unsigned int thisId = 0, unsigned int otherId = 0) const;
    
    // Overridden methods
    bool isValid() const override;
    bool isInArena() const override;
    int draw() const override;
    void print() const override;
    bool isFaiseur() const override { return true; }
};

#endif