//contributors: 399554 397957
#include "chaine.h"
#include <iostream>
#include <sstream>


Chaine::Chaine() : _mode("CONSTRUCTION") {
    articulations.clear();
};

int Chaine::create_chain(int nbArt, std::vector<S2d> art, double r_max, 
    double r_capture) {
    articulations.clear();
    r_capt = r_capture;
    if (nbArt != 0) {
        for (int i = 0; i < nbArt; ++i) {
            double distFromOrigin = distance(art[i], ORIGIN);
            if (distFromOrigin >= r_max) 
            {
                std::cout << message::articulation_outside(art[i].x, art[i].y);
                return -1;
            }
            if(i == 0)
            {
                if(distFromOrigin + r_capture + EPSIL_ZERO < r_max){
                    std::cout << message::chaine_racine(art[0].x, art[0].y);
                    return -1;
                }
            }
            if (i > 0) {
                double dist_to_prev = distance(art[i], articulations[i - 1]);
                if (dist_to_prev + EPSIL_ZERO > r_capture) 
                {
                    std::cout << message::chaine_max_distance(i - 1);
                    return -1;
                }
            }
            articulations.push_back(art[i]);
        }
    } 
    return 0;
}

const std::vector<S2d>& Chaine::getArticulations() const {
    return articulations;
}

std::string Chaine::get_mode() const {
    return _mode;
}

int Chaine::set_mode(std::string mode = "CONSTRUCTION") {
    if(mode != "CONSTRUCTION" && mode != "GUIDAGE") return -1;
    _mode = mode;
    return 0;
}

void Chaine::clear() {
    articulations.clear();
}

void Chaine::print() const {
    std::cout << "Chaine (" << articulations.size() << " articulations):\n";
    for (size_t i = 0; i < articulations.size(); ++i) {
        std::cout << "  Articulation " << i << ": (" 
                  << articulations[i].x << ", " << articulations[i].y << ")\n";
    }
    std::cout << "Mode: " << _mode << "\n";
}

bool Chaine::is_in(Circle arena) {
    for (const auto& art : articulations) {
        if (distance(art, arena.get_center()) >= arena.get_radius()) {
            return false;
        }
    }
    return true;
}

void Chaine::draw() const {
    for (size_t i = 0; i < articulations.size(); i++) {
        Point p;
        p.set_center(articulations[i]);
        p.draw(RED);
        
        if (i > 0) {
            Point prev;
            prev.set_center(articulations[i-1]);
            draw_line(prev, p, RED);
        }
    }
    if(articulations.size() > 0) {
        Circle c(articulations[articulations.size()-1], r_capt);
        c.draw_outline(RED);
    }
} 
S2d Chaine::calculateCaptureCenter(const S2d& mousePos, double arenaRadius) const {
    if (articulations.empty()) {
        // No chain yet, project to boundary using Point and Circle classes
        Point mouse;
        mouse.set_center(mousePos);
        
        // Create arena circle
        Circle arena(ORIGIN, arenaRadius);
        
        // Calculate direction from origin to mouse
        Vector direction;
        direction.set_coordinates(ORIGIN, mousePos);
        
        // Normalize the direction and scale to arena radius
        direction.set_length(arenaRadius);
        
        // Return the point on boundary
        return direction.get_end();
    } else {
        // Use effecteur as center
        return articulations.back();
    }
}

S2d Chaine::calculateFinalGoal(double arenaRadius) const {
    if (articulations.empty()) {
        return ORIGIN; // Default if no root exists
    }
    
    // Get the root position
    S2d root = articulations.front();
    
    // For a point directly opposite on a circle, we simply negate the coordinates
    // and normalize to the arena radius
    
    // Create a vector from the origin to the opposite direction
    Vector toGoal;
    // Use negative of root coordinates to go in opposite direction
    toGoal.set_coordinates(ORIGIN, {-root.x, -root.y});
    
    // Scale to arena radius
    toGoal.set_length(arenaRadius);
    
    // Return endpoint on boundary
    return toGoal.get_end();
}

