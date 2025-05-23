//contributors: 399554 397957
#include "jeu.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

// Constructor initializes the game state
Jeu::Jeu() : score(0), lastLoadedFile(""), status(ONGOING) {
    // All other containers are initialized by their default constructors
}

// Helper methods for type-safe entity access
Particule* Jeu::getParticule(size_t index) const{
    if (index >= particuleIndices.size()) return nullptr;
    return static_cast<Particule*>(mobiles[particuleIndices[index]].get());
}

Faiseur* Jeu::getFaiseur(size_t index) const {
    if (index >= faiseurIndices.size()) return nullptr;
    return static_cast<Faiseur*>(mobiles[faiseurIndices[index]].get());
}

std::vector<Faiseur*> Jeu::getAllFaiseurs() {
    std::vector<Faiseur*> result;
    for (size_t i = 0; i < faiseurIndices.size(); ++i) {
        Faiseur* f = getFaiseur(i);
        if (f) result.push_back(f);
    }
    return result;
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
    std::unique_ptr<Particule> particle(new Particule(S2d{x, y}, 
        alpha, displacement, counter));

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
    std::unique_ptr<Faiseur> maker(new Faiseur(S2d{x, y}, alpha, displacement, 
        radius, nbe));
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

    S2d point = {x, y};
    // Check if articulation is inside arena
    if (distance(point, ORIGIN) >= r_max) {
        std::cout << message::articulation_outside(point.x, point.y);
        return -1;
    }
    
    // Check root articulation is close to boundary
    if (articulationIndex == 0) {
        double distFromOrigin = distance(point, ORIGIN);
        if(distFromOrigin + r_capture + EPSIL_ZERO < r_max){
            std::cout << message::chaine_racine(point.x, point.y);
            return -1;
        }
    }
    
    // Validate distance between consecutive articulations
    if (articulationIndex > 0) {
        double dist = distance(point, articulations[articulationIndex-1]);
        if (dist + EPSIL_ZERO > r_capture) {
            std::cout << message::chaine_max_distance(articulationIndex-1);
            return -1;
        }
    }
    // Check for collisions with makers
    if (checkArticulationFaiseurCollision(point, articulationIndex)) {
        return -1;
    }
    articulations.push_back(point);
    // Check if we've read all articulations
    articulationIndex++;
    if (articulationIndex >= totalArticulations) {
        nextState = READ_MODE;
    }
    return 0;
}

