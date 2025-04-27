//contributors: 399554 397957
#ifndef JEU_H
#define JEU_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "chaine.h"
#include "mobile.h"
#include "constantes.h"
#include "message.h"
#include "tools.h"

// Define Status enum outside of the class so it can be used as a return type
enum Status {
    ONGOING,
    WON,
    LOST,
    ERROR = -1
};

/** 
 * This class is responsible for:
 * - Reading configuration files
 * - Managing game entities (particles, makers, chain)
 * - Managing the overall game state (score, mode, etc.)
 */
class Jeu {
private:
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

    //attributs
    unsigned int score;           
    // Polymorphic collection
    std::vector<std::unique_ptr<Mobile>> mobiles;
    // Indices of particles in the mobiles vector
    std::vector<size_t> particuleIndices;          
    // Indices of makers in the mobiles vector
    std::vector<size_t> faiseurIndices;
    Chaine chaine;   
    std::string lastLoadedFile;   
    Status status;                

    
    // Helper methods for entity access
    Particule* getParticule(size_t index);
    Faiseur* getFaiseur(size_t index) const;
    std::vector<Faiseur*> getAllFaiseurs();
    
    // File reading helpers
    bool readNextLine(std::ifstream& file, std::string& line);
    int handleScoreState(const std::string& line, ReadState& nextState); 
    int handleParticleCountState(const std::string& line, ReadState& nextState);  
    int handleParticleDataState(const std::string& line, unsigned int& particleIndex,  
                               unsigned int totalParticles, ReadState& nextState);
    int handleFaiseurCountState(const std::string& line, ReadState& nextState);  
    int handleFaiseurDataState(const std::string& line, unsigned int& faiseurIndex, 
                              unsigned int totalFaiseurs, ReadState& nextState);
    int handleArticulationCountState(const std::string& line, ReadState& nextState);  
    int handleArticulationDataState(const std::string& line, 
                                std::vector<S2d>& articulations,  
                                   unsigned int& articulationIndex, 
                                   unsigned int totalArticulations,
                                   ReadState& nextState);
    int handleModeState(const std::string& line, const std::vector<S2d>& articulations, 
                        int totalArticulations, ReadState& nextState);
                         
    // Update helpers
    void updateParticules();
    void updateFaiseurs();
    void removeMarkedEntities(const std::vector<size_t>& indicesToRemove, 
        std::vector<size_t>& entityIndices);
    
    // Data management
    void clearGameData();
    
    // Collision detection methods
    bool checkArticulationFaiseurCollision(const S2d& articulation, 
                                         unsigned int articulationIndex) const;
    bool checkChainFaiseurCollisions();
    
public:
    // Constructor
    Jeu();
    
    // File operations
    bool lecture(const std::string& filename);
    bool saveToFile(const std::string& filename);
    bool restart();
    bool update();
    
    // Accessors
    unsigned int getScore() const;
    size_t getNbParticules() const;
    size_t getNbFaiseurs() const;
    size_t getNbArticulations() const;
    std::string getMode() const;
    Status getStatus() const;
    
    // GUI related methods
    bool save(const std::string& filename);
    bool set_jeu(const std::string& filename);
    void clear();
    void draw() const;
    void set_mode(Mode mode);
    void setStatus(Status newStatus);
    unsigned int get_score() const;
    size_t get_particle_count() const;
    size_t get_faiseur_count() const; 
    size_t get_articulation_count() const;
};

#endif