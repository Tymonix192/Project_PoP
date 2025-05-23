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
    // Helper methods for chain guidance algorithm
    std::vector<double> calculateSegmentLengths() const;
    bool checkPositionsInArena(const std::vector<S2d>& positions, 
        double arenaRadius) const;
    bool applyGuidanceAlgorithm(const S2d& goalPos, std::vector<S2d>& newPositions, 
        double captureRadius) const;
    double calculateTotalLength() const;
    bool isGoalReachable(const S2d& goalPos) const;
    bool handleUnreachableGoal(const S2d& goalPos, 
        std::vector<S2d>& newPositions) const;
    void performBackwardPass(const S2d& goalPos, std::vector<S2d>& positions, 
        const std::vector<double>& lengths) const;
    void performForwardPass(std::vector<S2d>& positions, 
        const std::vector<double>& lengths) const;
    bool hasConverged(const std::vector<S2d>& positions, 
        const S2d& goalPos, double tolerance) const;
public:
    Chaine();
    int create_chain(int nbArt, std::vector<S2d> articulaton, double r_max, 
        double r_capture);
        // -1 dist from origin >= rmax
        // -2 dist to previous > r_capture
    const std::vector<S2d>& getArticulations() const;
    std::string get_mode() const;
    int set_mode(std::string mode);

    void draw() const;
    
    void print() const; // debug feature
    bool is_in(Circle arena = {ORIGIN, 0});
    
    S2d calculateCaptureCenter(const S2d& mousePos, double arenaRadius) const;
    S2d calculateFinalGoal(double arenaRadius) const;
    S2d calculateIntermediateGoal(const S2d& mousePos, double captureRadius) const;
    bool guideTo(const S2d& goalPos, double arenaRadius, double captureRadius);
    // Method to clear all articulations
    void clear();
};

#endif