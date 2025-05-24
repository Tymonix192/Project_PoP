//contributors: 399554 397957
#include "project.h"
#include <iostream>



int main(int argc, char* argv[]) {
    std::string filename = "";
    
    // Check if a filename was provided as a command-line argument
    if (argc >= 2) {
        filename = argv[1];
    }
    
    auto jeu = std::make_unique<Jeu>();

    return start_gui_application(argc, argv, filename, std::move(jeu));
}