int Jeu::handleModeState(const std::string& line, 
    const std::vector<S2d>& articulations, int totalArticulations, 
    ReadState& nextState) {
    
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

void Jeu::clearGameData() {
    // Clear all game data structures
    mobiles.clear();
    particuleIndices.clear();
    faiseurIndices.clear();
    // The chaine class handles its own data clearing
}

bool Jeu::lecture(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    
    clearGameData();
    
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
            clearGameData();
            return false;
        }
        // Handle the current state
        switch (state) {
            case READ_SCORE:
                result = handleScoreState(line, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                break;
                
            case READ_PARTICULE_COUNT:
                result = handleParticleCountState(line, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                nbPart = result;
                particleIndex = 0;
                break;
                
            case READ_PARTICULE_DATA:
                result = handleParticleDataState(line, particleIndex, nbPart, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                break;
                
            case READ_FAISEUR_COUNT:
                result = handleFaiseurCountState(line, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                nbFais = result;
                faiseurIndex = 0;
                break;
                
            case READ_FAISEUR_DATA:
                result = handleFaiseurDataState(line, faiseurIndex, nbFais, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                break;
                
            case READ_ARTICULATION_COUNT:
                result = handleArticulationCountState(line, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                nbArt = result;
                articulationIndex = 0;
                articulations.clear();
                break;
                
            case READ_ARTICULATION_DATA:
                result = handleArticulationDataState(line, articulations, 
                                            articulationIndex, nbArt, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                break;
                
            case READ_MODE:
                result = handleModeState(line, articulations, nbArt, state);
                if (result < 0) {
                    clearGameData();
                    return false;
                }
                break;
                
            default:
                std::cerr << "Invalid state in file reading state machine" 
                            << std::endl;
                clearGameData();
                return false;
        }
    }
    
    // Save the filename for potential restart
    lastLoadedFile = filename;
    
    // Success message
    std::cout << message::success();
    return true;
}

bool Jeu::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file for writing: " << filename << std::endl;
        return false;
    }
    
    // Write score
    file << score << std::endl;

    // Write particles
    file << particuleIndices.size() << std::endl;
    for (size_t i = 0; i < particuleIndices.size(); ++i) {
        Particule* p = getParticule(i);
        file << p->getPosition().x << " " 
             << p->getPosition().y << " "
             << p->getAlpha() << " " 
             << p->getDisplacement() << " "
             << p->getCounter() << std::endl;
    }
    
    // Write makers (faiseurs)
    file << faiseurIndices.size() << std::endl;
    for (size_t i = 0; i < faiseurIndices.size(); ++i) {
        Faiseur* f = getFaiseur(i);
        file << f->getPosition().x << " " 
             << f->getPosition().y << " "
             << f->getAlpha() << " " 
             << f->getDisplacement() << " "
             << f->getRadius() << " " 
             << f->getNumElements() << std::endl;
    }
    
    // Write chain articulations
    const std::vector<S2d>& articulations = chaine.getArticulations();
    file << articulations.size() << std::endl;
    for (const S2d& art : articulations) {
        file << art.x << " " << art.y << std::endl;
    }
    
    // Write mode
    file << chaine.get_mode() << std::endl;
    
    return true;
}

bool Jeu::restart() {
    if (lastLoadedFile.empty()) {
        std::cerr << "No previous file loaded to restart" << std::endl;
        return false;
    }
    return lecture(lastLoadedFile);
}

//Collision check
bool Jeu::checkArticulationFaiseurCollision(const S2d& articulation,
    unsigned int articulationIndex) const {
    // Check each faiseur for collision with this articulation
    for (size_t i = 0; i < faiseurIndices.size(); ++i) {
        const Faiseur* faiseur = getFaiseur(i);
        if (!faiseur) continue;

        if (faiseur->collidesWithPoint(articulation)) {
            // report the collision with the correct element index
            // for rendu2 assuming collision with first element
            std::cout << message::chaine_articulation_collision(
            articulationIndex, i, 0); 
            return true;
        }
    }
    return false;
}

bool Jeu::checkChainFaiseurCollisions() {
    // If no articulations, no collision possible
    const std::vector<S2d>& articulations = chaine.getArticulations();
    if (articulations.empty()) {
        return false;
    }

    // Check each articulation against all faiseurs
    for (size_t i = 0; i < articulations.size(); ++i) {
        if (checkArticulationFaiseurCollision(articulations[i], i)) {
            chaine.clear();
            return true;
        }
    }
    return false;
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

// Update

bool Jeu::update() {
    // Check if game is already over
    if (status != ONGOING) {
        return false;
    }
    
    // Decrement score
    if (score > 0) {
        score--;
    }
    
    // Update all entities
    updateEntities();
    
    // Handle chain-related updates
    updateChain();
    
    // Check game end conditions
    checkGameEndConditions();
    
    return (status == ONGOING);
}

void Jeu::updateParticules() {
    std::vector<size_t> particlesToRemove;
    std::vector<std::unique_ptr<Particule>> newParticles;
    
    for (size_t i = 0; i < particuleIndices.size(); ++i) {
        Particule* p = getParticule(i);
        if (!p) continue;
        
        // Increment counter
        p->incrementCounter();
        
        if (p->shouldSplit()) {
            if (particuleIndices.size() + newParticles.size() >= nb_particule_max) {
                // Maximum particles reached - destroy this particle
                particlesToRemove.push_back(i);
            } else {
                // Create two new particles from the split
                p->createSplitParticles(newParticles);
                
                // Mark original particle for removal
                particlesToRemove.push_back(i);
            }
        }
    }
    
    // Move existing particles
    for (size_t i = 0; i < particuleIndices.size(); ++i) {
        // Skip particles marked for removal
        if (std::find(particlesToRemove.begin(), 
        particlesToRemove.end(), i) != particlesToRemove.end()) {
            continue;
        }
        
        Particule* p = getParticule(i);
        if (p) p->move();
    }
    
    // Move and add new particles
    for (auto& newParticle : newParticles) {
        newParticle->move(); // Move the new particle
        
        // Add to game state
        particuleIndices.push_back(mobiles.size());
        mobiles.push_back(std::move(newParticle));
    }
    
    // Remove particles marked for deletion
    removeMarkedEntities(particlesToRemove, particuleIndices);
}

void Jeu::updateFaiseurs() {
    for (size_t i = 0; i < faiseurIndices.size(); ++i) {
        Faiseur* f = getFaiseur(i);
        if (!f) continue;
        
        // Calculate next position without changing the faiseur
        S2d nextPos = f->calculateNextPosition();
        bool canMove = true;
        
        // Check collision with other faiseurs
        for (size_t j = 0; j < faiseurIndices.size(); ++j) {
            if (i == j) continue; // Skip self
            
            Faiseur* otherF = getFaiseur(j);
            if (!otherF) continue;
            
            // Check if the hypothetical move would cause a collision
            if (f->collidesWithFaiseur(*otherF, i, j, &nextPos)) {
                canMove = false;
                break;
            }
        }
        
        // If no collisions would occur, move the faiseur
        if (canMove) {
            f->move();
        }
    }
}

void Jeu::updateEntities() {
    updateParticules();
    updateFaiseurs();
}

void Jeu::updateChain() {
    if (chaine.getArticulations().empty()) {
        return;
    }
    
    // Check for collisions between chain and faiseurs
    if (checkChainFaiseurCollisions()) {
        return; // Chain was destroyed
    }
    
    // Apply guidance if in guidance mode
    applyChainGuidance();
    
    // Check for collisions again after guidance
    checkChainFaiseurCollisions();
}

void Jeu::applyChainGuidance() {
    if (chaine.get_mode() != "GUIDAGE" || chaine.getArticulations().size() < 2) {
        return;
    }
    
    // Use the stored mouse position for consistency
    S2d intermediateGoal = chaine.calculateIntermediateGoal(mousePosition, r_capture);
    
    // Try to apply guidance - if it fails, just continue
    chaine.guideTo(intermediateGoal, r_max, r_capture);
}

void Jeu::checkGameEndConditions() {
    checkWinCondition();
    
    if (score <= 0) {
        checkLossCondition();
    }
}

bool Jeu::isParticleCapturable(size_t particleIndex, const S2d& captureCenter) const {
    Particule* particle = getParticule(particleIndex);
    if (!particle) {
        return false;
    }
    
    double distToCapture = distance(particle->getPosition(), captureCenter);
    return distToCapture <= r_capture - EPSIL_ZERO;
}

bool Jeu::validateRootPlacement(const S2d& root) const {
    double distFromOrigin = distance(root, ORIGIN);
    
    // Root should be close to arena boundary but inside
    return (distFromOrigin + r_capture + EPSIL_ZERO >= r_max) && 
           (distFromOrigin < r_max);
}

bool Jeu::validateArticulationDistances(const std::vector<S2d>& articulations) const {
    for (size_t i = 1; i < articulations.size(); ++i) {
        double dist = distance(articulations[i-1], articulations[i]);
        if (dist > r_capture + EPSIL_ZERO) {
            return false;
        }
    }
    return true;
}

bool Jeu::validateChainCollisions(const std::vector<S2d>& articulations) const {
    for (size_t artIdx = 0; artIdx < articulations.size(); ++artIdx) {
        if (checkArticulationFaiseurCollision(articulations[artIdx], artIdx)) {
            return false;
        }
    }
    return true;
}

bool Jeu::validateChainCreation(const std::vector<S2d>& newArticulations) const {
    if (newArticulations.empty()) {
        return false;
    }
    
    // Validate root placement
    if (!validateRootPlacement(newArticulations[0])) {
        return false;
    }
    
    // Validate distances between articulations
    if (!validateArticulationDistances(newArticulations)) {
        return false;
    }
    
    // Check for collisions with faiseurs
    return validateChainCollisions(newArticulations);
}

void Jeu::removeMarkedEntities(const std::vector<size_t>& indicesToRemove, 
                              std::vector<size_t>& entityIndices) {
    // Sort indices in descending order to avoid index shifting issues
    std::vector<size_t> sortedIndices = indicesToRemove;
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<size_t>());
    
    for (size_t i = 0; i < sortedIndices.size(); ++i) {
        size_t idx = sortedIndices[i];
        if (idx >= entityIndices.size()) continue;
        
        size_t mobileIdx = entityIndices[idx];
        
        // Remove from entityIndices
        entityIndices.erase(entityIndices.begin() + idx);
        
        // Update all indices greater than the removed one
        for (size_t j = 0; j < particuleIndices.size(); ++j) {
            if (particuleIndices[j] > mobileIdx) {
                particuleIndices[j]--;
            }
        }
        
        for (size_t j = 0; j < faiseurIndices.size(); ++j) {
            if (faiseurIndices[j] > mobileIdx) {
                faiseurIndices[j]--;
            }
        }
        
        // Remove the mobile
        if (mobileIdx < mobiles.size()) {
            mobiles.erase(mobiles.begin() + mobileIdx);
        }
    }
}


void Jeu::handle_mouse_move(const S2d& mousePos) {
    // Store the mouse position
    mousePosition = mousePos;
    
    // Validate mouse position is within reasonable bounds
    double distFromCenter = distance(mousePos, ORIGIN);
    if (distFromCenter > r_max * 2) {
        // Mouse is too far from arena, don't update visual elements
        showCaptureRegion = false;
        showGoal = false;
        return;
    }
    
    // Always update capture center for visual feedback
    captureCenter = chaine.calculateCaptureCenter(mousePos, r_max);
    showCaptureRegion = true;
    
    // Update goals if chain exists
    if (!chaine.getArticulations().empty()) {
        finalGoal = chaine.calculateFinalGoal(r_max);
        showGoal = true;
        
        // Only calculate intermediate goal if we have an effecteur
        if (chaine.getArticulations().size() >= 2) {
            intermediateGoal = chaine.calculateIntermediateGoal(mousePos, r_capture);
        }
    } else {
        showGoal = false;
    }
}

void Jeu::handle_right_click(const S2d& clickPos) {
    // Switch to guidance mode
    chaine.set_mode("GUIDAGE");
}

void Jeu::handle_left_click(const S2d& clickPos) {
    // Switch to construction mode
    chaine.set_mode("CONSTRUCTION");
    
    // Update mouse position
    mousePosition = clickPos;
    
    // Use chain's capture center calculation for visual feedback
    captureCenter = chaine.calculateCaptureCenter(clickPos, r_max);
    showCaptureRegion = true;
    
    // Attempt particle capture
    tryParticleCapture();
    
    // Update visual state
    if (!chaine.getArticulations().empty()) {
        finalGoal = chaine.calculateFinalGoal(r_max);
        showGoal = true;
    }
}


bool Jeu::findCaptureCandidate(size_t& particleIndex) const {
    if (particuleIndices.empty()) {
        return false;
    }

    S2d captureCenter = chaine.calculateCaptureCenter(mousePosition, r_max);
    std::vector<size_t> candidates;
    
    for (size_t i = 0; i < particuleIndices.size(); ++i) {
        if (isParticleCapturable(i, captureCenter)) {
            candidates.push_back(i);
        }
    }
    
    if (candidates.size() == 1) {
        particleIndex = candidates[0];
        return true;
    }
    
    return false;
}

bool Jeu::tryParticleCapture() {
    if (particuleIndices.empty()) {
        return false;
    }
    
    size_t particleIndex;
    if (!findCaptureCandidate(particleIndex)) {
        return false;
    }
    
    Particule* particle = getParticule(particleIndex);
    if (!particle) {
        return false;
    }
    
    // Create new articulation list
    const std::vector<S2d>& currentArticulations = chaine.getArticulations();
    std::vector<S2d> newArticulations = currentArticulations;
    newArticulations.push_back(particle->getPosition());
    
    // Validate and create chain
    if (!validateChainCreation(newArticulations)) {
        return false;
    }
    
    int result = chaine.create_chain(newArticulations.size(), newArticulations, 
                                   r_max, r_capture);
    if (result < 0) {
        return false;
    }
    
    // Remove captured particle
    std::vector<size_t> toRemove = {particleIndex};
    removeMarkedEntities(toRemove, particuleIndices);
    
    // Update visual feedback for first articulation
    if (currentArticulations.empty()) {
        finalGoal = chaine.calculateFinalGoal(r_max);
        showGoal = true;
    }
    
    return true;
}

bool Jeu::checkWinCondition() {
    const std::vector<S2d>& articulations = chaine.getArticulations();
    if (articulations.size() < 2) {
        return false; // Need at least root and effecteur
    }
    
    // Get effecteur and final goal
    S2d effecteur = articulations.back();
    S2d goal = chaine.calculateFinalGoal(r_max);
    
    // Check if effecteur is within capture radius of goal
    double distanceTogoal = distance(effecteur, goal);
    if (distanceTogoal <= r_capture) {
        std::string winMessage = std::string("Congratulations! Chaine has reached the goal!\n") +
        std::string("Final score: ") + std::to_string(score) + std::string("\n") +
        std::string("Distance between chaine tip and goal: ") + std::to_string(distanceTogoal) +
        std::string("\n") +
        std::string("Capture radius: ") + std::to_string(r_capture) + std::string("\n");

        endGame(WON, winMessage);
        return true;
    }
    
    return false;
}

void Jeu::checkLossCondition() {
    if (score <= 0) {
        endGame(LOST, "Game over! Your final score is 0.");
    }
}

void Jeu::endGame(Status newStatus, const std::string& message) {
    status = newStatus;
    score = (newStatus == LOST) ? 0 : score; // Set score to 0 for loss
    std::cout << message << std::endl;
}


//GUI related methods
Status Jeu::getStatus() const {
    return this->status;
}

void Jeu::drawGameElements() const {
    // Draw arena
    Circle arena(ORIGIN, r_max);
    arena.draw_outline(Color::GREEN);
    
    // Draw all mobiles
    for (const auto& mobile : mobiles) {
        mobile->draw();
    }
    
    // Draw chain
    chaine.draw();
}

void Jeu::drawCaptureMechanism(const S2d& captureCenter, bool showCaptureRegion) const {
    if (showCaptureRegion) {
        Circle captureCircle(captureCenter, r_capture);
        captureCircle.draw_outline(Color::RED);
    }
}

void Jeu::drawGoals(const S2d& finalGoal, const S2d& intermediateGoal, bool showGoal) const {
    if (showGoal && !chaine.getArticulations().empty()) {
        // Draw final goal as a black point
        Point goalPoint;
        goalPoint.set_center(finalGoal);
        goalPoint.draw(Color::BLACK);
    }
}

// Main draw method now calls the helper functions
void Jeu::draw() const {
    drawGameElements();
    drawCaptureMechanism(captureCenter, showCaptureRegion);
    drawGoals(finalGoal, intermediateGoal, showGoal);
}

void Jeu::clear()
{
    // Clear all game data structures
    mobiles.clear();
    particuleIndices.clear();
    faiseurIndices.clear();
    chaine.clear();
    score = 0;
}


bool Jeu::set_jeu(const string& file_name) {
    if (!lecture(file_name)) {
        clear();
        return false;
    }
    return true;
}

void Jeu::set_mode(Mode mode) {
    if (mode == CONSTRUCTION) {
        chaine.set_mode("CONSTRUCTION");
    } else if (mode == GUIDAGE) {
        chaine.set_mode("GUIDAGE");
    }
}

unsigned int Jeu::get_score() const {
    return score;
}

size_t Jeu::get_articulation_count() const {
    return chaine.getArticulations().size();
}

size_t Jeu::get_particle_count() const {
    return particuleIndices.size();
}

size_t Jeu::get_faiseur_count() const {
    return faiseurIndices.size();
}

bool Jeu::save(const std::string& filename) {
    return saveToFile(filename);
}

void Jeu::setStatus(Status newStatus) {
    status = newStatus;
}
