//contributors: 399554 397957
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

// Helper methods for type-safe entity access
Particule* Jeu::getParticule(size_t index) {
    if (index >= particuleIndices.size()) return nullptr;
    return static_cast<Particule*>(mobiles[particuleIndices[index]].get());
}

Faiseur* Jeu::getFaiseur(size_t index) {
    if (index >= faiseurIndices.size()) return nullptr;
    return static_cast<Faiseur*>(mobiles[faiseurIndices[index]].get());
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
    
    // Create particle
    std::unique_ptr<Particule> particle(new Particule(S2d{x, y}, alpha, displacement, counter));

    if (!particle->isValid()) {
        return -1;
    }
    
    // Add valid particle and store its index
    particuleIndices.push_back(mobiles.size());
    mobiles.push_back(std::move(particle));
    
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
    
    // Create maker
    std::unique_ptr<Faiseur> maker(new Faiseur (S2d{x, y}, alpha, displacement, radius, nbe));
    if (!maker->isValid()) {
        return -1;
    }
    
    // Check for collisions with existing makers
    for (size_t j = 0; j < faiseurIndices.size(); ++j) {
        Faiseur* existingMaker = getFaiseur(j);
        if (maker->collidesWithFaiseur(*existingMaker, faiseurIndex, j)) {
            return -1;
        }
    }
    
    // Add valid maker and store its index
    faiseurIndices.push_back(mobiles.size());
    mobiles.push_back(std::move(maker));
    
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

int Jeu::handleArticulationDataState(const std::string& line,
                                std::vector<S2d>& articulations,
                                   unsigned int& articulationIndex, 
                                   unsigned int totalArticulations,
                                   ReadState& nextState) {
    double x, y;
    std::istringstream iss(line);
    
    if (!(iss >> x >> y)) {
        std::cerr << "Error parsing articulation data" << std::endl;
        return -1;
    }
    
    // Use the tools module to check if the point is inside the arena
    S2d point = {x, y};
    articulations.push_back(point);
    
    // Validate distance between consecutive articulations
    if (articulationIndex > 0) {
        double dist = distance(articulations[articulationIndex], 
                                articulations[articulationIndex-1]);
        if (dist > r_capture) {
            std::cout << message::chaine_max_distance(articulationIndex-1);
            return -1;
        }
    }
    
    // Check for collisions with makers
    for (size_t j = 0; j < faiseurIndices.size(); ++j) {
        Faiseur* maker = getFaiseur(j);
        if (maker->collidesWithPoint(point)) {
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
                        int totalArticulations, ReadState& nextState) {
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
        int result = chaine.create_chain(totalArticulations, articulations, 
                                        r_max, r_capture);
        if(result < 0) return -1;
        chaine.set_mode(mode);
    }
    
    nextState = READ_COMPLETE;
    return 0;
}

bool Jeu::lecture(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    
    // Clear existing data
    mobiles.clear();
    particuleIndices.clear();
    faiseurIndices.clear();
    
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
            return false;
        }
        // Handle the current state
        switch (state) {
            case READ_SCORE:
                result = handleScoreState(line, state);
                if (result < 0) return false;
                break;
                
            case READ_PARTICULE_COUNT:
                result = handleParticleCountState(line, state);
                if (result < 0) return false;
                nbPart = result;
                particleIndex = 0;
                break;
                
            case READ_PARTICULE_DATA:
                result = handleParticleDataState(line, particleIndex, nbPart, state);
                if (result < 0) return false;
                break;
                
            case READ_FAISEUR_COUNT:
                result = handleFaiseurCountState(line, state);
                if (result < 0) return false;
                nbFais = result;
                faiseurIndex = 0;
                break;
                
            case READ_FAISEUR_DATA:
                result = handleFaiseurDataState(line, faiseurIndex, nbFais, state);
                if (result < 0) return false;
                break;
                
            case READ_ARTICULATION_COUNT:
                result = handleArticulationCountState(line, state);
                if (result < 0) return false;
                nbArt = result;
                articulationIndex = 0;
                articulations.clear();
                break;
                
            case READ_ARTICULATION_DATA:
                result = handleArticulationDataState(line, articulations, 
                                            articulationIndex, nbArt, state);
                if (result < 0) return false;
                break;
                
            case READ_MODE:
                result = handleModeState(line, articulations, nbArt, state);
                if (result < 0) return false;
                break;
                
            default:
                std::cerr << "Invalid state in file reading state machine" 
                            << std::endl;
                return false;
        }
    }
    
    // Success message
    std::cout << message::success();
    return true;
}

unsigned int Jeu::getScore() const {
    return score;
}

size_t Jeu::getNbParticules() const {
    return particuleIndices.size();
}

size_t Jeu::getNbFaiseurs() const {
    return faiseurIndices.size();
}

size_t Jeu::getNbArticulations() const {
    return chaine.getArticulations().size();
}

std::string Jeu::getMode() const {
    return chaine.get_mode();
}