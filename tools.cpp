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

    Circle::Circle(S2d center, double radius){
        if(radius>0)
            _radius = radius;
        else    
            _radius = 0;
        _center = center;
    }

    bool Circle::check_intersect(Circle other_circle){
        double dist_c = distance(other_circle._center, _center);
        if(dist_c > abs(other_circle._radius - _radius) && dist_c<other_circle._radius + _radius)
            return true;
        return false;    
    }

    bool Circle::check_inside(Circle circle){
        double dist_c = distance(circle._center, _center);
        if(dist_c + _radius+EPSIL_ZERO < circle._radius)
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
        _length = distance(start_point, end_point);
        if(start_point.x != end_point.x)
            _angle = atan2(end_point.y - start_point.y, end_point.x - start_point.x);
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
        if(angle < M_PI && angle > -M_PI){
            _angle = angle;
            _coordinate_end.x = _coordinate_start.x + _length * cos(_angle);
            _coordinate_end.y = _coordinate_start.y + _length * sin(_angle);
            return 0;
        }
        return -1;
    }

    double Vector::get_length(){
        return _length;
    }

    int Vector::set_length(double length){
        if(length > 0){
            double change = _length - length;
            _length = length;
            _coordinate_end.x -=change * sin(_angle);
            _coordinate_end.y -=change * cos(_angle);
            return 0;
        }
        return -1;
    }


    double Vector::bounce(S2d circle_center) {
        S2d to_end = {_coordinate_end.x - circle_center.x, _coordinate_end.y - circle_center.y};
        double beta = atan2(to_end.y, to_end.x);
        double new_angle = M_PI + 2 * beta - _angle;
        if (new_angle > M_PI) new_angle -= 2 * M_PI;
        else if (new_angle <= -M_PI) new_angle += 2 * M_PI;
        return new_angle;
    }

    int Vector::add(Vector vector){
        _coordinate_end.x += vector._length*cos(vector._angle);
        _coordinate_end.y += vector._length*sin(vector._angle);
        return 0;
    }
    int Vector::multiply_const(double constant){
        _coordinate_end.x = _coordinate_start.x + (_coordinate_end.x - _coordinate_start.x) * constant;
        _coordinate_end.y = _coordinate_start.y + (_coordinate_end.y - _coordinate_start.y) * constant;
        _length *= fabs(constant);
        return 0;
    }
    int Vector::subtract(Vector vector){
        vector = vector*(-1);
        this->add(vector);
        return 0;
    }

    Vector Vector::operator*(double constant){
        Vector res;
        res.set_length(this->_length * constant);
        res.set_angle(this->_angle);
        res._coordinate_end = (S2d){_coordinate_start.x+_length*cos(_angle),_coordinate_start.y+_length*sin(_angle)};
        res._coordinate_start = this->_coordinate_start; 
        return res;
    }

    Vector Vector::operator+(const Vector& vector){
        Vector res;
        res.set_coordinates(_coordinate_start,
            {_coordinate_end.x + vector._length * cos(vector._angle),
             _coordinate_end.y + vector._length * sin(vector._angle)});
        return res;      
    }

    Vector Vector::operator-(const Vector& vector){
        Vector negated;
        negated._coordinate_start = vector._coordinate_start;
        negated._coordinate_end.x = vector._coordinate_start.x - (vector._coordinate_end.x - vector._coordinate_start.x);
        negated._coordinate_end.y = vector._coordinate_start.y - (vector._coordinate_end.y - vector._coordinate_start.y);
        negated._length = distance(negated._coordinate_start, negated._coordinate_end);
        negated._angle = atan2(negated._coordinate_end.y - negated._coordinate_start.y, 
                           negated._coordinate_end.x - negated._coordinate_start.x);
        *this = *this + negated;
        return *this;
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