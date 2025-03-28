#ifndef MOBILE_H
#define MOBILE_H

#include <string>
#include <vector>
#include "tools.h"
using namespace std;

class Particule {
private:
    S2d position;      
    double alpha;      // Orientation in radians [-π, π]
    double displacement; 
    unsigned int counter;

    bool isInArena() const;

public:
    // Constructor
    Particule(const S2d& pos, double alpha, double disp, unsigned int count = 0);

    // Default constructor
    Particule();

    // Accessors
    S2d getPosition() const;
    double getAlpha() const;
    double getDisplacement() const;
    unsigned int getCounter() const;

    // Mutators
    void setPosition(const S2d& pos);
    void setAlpha(double a);
    void setDisplacement(double d);
    void setCounter(unsigned int c);
    void incrementCounter();

    // Validation
    // handles error message directly
    bool isValid() const; 

    // Debug
    void print() const;  // For testing/debugging
};

class Faiseur {
private:
    S2d position;           // head element
    double alpha;          
    double displacement;   
    double radius;     
    unsigned int numElements; // Number of elements in the maker
    vector<S2d> elements; // Positions of all elements

    bool isInArena() const;
    
public:
    // Constructors
    Faiseur(); 
    Faiseur(const S2d& pos, double alpha, double disp, double rad, unsigned int nbe);
    
    // Accessors
    S2d getPosition() const;
    double getAlpha() const;
    double getDisplacement() const;
    double getRadius() const;
    unsigned int getNumElements() const;
    const vector<S2d>& getElements() const;
    
    // Mutators
    void setPosition(const S2d& pos);
    void setAlpha(double a);
    void setDisplacement(double d);
    void setRadius(double r);
    void setNumElements(unsigned int nbe);
    
    // Calculate positions of all elements 
    void calculateElements();
    
    // Validation
    // handles error message
    bool isValid() const;
    
    // Collision detection
    bool collidesWithArena() const;
    bool collidesWithPoint(const S2d& point) const;
    bool collidesWithFaiseur(const Faiseur& other, 
                            unsigned int thisId = 0, unsigned int otherId = 0) const;
    
    // Debug
    void print() const;
};



#endif