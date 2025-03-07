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
        double dist_c = distance(other_circle._center, _center);
        if(dist_c > (other_circle._radius - _radius) && dist_c<other_circle._radius + _radius)
            return true;
        return false;    
    }

    bool Circle::check_inside(Circle circle){
        double dist_c = distance(circle._center, _center);
        if(dist_c < circle._radius - _radius)
            return true;
        return false;  
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
        _lenght = distance(start_point, end_point);
        if(start_point.x != end_point.x)
            _angle = tan((start_point.y-end_point.y)/(start_point.x - end_point.x));
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

    int Vector::add(Vector vector){
        _coordinate_end.x += vector._lenght*cos(vector._angle);
        _coordinate_end.y += vector._lenght*sin(vector._angle);
        return 0;
    }
    int Vector::multiply(double constant){
        _coordinate_end.x *= constant;
        _coordinate_end.y *= constant;
        return 0;
    }
    int Vector::subctract(Vector vector){
        vector = vector*(-1);
        this->add(vector);
        return 0;
    }

    Vector Vector::operator*(double constant){
        Vector res;
        res.set_lenght(this->_lenght * constant);
        res.set_angle(this->_angle);
        return res;
    }

    Vector Vector::operator+(const Vector& vector){
        Vector res;
        res.set_coordinates(this->_coordinate_start,
             {this->_coordinate_end.x + vector._lenght*cos(vector._angle),this->_coordinate_end.y + vector._lenght*sin(vector._angle) } );
            
        return res;       
    }

    Vector Vector::operator-(const auto& vector){
        vector *= -1;
        return this + vector;
    }

    Circle* circles_inside(Circle* circle_1, Circle* circle_2){
        if(circle_1->check_inside(*circle_2))
            return circle_1;
        if(circle_2->check_inside(*circle_1))
            return circle_2;
        else
            return NULL;
    }

    bool circles_intersect(Circle circle_1, Circle circle_2){
        if(circle_1.check_intersect(circle_2))
            return true;
        return false;
    }