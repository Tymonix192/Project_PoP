#include "chaine.h"
#include <iostream>
#include <sstream>


Chaine::Chaine() : _mode("CONSTRUCTION") {
    articulations.clear();
};

int Chaine::create_chain(int nbArt, std::vector<S2d> art, double r_max, double r_capture) {
    articulations.clear();
    if (nbArt != 0) {
        for (int i = 0; i < nbArt; ++i) {
            double x, y;
            double distFromOrigin = distance(art[i], ORIGIN);
            if (distFromOrigin >= r_max) return -1; 
            if (i > 0) {
                double dist_to_prev = distance(art[i], articulations[i - 1]);
                if (dist_to_prev > r_capture) return -2; 
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
int Chaine::set_mode(std::string mode = "CONSTRUCTION"){
    if(mode != "CONSTRUCTION" && mode != "GUIDAGE") return -1;
    _mode = mode;
    return 0;
}


void Chaine::print() const {
    std::cout << "Chaine (" << articulations.size() << " articulations):\n";
    for (size_t i = 0; i < articulations.size(); ++i) {
        std::cout << "  Articulation " << i << ": (" 
                  << articulations[i].x << ", " << articulations[i].y << ")\n";
    }
    std::cout << "Mode: " << _mode << "\n";
}