//contributors: 399554 397957
#include "mobile.h"
#include "constantes.h"
#include "message.h"
#include "tools.h"
#include <iostream>
#include <sstream>
#include <cmath>
using namespace std;

//Mobile

// Constructors
Mobile::Mobile(const S2d& pos, double alpha, double disp) :
    position(pos), alpha(alpha), displacement(disp) {}

Mobile::Mobile() : position(), alpha(0.0), displacement(0.0) {}

// Accessors
S2d Mobile::getPosition() const {
    return position;
}

double Mobile::getAlpha() const {
    return alpha;
}

double Mobile::getDisplacement() const {
    return displacement;
}

// Mutators
void Mobile::setPosition(const S2d& pos) {
    position = pos;
}

void Mobile::setAlpha(double a) {
    //alpha is within [-π, π]
    while (a > M_PI)
        a -= 2 * M_PI;
    while (a < -M_PI)
        a += 2 * M_PI;
    alpha = a;
}

void Mobile::setDisplacement(double d) {
    displacement = d;
}

// Calculate next position
S2d Mobile::calculateNextPosition() const {
    return {
        position.x + displacement * cos(alpha),
        position.y + displacement * sin(alpha)
    };
}

//Particule

// Constructors
Particule::Particule(const S2d& pos, double alpha, double disp, unsigned int count) :
    Mobile(pos, alpha, disp), counter(count) {}

Particule::Particule() : Mobile(), counter(0) {}

// Accessors
unsigned int Particule::getCounter() const {
    return counter;
}

// Mutators
void Particule::setCounter(unsigned int c) {
    counter = c;
}

void Particule::incrementCounter() {
    counter++;
}

// Check if particle should split
bool Particule::shouldSplit() const {
    return counter >= time_to_split;
}

// Create two new particles from this one
void Particule::createSplitParticles(std::vector<std::unique_ptr<Particule>>& newParticles) const {
    // First new particle
    auto p1 = std::make_unique<Particule>(
        position,
        alpha + delta_split,
        displacement * coef_split
    );
    
    // Second new particle
    auto p2 = std::make_unique<Particule>(
        position,
        alpha - delta_split,
        displacement * coef_split
    );
    
    // Add both to the provided vector
    newParticles.push_back(std::move(p1));
    newParticles.push_back(std::move(p2));
}

// Implementation of move for particles
void Particule::move() {
    S2d nextPos = calculateNextPosition();
    
    // Check if would move outside arena
    double distFromOrigin = distance(nextPos, ORIGIN);
    if (distFromOrigin > r_max) {
        // Calculate bounce
        Vector moveVector;
        moveVector.set_coordinates(position, nextPos);
        double newAlpha = moveVector.bounce();
        
        // Update angle
        setAlpha(newAlpha);
        
        // Recalculate position with new angle
        nextPos = calculateNextPosition();
    }
    
    // Update position
    setPosition(nextPos);
}

// Validation
bool Particule::isInArena() const {
    return distance(position, ORIGIN) <= r_max;
}

bool Particule::isValid() const {
    // Check if the particle is valid
    if (!isInArena()) {
        cout << message::particule_outside(position.x, position.y);
        return false;
    }
    
    // Check displacement bounds
    if (displacement < 0 || displacement > d_max) {
        cout << message::mobile_displacement(displacement);
        return false;
    }
    
    // Check counter bounds
    if (counter >= time_to_split) {
        cout << message::particule_counter(counter);
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

int Particule::draw() const {
    // Draw the particle as a circle
    Point point;
    point.set_center(this->position);

    point.draw();
    
    return 0;
}


// Faiseur

// Constructors
Faiseur::Faiseur() :
    Mobile(), radius(r_min_faiseur), numElements(1) {
    calculateElements();
}

Faiseur::Faiseur(const S2d& pos, double alpha, double disp, double rad, unsigned int nbe) :
    Mobile(pos, alpha, disp), radius(rad), numElements(nbe) {
    calculateElements();
}

// Accessors
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

// Implementation of move for faiseurs
void Faiseur::move() {
    S2d nextPos = calculateNextPosition();
    
    // Check if would move outside arena
    Circle arenaCircle(ORIGIN, r_max);
    Circle faiseurCircle(nextPos, radius);
    
    if (!faiseurCircle.check_inside(arenaCircle)) {
        // Calculate bounce
        Vector moveVector;
        moveVector.set_coordinates(position, nextPos);
        double newAlpha = moveVector.bounce();
        
        // Update angle
        setAlpha(newAlpha);
        
        // Recalculate position with new angle
        nextPos = calculateNextPosition();
    }
    
    // Update position and recalculate elements
    setPosition(nextPos);
    calculateElements();
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
        cout << message::faiseur_nbe(numElements);
        return false;
    }
    // Check radius bounds
    if (radius < r_min_faiseur || radius > r_max_faiseur) {
        cout << message::faiseur_radius(radius);
        return false;
    }
    // Check displacement bounds
    if (displacement < 0 || displacement > d_max) {
        cout << message::mobile_displacement(displacement);
        return false;
    }
    // Check if all elements are inside arena
    if (!isInArena()) {
        cout << message::faiseur_outside(position.x, position.y);
        return false;
    }
    
    return true;
}

// Collision detection
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

bool Faiseur::collidesWithFaiseur(const Faiseur& other, unsigned int thisId, 
                                unsigned int otherId) const {
    for (size_t i = 0; i < elements.size(); ++i) {
        const S2d& elem1 = elements[i];
        Circle circle1(elem1, radius);
        
        for (size_t j = 0; j < other.elements.size(); ++j) {
            const S2d& elem2 = other.elements[j];
            Circle circle2(elem2, other.radius);
            
            if (circles_intersect(circle1, circle2)) {
                cout << message::faiseur_element_collision(thisId, i, otherId, j);
                return true;
            }
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
        cout << "  " << i << ": (" << elements[i].x << ", " << elements[i].y << ")" 
        << endl;
    }
}

int Faiseur::draw() const {
    // Draw the faiseur as a series of circles
    for (const S2d& element : elements) {
        Circle circle(element, radius);
        
        // Draw the circle using graphic library
        circle.draw();
    }
    
    return 0;
}