S2d Chaine::calculateIntermediateGoal(const S2d& mousePos, double captureRadius) const {
    if (articulations.size() < 2) {
        return mousePos; // No effecteur yet
    }
    
    // Use Vector to calculate direction from effecteur to mouse
    Vector toMouse;
    toMouse.set_coordinates(articulations.back(), mousePos);
    
    // Limit to capture radius
    if (toMouse.get_length() > captureRadius) {
        toMouse.set_length(captureRadius);
    }
    
    // Return the limited position
    return toMouse.get_end();
}

std::vector<double> Chaine::calculateSegmentLengths() const {
    std::vector<double> lengths;
    for (size_t i = 1; i < articulations.size(); ++i) {
        lengths.push_back(distance(articulations[i-1], articulations[i]));
    }
    return lengths;
}

bool Chaine::checkPositionsInArena(const std::vector<S2d>& positions, 
                                  double arenaRadius) const {
    // Use Point class and distance_org method
    Point point;
    
    // Check all articulations except root (which is always at the boundary)
    for (size_t i = 1; i < positions.size(); ++i) {
        point.set_center(positions[i]);
        if (point.distance_org() >= arenaRadius) {
            return false;
        }
    }
    
    return true;
}

bool Chaine::applyGuidanceAlgorithm(const S2d& goalPos, std::vector<S2d>& newPositions,
                                   double captureRadius) const {
    if (articulations.size() < 2) {
        return false; // Need at least root and effecteur
    }
    
    // Get segment lengths
    std::vector<double> lengths = calculateSegmentLengths();
    
    // First iteration: from effecteur to root (Fig 8a)
    std::vector<S2d> tempPositions = articulations;
    tempPositions.back() = goalPos; // Place effecteur at goal
    
    // Work backwards from effecteur to root
    for (int i = articulations.size() - 2; i >= 0; --i) {
        // Use Vector class for all calculations
        Vector link;
        link.set_coordinates(articulations[i+1], articulations[i]);
        
        // Check for zero length
        if (link.get_length() < EPSIL_ZERO) {
            return false; // Avoid division by zero
        }
        
        // Create new vector from this tempPosition in the same direction
        Vector newLink;
        newLink.set_coordinates(tempPositions[i+1], tempPositions[i+1]); // Start at same point
        newLink.set_angle(link.get_angle());
        newLink.set_length(lengths[i]);
        
        // Set new position
        tempPositions[i] = newLink.get_end();
    }
    
    // Second iteration: from root to effecteur (Fig 8b)
    newPositions.resize(articulations.size());
    newPositions[0] = articulations[0]; // Root position is fixed
    
    // Work forwards from root to effecteur
    for (size_t i = 1; i < articulations.size(); ++i) {
        // Use Vector class for all calculations
        Vector link;
        link.set_coordinates(tempPositions[i-1], tempPositions[i]);
        
        // Check for zero length
        if (link.get_length() < EPSIL_ZERO) {
            return false; // Avoid division by zero
        }
        
        // Create new vector from fixed position in the same direction
        Vector newLink;
        newLink.set_coordinates(newPositions[i-1], newPositions[i-1]); // Start at fixed point
        newLink.set_angle(link.get_angle());
        newLink.set_length(lengths[i-1]);
        
        // Set new position
        newPositions[i] = newLink.get_end();
    }
    
    return checkPositionsInArena(newPositions, captureRadius);
}

bool Chaine::guideTo(const S2d& goalPos, double arenaRadius, double captureRadius) {
    std::vector<S2d> newPositions;
    
    // Apply the guidance algorithm
    if (!applyGuidanceAlgorithm(goalPos, newPositions, arenaRadius)) {
        return false; // Guidance failed
    }
    
    // Create new chain with updated positions
    return create_chain(newPositions.size(), newPositions, arenaRadius, captureRadius) >= 0;
}