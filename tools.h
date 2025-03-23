#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <iostream>

constexpr float EPSIL_ZERO = 0.0;

struct S2d {double x=0; double y=0;};

#define ORIGIN S2d{0,0}

double distance(S2d point1, S2d point2);
bool circles_intersect(Circle circle_1, Circle circle_);
Circle* circles_inside(Circle* circle_1, Circle* circle_2); 
// returns a pointer to the inside circle.
// If the circles arent inside each other, returns NULL


class Point{
    public:
        Point::Point();
        virtual int set_center(S2d coordinates);
        virtual S2d get_center();
        double distance_org(); // returns distance from origin
    private:
        S2d _point;
        
};


class Circle: public Point{
    public:
        Circle();
        Circle::Circle(S2d cent, double rad);
        int set_radius(double r);
        double get_radius();
        bool check_inside(Circle circle);
        bool check_intersect(Circle other_circle);
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
        int set_length(double length); 
        double get_length();
        double bounce(S2d circle_center = {0,0});
        int add(Vector v);
        int subtract(Vector v);
        int multiply_const(double c);
        Vector operator*(double constant);
        Vector operator+(const Vector& vector);
        Vector operator-(const Vector& vector);
    private:
        double _angle;
        double _length;
        S2d _coordinate_start;
        S2d _coordinate_end;

};

#endif