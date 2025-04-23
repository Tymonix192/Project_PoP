#include <cmath>
#include <iostream>
#include "graphic.h"
#include "graphic_gui.h"

using namespace std;

static const Cairo::RefPtr<Cairo::Context> *ptcr(nullptr);

// local prototypes
static void set_color(Color color);

void graphic_set_context(const Cairo::RefPtr<Cairo::Context>& cr)
{
    ptcr = &cr;
    (*ptcr)->set_line_width(1.0);
    (*ptcr)->set_antialias(Cairo::ANTIALIAS_GRAY);
}

int graphic_draw_line(double x1, double y1, double x2, double y2, Color color)
{
    if (!ptcr) return -1; 
    (*ptcr)->save();
    set_color(color);
    (*ptcr)->move_to(x1, y1);
    (*ptcr)->line_to(x2, y2);
    (*ptcr)->stroke();
    (*ptcr)->restore();
    return 0;
}

int graphic_draw_circle(double x, double y, double radius, Color color)
{
    if (!ptcr || radius <= 0) return -1;
    (*ptcr)->save();
    set_color(color);
    (*ptcr)->arc(x, y, radius, 0, 2 * M_PI);
    (*ptcr)->fill_preserve(); 
    (*ptcr)->stroke();
    (*ptcr)->restore();
    return 0;
}

int graphic_draw_rectangle(double x, double y, double width, double height, Color color)
{
    if (!ptcr || width <= 0 || height <= 0) return -1; 
    (*ptcr)->save();
    set_color(color);
    double half_width = width / 2.0;
    double half_height = height / 2.0;
    (*ptcr)->rectangle(x - half_width, y - half_height, width, height);
    (*ptcr)->fill_preserve(); 
    (*ptcr)->stroke();
    (*ptcr)->restore();
    return 0;
}

// local function
static void set_color(Color color)
{
    double r(0.), g(0.), b(0.);
    switch (color)
    {
    case WHITE:
        r = g = b = 1;
        break;
    case GREY:
        r = g = b = 0.5;
        break;
    case BLACK:
        r = g = b = 0.;
        break;
    case RED:
        r = 1;
        break;
    case GREEN:
        g = 0.65;
        break;
    case BLUE:
        r = g = 0.65;
        b = 1;
        break;
    case ORANGE:
        r = 1;
        g = 0.65;
        break;
    case PURPLE:
        r = b = 0.65;
        break;
    case CYAN:
        g = b = 1;
        break;
    }
    (*ptcr)->set_source_rgb(r, g, b);
}