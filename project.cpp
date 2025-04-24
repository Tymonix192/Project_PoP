#include "project.h"
#include <iostream>



int main(int argc, char* argv[]) {
    // Check if a filename was provided as a command-line argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <test_file.txt>" << std::endl;
        return 1;
    }

    // Get the filename from the command-line argument
    std::string filename = argv[1];
 
    auto jeu = std::make_unique<Jeu>();
    int result = jeu->lecture(filename);
    std::cout<<"here"<<std::endl;

    return start_gui_application(argc, argv, filename, std::move(jeu));
}