#include "tools.h"
#include <cmath>

double distance(S2d point1, S2d point2){
    return sqrt(pow((point1.x - point2.x), 2) + pow((point1.y - point2.y), 2));
}

int Point::set_center(S2d coordinates){
    _point = coordinates;
    return 0;
}

S2d Point::get_center(){
    return _point;
}

double Point::distance_org(){
    return sqrt(_point.x*_point.x + _point.y*_point.y);
}

Circle::Circle(S2d center){
    _center = center;
}

bool Circle::check_intersect(Circle other_circle){

}

bool Circle::check_inside(Circle circle){
    //TODO 2
}

double Circle::get_radius(){
    return _radius;
}

int Circle::set_radius(double radius){
    if(radius>0){
        _radius = radius;
        return 0;
    }
    return -1;
}

S2d Circle::get_center(){
    return _center;
}

int Circle::set_center(S2d center_coordinates){
    _center = center_coordinates;
    return 0;
}

int Vector::set_coordinates(S2d start_point, S2d end_point){
    _coordinate_start = start_point;
    _coordinate_end = end_point;
    return 0;
};

S2d Vector::get_start(){
    return _coordinate_start;
}

S2d Vector::get_end(){
    return _coordinate_end;
}

double Vector::get_angle(){
    return _angle;
}

int Vector::set_angle(double angle){
    if(angle<M_PI && angle>-M_PI){
        _angle = angle;
    }
}

double Vector::get_lenght(){
    return _lenght;
}

int Vector::set_lenght(double lenght){
    if(lenght > 0){
        double change = _lenght - lenght;
        _lenght = lenght;
        _coordinate_end.x -=change * sin(_angle);
        _coordinate_end.x -=change * cos(_angle);
        return 0;
    }
    return -1;
}


double Vector::bounce(){
    double new_angle;

    //TODO 3

    return new_angle;
}
