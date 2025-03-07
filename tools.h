#include <vector>
#include <iostream>

#ifndef s2d
#define s2d

struct S2d {double x=0; double y=0;};

class Point{
    public:
        Point::Point();
        int set_point(S2d coordinates);
        S2d get_point();
        double distance_org();
    private:
        S2d point;
        
};

class Circle: public Point{
    public:
        Circle::Circle();
        bool check_cross();
        bool check_inside();
        int set_radius(double r);
        double get_radius();
        int set_center(S2d coordinates);
        S2d get_center();
    private:
        double radius;
};

#endif