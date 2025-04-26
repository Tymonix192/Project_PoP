#include "chaine.h"
#include <iostream>
#include <sstream>


Chaine::Chaine() : _mode("CONSTRUCTION") {
    articulations.clear();
};

int Chaine::create_chain(int nbArt, std::vector<S2d> art, double r_max, double r_capture) {
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
    Circle c(articulations[articulations.size()-1], r_capt);
    c.draw_outline(RED);
} 