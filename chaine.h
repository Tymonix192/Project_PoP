//contributors: 399554 397957
#ifndef CHAINE_H
#define CHAINE_H

#include <vector>
#include <string>
#include "tools.h"  
#include "message.h"

class Chaine {
private:
    std::vector<S2d> articulations; 
    std::string _mode;
    double r_capt;
public:
    Chaine();
    int create_chain(int nbArt, std::vector<S2d> articulaton, double r_max, double r_capture);// -1 dist from origin >= rmax
                                                                                              // -2 dist to previous > r_capture
    const std::vector<S2d>& getArticulations() const;
    std::string get_mode() const;
    int set_mode(std::string mode);

    void draw() const;
    
    void print() const; // debug feature
    bool is_in(Circle arena = {ORIGIN, 0});
    
    // Method to clear all articulations
    void clear();
};

#endif