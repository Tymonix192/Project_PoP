#include <vector>
#include <iostream>

#ifndef s2d
#define s2d

struct S2d {double x=0; double y=0;};

class Point{
    public:
        Point::Point();
        virtual int set_center(S2d coordinates);
        virtual S2d get_center();
        double distance_org();
    private:
        S2d _point;
        
};

class Circle: public Point{
    public:
        Circle::Circle(S2d cent);
        int set_radius(double r);
        double get_radius();
        bool check_inside();
        bool check_cross();
        int set_center(S2d coordinates);
        S2d get_center();
    private:
        double _radius;
        S2d _center;
};

class Vector{
    public:
        int set_coordinates(S2d start, S2d end);
        S2d get_start();
        S2d get_end();
        int set_angle(double angle);
        double get_angle();
        int set_lenght(double lenght); 
        double get_lenght();
    private:
        double _angle;
        double _lenght;
        S2d _coordinate_start;
        S2d _coordinate_end;

};

#endif