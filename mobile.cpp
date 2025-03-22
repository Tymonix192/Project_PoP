#include "mobile.h"
#include "constantes.h"
#include "message.h"
#include "tools.h"
#include <iostream>
#include <sstream>
#include <cmath>
using namespace std;

//Particle class
// Constructor
Particule::Particule(const S2d& pos, double alpha, double disp, unsigned int count) :
    position(pos), alpha(alpha), displacement(disp), counter(count) {}

Particule::Particule() :
    position(), alpha(0.0), displacement(0.0), counter(0) {}

// Accessors
S2d Particule::getPosition() const {
    return position;
}

double Particule::getAlpha() const {
    return alpha;
}

double Particule::getDisplacement() const {
    return displacement;
}

unsigned int Particule::getCounter() const {
    return counter;
}

// Mutators
void Particule::setPosition(const S2d& pos) {
    position = pos;
}

void Particule::setAlpha(double a) {
    //alpha is within [-π, π]
    while (a > M_PI)
        a -= 2 * M_PI;
    while (a < -M_PI)
        a += 2 * M_PI;
    alpha = a;
}

void Particule::setDisplacement(double d) {
    displacement = d;
}

void Particule::setCounter(unsigned int c) {
    counter = c;
}

void Particule::incrementCounter() {
    counter++;
}

// File parsing
bool Particule::readFromLine(const string& line) {
    istringstream iss(line);
    double x, y, a, d;
    unsigned int c;
    
    if (!(iss >> x >> y >> a >> d >> c)) {
        return false;
    }
    
    // Set the attributes
    position.x = x;
    position.y = y;
    setAlpha(a);  // normalize the angle
    displacement = d;
    counter = c;
    
    // Validate the particle
    return isValid();
}

// Validation
bool Particule::isInArena() const {
    Circle arenaCircle(ORIGIN, r_max);
    return distance(position, ORIGIN) <= r_max;
}

bool Particule::isValid() const {
    // Check if the particle is valid
    if (!isInArena()) {
        return false;
    }
    
    // Check displacement bounds
    if (displacement < 0 || displacement > d_max) {
        return false;
    }
    
    // Check counter bounds
    if (counter >= time_to_split) {
        return false;
    }
    
    return true;
}

// Debug
void Particule::print() const {
    cout << "Particule: Position(" << position.x << ", " << position.y << "), "
              << "Alpha: " << alpha << ", "
              << "Displacement: " << displacement << ", "
              << "Counter: " << counter << endl;
}



//class Faiseur

// Constructors
Faiseur::Faiseur() :
    position(), alpha(0.0), displacement(0.0), radius(r_min_faiseur), numElements(1) {
    calculateElements();
}

Faiseur::Faiseur(const S2d& pos, double alpha, double disp, double rad, unsigned int nbe) :
    position(pos), alpha(alpha), displacement(disp), radius(rad), numElements(nbe) {
    calculateElements();
}

// Accessors
S2d Faiseur::getPosition() const {
    return position;
}

double Faiseur::getAlpha() const {
    return alpha;
}

double Faiseur::getDisplacement() const {
    return displacement;
}

double Faiseur::getRadius() const {
    return radius;
}

unsigned int Faiseur::getNumElements() const {
    return numElements;
}

const std::vector<S2d>& Faiseur::getElements() const {
    return elements;
}

// Mutators
void Faiseur::setPosition(const S2d& pos) {
    position = pos;
    calculateElements();
}

void Faiseur::setAlpha(double a) {
    while (a > M_PI)
        a -= 2 * M_PI;
    while (a < -M_PI)
        a += 2 * M_PI;
    alpha = a;
    calculateElements();
}

void Faiseur::setDisplacement(double d) {
    displacement = d;
    calculateElements();
}

void Faiseur::setRadius(double r) {
    radius = r;
}

void Faiseur::setNumElements(unsigned int nbe) {
    numElements = nbe;
    calculateElements();
}

