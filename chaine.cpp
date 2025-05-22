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
        // Check for mouse at origin to avoid division by zero
        double mouseDistance = distance(mousePos, ORIGIN);
        if (mouseDistance < EPSIL_ZERO) {
            return S2d{arenaRadius - EPSIL_ZERO, 0}; // Default to edge of arena
        }
        
        // Project mouse position to arena boundary
        double scale = (arenaRadius - EPSIL_ZERO) / mouseDistance;
        return S2d{mousePos.x * scale, mousePos.y * scale};
    } else {
        // Use the last articulation (effecteur) as center
        return articulations.back();
    }
}

S2d Chaine::calculateFinalGoal(double arenaRadius) const {
    if (articulations.empty()) {
        return ORIGIN; // Default if no root exists
    }
    
    // Get the root position
    const S2d& root = articulations.front();
    double dirX = -root.x;
    double dirY = -root.y;
    
    // Get length of the direction vector
    double length = sqrt(dirX*dirX + dirY*dirY);
    
    // Handle special case where root is at origin (should never happen, but just in case)
    if (length < EPSIL_ZERO) {
        return S2d{arenaRadius, 0}; // Default to point on positive x-axis
    }
    
    // Normalize and scale to arena radius
    double normalizedX = dirX / length;
    double normalizedY = dirY / length;
    
    return S2d{normalizedX * arenaRadius, normalizedY * arenaRadius};
}

S2d Chaine::calculateIntermediateGoal(const S2d& mousePos, double captureRadius) const {
    if (articulations.size() < 2) {
        return mousePos; // No effecteur yet
    }
    
    const S2d& effecteur = articulations.back();
    
    // Calculate direction from effecteur to mouse
    double dx = mousePos.x - effecteur.x;
    double dy = mousePos.y - effecteur.y;
    double distToMouse = sqrt(dx * dx + dy * dy);
    
    // If mouse is very close to effecteur, return mouse position
    if (distToMouse < EPSIL_ZERO) {
        return mousePos;
    }
    
    // Limit to capture radius
    if (distToMouse <= captureRadius) {
        return mousePos;
    } else {
        // Scale to capture radius
        double scale = captureRadius / distToMouse;
        return S2d{effecteur.x + dx * scale, effecteur.y + dy * scale};
    }
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
        // Calculate direction from current temp position to original position
        Vector link;
        link.set_coordinates(tempPositions[i+1], articulations[i]);

        // Check for zero length
        if (link.get_length() < EPSIL_ZERO) {
            return false; // Avoid division by zero
        }

        // Set the new position at correct distance in same direction
        Vector newLink;
        newLink.set_coordinates(tempPositions[i+1], tempPositions[i+1]);
        newLink.set_angle(link.get_angle());
        newLink.set_length(lengths[i]);

        tempPositions[i] = newLink.get_end();
    }

    // Second iteration: from root to effecteur (Fig 8b)
    newPositions.resize(articulations.size());
    newPositions[0] = articulations[0]; // Root position is fixed

    // Work forwards from root to effecteur
    for (size_t i = 1; i < articulations.size(); ++i) {
        // Calculate direction from previous temp position to current temp position
        Vector link;
        link.set_coordinates(tempPositions[i-1], tempPositions[i]);

        // Check for zero length
        if (link.get_length() < EPSIL_ZERO) {
            return false; // Avoid division by zero
        }

        // Set the new position at correct distance in same direction
        Vector newLink;
        newLink.set_coordinates(newPositions[i-1], newPositions[i-1]);
        newLink.set_angle(link.get_angle());
        newLink.set_length(lengths[i-1]);

        newPositions[i] = newLink.get_end();
    }

    return true;
}

bool Chaine::guideTo(const S2d& goalPos, double arenaRadius, double captureRadius) {
    // Check if chain has at least two articulations
    if (articulations.size() < 2) {
        return false;
    }
    
    std::vector<S2d> newPositions;
    
    // Apply the guidance algorithm
    if (!applyGuidanceAlgorithm(goalPos, newPositions, captureRadius)) {
        return false; // Guidance failed
    }
    
    // Check if any articulation (except root) would go outside arena
    for (size_t i = 1; i < newPositions.size(); ++i) {
        if (distance(newPositions[i], ORIGIN) >= arenaRadius - EPSIL_ZERO) {
            return false; // Would move articulation outside
        }
    }
    
    // Update chain with new positions
    articulations = newPositions;
    return true;
}