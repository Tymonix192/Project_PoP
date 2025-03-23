#ifndef JEU_H
#define JEU_H

#include <string>
#include <vector>
#include <iostream>
#include "chaine.h"
#include "mobile.h"
#include "constantes.h"
#include "message.h"
#include "tools.h"

/** 
 * This class is responsible for:
 * - Reading configuration files
 * - Managing game entities (particles, makers, chain)
 * - Detecting collisions between entities
 * - Managing the overall game state (score, mode, etc.)
 */
class Jeu {
private:
    //attributs
    unsigned int score;                 
    std::vector<Particule> particules;   
    std::vector<Faiseur> faiseurs;      
    Chaine chaine;                      
        // States for the file reading state machine
    enum ReadState {
        READ_SCORE,
        READ_PARTICULE_COUNT,
        READ_PARTICULE_DATA,
        READ_FAISEUR_COUNT,
        READ_FAISEUR_DATA,
        READ_ARTICULATION_COUNT,
        READ_ARTICULATION_DATA,
        READ_MODE,
        READ_COMPLETE
    };
    //Methods
    // Validates any remaining global constraints
    bool validateGlobalConstraints() const;
    // Reads next non-empty, non-comment line
    bool readNextLine(std::ifstream& file, std::string& line);
    // Processes the score line
    int handleScoreState(const std::string& line, ReadState& nextState); 
    // Processes particle count line
    int handleParticleCountState(const std::string& line, ReadState& nextState);  
    // Processes a particle data line
    int handleParticleDataState(const std::string& line, unsigned int& particleIndex,  
                               unsigned int totalParticles, ReadState& nextState);
    // Processes maker count line
    int handleFaiseurCountState(const std::string& line, ReadState& nextState);  
     // Processes a maker data line
    int handleFaiseurDataState(const std::string& line, unsigned int& faiseurIndex, 
                              unsigned int totalFaiseurs, ReadState& nextState);
    // Processes articulation count line
    int handleArticulationCountState(const std::string& line, ReadState& nextState);  
    // Processes an articulation data line
    int handleArticulationDataState(const std::string& line, std::vector<S2d>& articulations,  
                                   unsigned int& articulationIndex, unsigned int totalArticulations,
                                   ReadState& nextState);
    // Processes the game mode line
    int handleModeState(const std::string& line, const std::vector<S2d>& articulations, 
                       unsigned int totalArticulations, ReadState& nextState); 
public:
    //constructor
    Jeu();


    int lecture(const std::string& filename);
    
    //accessors
    unsigned int getScore() const;
    size_t getNbParticules() const;
    size_t getNbFaiseurs() const;
    size_t getNbArticulations() const;
    std::string getMode() const;
};


#endif