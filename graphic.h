//contributors: 399554 397957
#ifndef GRAPHIC_H
#define GRAPHIC_H

enum Color
{
    WHITE,
    GREY,
    BLACK,
    RED,
    GREEN,
    BLUE,
    ORANGE,
    PURPLE,
    CYAN
};

// Draw a line from (x1, y1) to (x2, y2) with specified color
int graphic_draw_line(double x1, double y1, double x2, double y2, Color color);

// Draw a circle centered at (x, y) with given radius and color
int graphic_draw_circle(double x, double y, double radius, Color color);

// Draw a rectangle centered at (x, y) with given width, height, and color
int graphic_draw_rectangle(double x, double y, double width, double height, Color color);

// Draw a circle outline centered at (x, y) with given radius and color
int graphic_draw_circle_outline(double x, double y, double radius, Color color);

#endif