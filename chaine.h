#ifndef CHAINE_H
#define CHAINE_H

#include <vector>
#include <string>
#include "tools.h"  

class Chaine {
private:
    std::vector<S2d> articulations; 
    std::string _mode;
public:
    Chaine();
    int read(int nbArt, std::istream& input, double r_max, double r_capture);
    const std::vector<S2d>& getArticulations() const;
    std::string get_mode() const;
    int set_mode(std::string mode);
    void print() const; // debug feature
};

#endif 