// Calculate positions of all elements
void Faiseur::calculateElements() {
    elements.clear();
    elements.push_back(position); // Head element
    
    // Calculate opposite direction of movement
    double oppositeAlpha = alpha + M_PI;
    if (oppositeAlpha > M_PI) {
        oppositeAlpha -= 2 * M_PI;
    }
    
    Vector prevVector;
    S2d prevPos = position;
    double currentAlpha = oppositeAlpha;
    
    // Generate positions for all elements after the head
    for (unsigned int i = 1; i < numElements; ++i) {
        Vector moveVector;
        moveVector.set_coordinates(prevPos, 
            {prevPos.x + displacement * cos(currentAlpha), 
             prevPos.y + displacement * sin(currentAlpha)});
        
        // Get the end point as the next position
        S2d nextPos = moveVector.get_end();
        // Check if the position is inside the arena
        Circle arenaCircle(ORIGIN, r_max);
        Circle elementCircle(nextPos, radius);
        
        if (!elementCircle.check_inside(arenaCircle)) {
            Vector toCenter;
            toCenter.set_coordinates(ORIGIN, prevPos);
            currentAlpha = moveVector.bounce();
            // Recalculate position with new angle
            moveVector.set_angle(currentAlpha);
            nextPos = moveVector.get_end();
        }
        
        elements.push_back(nextPos);
        prevPos = nextPos;
    }
}

// File parsing
bool Faiseur::readFromLine(const string& line) {
    istringstream iss(line);
    double x, y, a, d, r;
    unsigned int nbe;
    
    // Parse the line
    if (!(iss >> x >> y >> a >> d >> r >> nbe)) {
        return false;
    }
    
    // Set basic attributes
    position.x = x;
    position.y = y;
    setAlpha(a);
    displacement = d;
    radius = r;
    numElements = nbe;
    
    // Calculate element positions
    calculateElements();
    
    // Validate the maker
    return isValid();
}

// Validation
bool Faiseur::isInArena() const {
    Circle arenaCircle(ORIGIN, r_max);
    // Check if all elements of the maker are within the arena
    for (const S2d& element : elements) {
        Circle elementCircle(element, radius);
        if (!elementCircle.check_inside(arenaCircle)) {
            return false;
        }
    }
    return true;
}

bool Faiseur::isValid() const {
    // Check if numElements is greater than 0
    if (numElements == 0) {
        return false;
    }
    // Check radius bounds
    if (radius < r_min_faiseur || radius > r_max_faiseur) {
        return false;
    }
    // Check displacement bounds
    if (displacement < 0 || displacement > d_max) {
        return false;
    }
    // Check if all elements are inside arena
    if (!isInArena()) {
        return false;
    }
    
    return true;
}

// Collision detection
bool Faiseur::collidesWithArena() const {
    Circle arenaCircle(ORIGIN, r_max);
    // Check if any element collides with the arena boundary
    for (const S2d& element : elements) {
        Circle elementCircle(element, radius);
        if (!elementCircle.check_inside(arenaCircle)) {
            return true;
        }
    }
    return false;
}

bool Faiseur::collidesWithFaiseur(const Faiseur& other) const {
    for (const S2d& elem1 : elements) {
        Circle circle1(elem1, radius);
        
        for (const S2d& elem2 : other.elements) {
            Circle circle2(elem2, other.radius);
            if (circles_intersect(circle1, circle2)) {
                return true;
            }
        }
    }
    return false;
}

bool Faiseur::collidesWithPoint(const S2d& point) const {
    for (const S2d& element : elements) {
        double dist = distance(point, element);
        
        // Point is inside element
        if (dist < radius) {
            return true;
        }
    }
    return false;
}

// Debug
void Faiseur::print() const {
    cout << "Faiseur: Head(" << position.x << ", " << position.y << "), "
              << "Alpha: " << alpha << ", "
              << "Displacement: " << displacement << ", "
              << "Radius: " << radius << ", "
              << "NumElements: " << numElements << endl;
    
    cout << "Elements: " << endl;
    for (size_t i = 0; i < elements.size(); ++i) {
        cout << "  " << i << ": (" << elements[i].x << ", " << elements[i].y << ")" << endl;
    }
}