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
    
    // Handle special case where root is at origin 
    if (length < EPSIL_ZERO) {
        return S2d{arenaRadius, 0};
    }
    
    // Normalize and scale to arena radius
    double normalizedX = dirX / length;
    double normalizedY = dirY / length;
    
    return S2d{normalizedX * arenaRadius, normalizedY * arenaRadius};
}

S2d Chaine::calculateIntermediateGoal(const S2d& mousePos, 
    double captureRadius) const {
    if (articulations.size() < 2) {
        return mousePos;
    }
    
    const S2d& effecteur = articulations.back();
    double distToMouse = distance(effecteur, mousePos);
    
    if (distToMouse < EPSIL_ZERO) {
        return mousePos;
    }
    
    if (distToMouse <= captureRadius) {
        return mousePos;
    }
    
    return normalizeToLength(effecteur, mousePos, captureRadius);
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

bool Chaine::isGoalReachable(const S2d& goalPos) const {
    if (articulations.empty()) {
        return false;
    }
    
    double totalLength = calculateTotalLength();
    double distToGoal = distance(articulations[0], goalPos);
    return distToGoal <= totalLength;
}

// Helper: Calculate total chain length
double Chaine::calculateTotalLength() const {
    double total = 0;
    for (size_t i = 1; i < articulations.size(); ++i) {
        total += distance(articulations[i-1], articulations[i]);
    }
    return total;
}

// Helper: Handle unreachable goal case - uses tools functions
bool Chaine::handleUnreachableGoal(const S2d& goalPos, 
    std::vector<S2d>& newPositions) const {
    S2d direction = calculateDirection(articulations[0], goalPos);
    std::vector<double> lengths = calculateSegmentLengths();
    
    newPositions[0] = articulations[0]; // Root stays fixed
    S2d currentPos = articulations[0];
    
    for (size_t i = 1; i < articulations.size(); ++i) {
        currentPos.x += direction.x * lengths[i-1];
        currentPos.y += direction.y * lengths[i-1];
        newPositions[i] = currentPos;
    }
    return true;
}

// Helper: Perform backward pass 
void Chaine::performBackwardPass(const S2d& goalPos, std::vector<S2d>& positions, 
                                const std::vector<double>& lengths) const {
    positions[positions.size() - 1] = goalPos;
    
    for (int i = static_cast<int>(positions.size()) - 2; i >= 0; --i) {
        S2d direction = calculateDirection(positions[i + 1], positions[i]);
        positions[i].x = positions[i + 1].x + direction.x * lengths[i];
        positions[i].y = positions[i + 1].y + direction.y * lengths[i];
    }
}

// Helper: Perform forward pass - uses tools functions
void Chaine::performForwardPass(std::vector<S2d>& positions, 
                               const std::vector<double>& lengths) const {
    positions[0] = articulations[0];
    
    for (size_t i = 1; i < positions.size(); ++i) {
        S2d direction = calculateDirection(positions[i - 1], positions[i]);
        positions[i].x = positions[i - 1].x + direction.x * lengths[i - 1];
        positions[i].y = positions[i - 1].y + direction.y * lengths[i - 1];
    }
}

// Helper: Check convergence
bool Chaine::hasConverged(const std::vector<S2d>& positions, 
                         const S2d& goalPos, double tolerance) const {
    double distToGoal = distance(positions.back(), goalPos);
    return distToGoal < tolerance;
}

// Main guidance algorithm 
bool Chaine::applyGuidanceAlgorithm(const S2d& goalPos, 
    std::vector<S2d>& newPositions, double captureRadius) const {
    
    if (articulations.size() < 2) {
        return false;
    }

    std::vector<double> lengths = calculateSegmentLengths();
    newPositions = articulations;
    
    // Check if goal is reachable - chain-specific logic
    if (!isGoalReachable(goalPos)) {
        return handleUnreachableGoal(goalPos, newPositions);
    }
    
    // Iterative FABRIK algorithm
    const int maxIterations = 10;
    const double tolerance = 0.1;
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        performBackwardPass(goalPos, newPositions, lengths);
        performForwardPass(newPositions, lengths);
        
        if (hasConverged(newPositions, goalPos, tolerance)) {
            break;
        }
    }
    
    return true;
}

bool Chaine::guideTo(const S2d& goalPos, double arenaRadius, double captureRadius) {
    if (articulations.size() < 2) {
        return false;
    }
    
    std::vector<S2d> newPositions;
    
    if (!applyGuidanceAlgorithm(goalPos, newPositions, captureRadius)) {
        return false;
    }
    
    if (!checkPositionsInArena(newPositions, arenaRadius)) {
        return false;
    }
    
    articulations = newPositions;
    return true;
}