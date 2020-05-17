#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iomanip>
#include "wmradar.h"
#include "../wmapp.h"

using std::sin;
using std::cos;
using std::atan2;

WMRadar::WMRadar(WMTextBar* t, WMSlider* s)
: angle(0), text_score(t), speed(s)
{
  setbuffered(true);
  text_score->settext("     0", false);
  speed->settotal(50, false);
  speed->setvalue(8, false);
}

void
WMRadar::real_display()
{
  if (!p_icon())
    seticon(false);
  setcolor(WMColor(Background));
  fill_rectangle(0, 0, b_width(), b_height()); //clear background

  setcolor(WMColor(Dim));
  for (double i = 0; i < 2 * M_PI; i += M_PI_4)
    draw_line(b_width() / 2, b_height() / 2, (1 + cos(i)) * b_width() / 2,
        (1 - sin(i)) * b_height() / 2); //draw radial lines
  
  draw_arc(b_width() / 3, b_height() / 3, b_width() / 3 - 1, b_height() / 3 - 1,
      0, 360 * 64);
  draw_arc(b_width() / 6, b_height() / 6, 2 * b_width() / 3,
      2 * b_height() / 3, 0, 360 * 64); //draw circles

  setcolor(Color(255, 0, 0));
  for (clist::const_iterator i = collisions.begin(); i != collisions.end(); ++i)
    fill_arc(i->x - 1, i->y - 1, 3, 3, 0, 360 * 64); //draw collision points
  
  std::vector<X::XPoint> points(3);
  for (plist::const_iterator i = planes.begin(); i != planes.end(); ++i)
  {
    setcolor(Color(0, 255 * i->brightness, 0)); // achieve a fading effect
    points[0].x = i->x;
    points[0].y = i->y;
    points[1].x = int(i->x) + 7 * cos(i->direction + 5.5 * M_PI / 6);
    points[1].y = int(i->y) - 7 * sin(i->direction + 5.5 * M_PI / 6);
    points[2].x = int(i->x) + 7 * cos(i->direction + 6.5 * M_PI / 6);
    points[2].y = int(i->y) - 7 * sin(i->direction + 6.5 * M_PI / 6);
    // construct an isoceles triangle pointing in plane's direction of motion
    fill_polygon(points); // and draw it
  }

  setcolor(Color(0, 255, 0));
  draw_line(b_width() / 2, b_height() / 2, (1 + cos(angle)) * b_width() / 2,
      (1 - sin(angle)) * b_height() / 2); //draw radar line

  WMImage::real_display(); //update window
}

double rand(double low, double high)
{
  return low + std::rand() * (high - low) / RAND_MAX;
}

void
WMRadar::add_plane()
{
  if (planes.size() > 4)
    return;

  double theta = rand(0, 2 * M_PI); //put plane somewhere along edge
  plane p = { (1 + cos(theta)) * b_width() / 2,
    (1 - sin(theta)) * b_height() / 2,
    rand(theta + 3 * M_PI_4, theta + 5 * M_PI_4), //direction pointing inwards
    0.5 }; //brightness

  planes.push_back(p);
}

inline double dist2 (double x1, double y1, double x2, double y2)
{ //return distance squared between points
  return std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2);
}

inline double norm_angle(double angle)
{ //return angle shifted into the range [0, 2pi)
  return angle - 2 * M_PI * std::floor(angle / (2 * M_PI));
}

bool
WMRadar::release(int, int x, int y)
{
  if (wPressed)
  {
    wPressed = false;
    if (contains(x, y) && planes.size())
    {
      int newx = x - b_left(),
          newy = y - b_top();
      plist::iterator closest;
      double mindist2 = 10000;
      for (plist::iterator i = planes.begin(); i != planes.end(); ++i)
      {
        double d2 = dist2(i->x, i->y, newx, newy);
        if (d2 < mindist2)
        {
          mindist2 = d2;
          closest = i;
        }
      }
      closest->direction = norm_angle(atan2(double(int(closest->y) - newy),
          double(newx - int(closest->x))));
      //change nearest plane's direction towards (x, y)
    }
    return true;
  }
  else return false;
}

void
WMRadar::increment_time()
{
  for (plist::iterator i = planes.begin(); i != planes.end();)
  { //Collision test planes
    if (!contains(i->x + left(), i->y + top()))
      ++i;
    else
    {
      plist::iterator j = i;
      bool collided = false;
      for (++j; j != planes.end();)
        if (dist2(i->x, i->y, j->x, j->y) <= 0.75)
        { //remove each collided plane and increment score
          j = planes.erase(j);
          collided = true;
          increment_score();
        }
        else
          ++j;
      if (collided)
      {
        increment_score();
        collision c = { i->x, i->y };
        collisions.push_back(c); //make record of collision location
        i = planes.erase(i);
      }
      else
        ++i;
    }
  }
    
  const double angleinc = M_PI / 125;

  for (plist::iterator i = planes.begin(); i != planes.end();)
  {
    double incx = speed->value() * cos(i->direction) / 100;
    double incy = -speed->value() * sin(i->direction) / 100;
    double nextx = i->x + incx;
    double nexty = i->y + incy;
    
    //to check if a plane crossed the radar line, approximate the plane's path
    //as an arc:
    double& l_start_angle = angle; //make an interval of radar line's angles
    const double l_end_angle = l_start_angle + angleinc;

    //make an interval of the plane's angles
    double p_start_angle = norm_angle(atan2(b_height() / 2 - i->y,
          i->x - b_width() / 2));
    double p_end_angle = norm_angle(atan2(b_height() / 2 - nexty,
          nextx - b_width() / 2));

    const double crossp = (b_width() / 2 - i->x) * incy
      - (b_height() / 2 - i->y) * incx; //test whether plane is moving in
      //a positive or a negative angle
    if (crossp < 0.0) //make angle increase counterclockwise
    {
      const double temp = p_start_angle;
      p_start_angle = p_end_angle;
      p_end_angle = temp;
    }
    if (p_end_angle < p_start_angle)
      p_end_angle += 2 * M_PI; //make sure end is > start

    //intersect radar line interval and plane interval. If result is non-empty,
    //there was an intersection.
    if (std::min(p_end_angle, l_end_angle)
        >= std::max(p_start_angle, l_start_angle))
      i->brightness = 1.0; //if plane crossed radar line, make it bright
    else
      i->brightness *= 0.996; //otherwise decay brightness
    

    i->x = nextx; //update plane's position
    i->y = nexty;
    if (!WMWidget::contains(i->x + b_left(), i->y + b_top()))
      i = planes.erase(i); //cull planes outside of radar
    else
      ++i;
  }

  angle = norm_angle(angle + angleinc); //increment radar direction

  display();
}

void
WMRadar::increment_score ()
{
  using namespace std;
  istringstream in(text_score->text()); //get string from textbar
  int score;
  in >> score; //extract score
  ostringstream out;
  out << setw(6) << setfill(' ') << score + 1; //increment and put in a string
  text_score->settext(out.str()); //put back in textbar
}
