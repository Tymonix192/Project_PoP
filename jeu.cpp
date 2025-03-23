#include "jeu.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>

// Constructor initializes the game state
Jeu::Jeu() : score(0) {
    // All other containers are initialized by their default constructors
}


bool Jeu::readNextLine(std::ifstream& file, std::string& line) {
    while (getline(file >> std::ws, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        return true;
    }
    return false;
}

int Jeu::handleScoreState(const std::string& line, ReadState& nextState) {
    std::istringstream iss(line);
    
    if (!(iss >> score)) {
        std::cerr << "Error parsing score" << std::endl;
        return -1;
    }
    
    // Validate score
    if (score <= 0 || score > score_max) {
        std::cout << message::score_outside(score);
        return -1;
    }
    
    nextState = READ_PARTICULE_COUNT;
    return 0;
}

int Jeu::handleParticleCountState(const std::string& line, ReadState& nextState) {
    unsigned int nbPart;
    std::istringstream iss(line);
    
    if (!(iss >> nbPart)) {
        std::cerr << "Error parsing particle count" << std::endl;
        return -1;
    }
    
    // Validate number of particles
    if (nbPart > nb_particule_max) {
        std::cout << message::nb_particule_outside(nbPart);
        return -1;
    }
    
    // If no particles, skip to next state
    if (nbPart == 0) {
        nextState = READ_FAISEUR_COUNT;
    } else {
        nextState = READ_PARTICULE_DATA;
    }
    
    return nbPart; // Return count as a positive number
}

int Jeu::handleParticleDataState(const std::string& line, unsigned int& particleIndex, 
                               unsigned int totalParticles, ReadState& nextState) {
    double x, y, alpha, displacement;
    unsigned int counter;
    std::istringstream iss(line);
    
    if (!(iss >> x >> y >> alpha >> displacement >> counter)) {
        std::cerr << "Error parsing particle data" << std::endl;
        return -1;
    }
    
    // Create particle and use its validation methods
    Particule p({x, y}, alpha, displacement, counter);

    int validation = p.isValid(); //look at mobile.h
    if (validation < 0) {
        // The specific error message depends on what failed
        if (validation == -2) {
            std::cout << message::mobile_displacement(displacement);
            return -1;
        }
        
        if (validation == -3) {
            std::cout << message::particule_counter(counter);
            return -1;
        }
        if (validation == -1) {
        std::cout << message::particule_outside(x, y);
        return -1;
    }
        
        // Generic error if validation fails for another reason
        return -1;
    }
    
    // Add valid particle
    particules.push_back(p);
    
    // Check if we've read all particles
    particleIndex++;
    if (particleIndex >= totalParticles) {
        nextState = READ_FAISEUR_COUNT;
    }
    
    return 0;
}


int Jeu::handleFaiseurCountState(const std::string& line, ReadState& nextState) {
    unsigned int nbFais;
    std::istringstream iss(line);
    
    if (!(iss >> nbFais)) {
        std::cerr << "Error parsing maker count" << std::endl;
        return -1;
    }
    
    // If no makers, skip to next state
    if (nbFais == 0) {
        nextState = READ_ARTICULATION_COUNT;
    } else {
        nextState = READ_FAISEUR_DATA;
    }
    
    return nbFais; // Return count as a positive number
}

int Jeu::handleFaiseurDataState(const std::string& line, unsigned int& faiseurIndex, 
                               unsigned int totalFaiseurs, ReadState& nextState) {
        double x, y, alpha, displacement, radius;
    unsigned int nbe;
    std::istringstream iss(line);
    
    if (!(iss >> x >> y >> alpha >> displacement >> radius >> nbe)) {
        std::cerr << "Error parsing maker data" << std::endl;
        return -1;
    }
    
    // Create maker and use its validation methods
    Faiseur f({x, y}, alpha, displacement, radius, nbe);
    int validation = f.isValid();
    if (!f.isValid()) { //look at mobile.h
        // The specific error message depends on what failed
        if (validation == -4) {
            std::cout << message::faiseur_radius(radius);
            return -1;
        }
        if (validation == -2) {
            std::cout << message::mobile_displacement(displacement);
            return -1;
        }
        if (validation == -3) {
            std::cout << message::faiseur_nbe(nbe);
            return -1;
        }
        if (validation == -1) {
            std::cout << message::faiseur_outside(x, y);
            return -1;
        }
        
        // Generic error if validation fails for another reason
        return -1;
    }
    
    // Check for collisions with existing makers
    for (size_t j = 0; j < faiseurs.size(); ++j) {
        if (f.collidesWithFaiseur(faiseurs[j])) {
            std::cout << message::faiseur_element_collision(faiseurIndex, 0, j, 0);
            return -1;
        }
    }
    
    // Add valid maker
    faiseurs.push_back(f);
    // Check if we've read all makers
    faiseurIndex++;
    if (faiseurIndex >= totalFaiseurs) {
        nextState = READ_ARTICULATION_COUNT;
    }
    
    return 0;
}

int Jeu::handleArticulationCountState(const std::string& line, ReadState& nextState) {
    unsigned int nbArt;
    std::istringstream iss(line);
    
    if (!(iss >> nbArt)) {
        std::cerr << "Error parsing articulation count" << std::endl;
        return -1;
    }
    
    // If no articulations, skip to next state
    if (nbArt == 0) {
        nextState = READ_MODE;
    } else {
        nextState = READ_ARTICULATION_DATA;
    }
    
    return nbArt; // Return count as a positive number
}

int Jeu::handleArticulationDataState(const std::string& line, std::vector<S2d>& articulations,
                                   unsigned int& articulationIndex, unsigned int totalArticulations,
                                   ReadState& nextState) {
    double x, y;
    std::istringstream iss(line);
    
    if (!(iss >> x >> y)) {
        std::cerr << "Error parsing articulation data" << std::endl;
        return -1;
    }
    
    // Use the tools module to check if the point is inside the arena
    S2d point = {x, y};
    if (distance(point, ORIGIN) >= r_max) {
        std::cout << message::articulation_outside(x, y);
        return -1;
    }
    
    articulations.push_back(point);
    
    // Validate distance between consecutive articulations
    if (articulationIndex > 0) {
        double dist = distance(articulations[articulationIndex], articulations[articulationIndex-1]);
        if (dist > r_capture) {
            std::cout << message::chaine_max_distance(articulationIndex-1);
            return -1;
        }
    }
    
    // Check for collisions with makers
    for (size_t j = 0; j < faiseurs.size(); ++j) {
        if (faiseurs[j].collidesWithPoint(point)) {
            std::cout << message::chaine_articulation_collision(articulationIndex, j, 0);
            return -1;
        }
    }
    
    // Check if we've read all articulations
    articulationIndex++;
    if (articulationIndex >= totalArticulations) {
        nextState = READ_MODE;
    }
    
    return 0;
}

int Jeu::handleModeState(const std::string& line, const std::vector<S2d>& articulations,
                       unsigned int totalArticulations, ReadState& nextState) {
    std::string mode;
    std::istringstream iss(line);
    
    if (!(iss >> mode)) {
        std::cerr << "Error parsing mode" << std::endl;
        return -1;
    }
    
    // Validate mode
    if (mode != "CONSTRUCTION" && mode != "GUIDAGE") {
        std::cerr << "Invalid mode: " << mode << std::endl;
        return -1;
    }
    
    // Create the chain with articulations and mode
    if (totalArticulations > 0) {
        std::ifstream dummyFile; // Using a dummy file since we already parsed the articulations
        int result = chaine.read(totalArticulations, dummyFile, r_max, r_capture);
        if (result < 0) {
            return -1;
        }
        chaine.set_mode(mode);
    }
    
    nextState = READ_COMPLETE;
    return 0;
}

int Jeu::lecture(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return -1;
    }
    
    // Clear existing data
    particules.clear();
    faiseurs.clear();
    
    ReadState state = READ_SCORE;
    std::string line;
    unsigned int nbPart = 0, nbFais = 0, nbArt = 0;
    unsigned int particleIndex = 0, faiseurIndex = 0, articulationIndex = 0;
    std::vector<S2d> articulations;
    int result;
    
    // Main state machine loop
    while (state != READ_COMPLETE) {
        // Read the next significant line
        if (!readNextLine(file, line)) {
            std::cerr << "Unexpected end of file in state " << state << std::endl;
            return -1;
        }
        // Handle the current state
        switch (state) {
            case READ_SCORE:
                result = handleScoreState(line, state);
                if (result < 0) return -1;
                break;
                
            case READ_PARTICULE_COUNT:
                result = handleParticleCountState(line, state);
                if (result < 0) return -1;
                nbPart = result;
                particleIndex = 0;
                break;
                
            case READ_PARTICULE_DATA:
                result = handleParticleDataState(line, particleIndex, nbPart, state);
                if (result < 0) return -1;
                break;
                
            case READ_FAISEUR_COUNT:
                result = handleFaiseurCountState(line, state);
                if (result < 0) return -1;
                nbFais = result;
                faiseurIndex = 0;
                break;
                
            case READ_FAISEUR_DATA:
                result = handleFaiseurDataState(line, faiseurIndex, nbFais, state);
                if (result < 0) return -1;
                break;
                
            case READ_ARTICULATION_COUNT:
                result = handleArticulationCountState(line, state);
                if (result < 0) return -1;
                nbArt = result;
                articulationIndex = 0;
                articulations.clear();
                break;
                
            case READ_ARTICULATION_DATA:
                result = handleArticulationDataState(line, articulations, articulationIndex, nbArt, state);
                if (result < 0) return -1;
                break;
                
            case READ_MODE:
                result = handleModeState(line, articulations, nbArt, state);
                if (result < 0) return -1;
                break;
                
            default:
                std::cerr << "Invalid state in file reading state machine" << std::endl;
                return -1;
        }
    }
    
    // Success message
    std::cout << message::success();
    return 0;
}

unsigned int Jeu::getScore() const {
    return score;
}

size_t Jeu::getNbParticules() const {
    return particules.size();
}

size_t Jeu::getNbFaiseurs() const {
    return faiseurs.size();
}

size_t Jeu::getNbArticulations() const {
    return chaine.getArticulations().size();
}

std::string Jeu::getMode() const {
    return chaine.get_mode();
}

