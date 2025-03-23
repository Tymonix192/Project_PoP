#ifndef CHAINE_H
#define CHAINE_H

#include <vector>
#include <string>
#include "tools.h" 

enum mode{
    CONSTRUCTION,
    GUIDAGE
};   

class Chaine {
private:
    std::vector<S2d> articulations; 
    std::string mode;
public:
    Chaine();
    int read(int nbArt, std::istream& input, double r_max, double r_capture);
    const std::vector<S2d>& getArticulations() const;
    std::string getMode() const;
    int setMode();
    void print() const; // debug feature
};

#endif 