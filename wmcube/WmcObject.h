#ifndef _WMCOBJECT_H_
#define _WMCOBJECT_H_

#include <vector>
#include "wmapp/wmcanvas.h"

typedef struct { float x, y, z; } Vertex;
typedef struct { Vertex *a, *b, *c; float color; int ab, bc, ac; } Plane;
typedef struct { Vertex *a, *b; } Line;
typedef enum { WIREFRAME, SOLID, SOLID_WIRE, NUM_MODES };

class WmcObject
{
public:
  WmcObject();
  WmcObject(const char *);
  ~WmcObject();

  void setMode(int m);
  void setColorShading(unsigned color, float shading);
  void rotate(float xang, float yang, float zang);
  void draw(WMCanvas *icanvas);

  void setYOffset(int pixels);
  void modifyZOffset(int idepth);
  void setLightSource(float xc, float yc, float zc);
  
private:
  
  bool   load(const char *);
  bool   loadVertices(const char *);
  bool   loadPlanes(const char *);
  bool   loadLines(const char *);
  int    freadline(FILE *fp, int maxlen, char *dest);
  void   sort(vector<Plane> &plane);
  bool   visible(Plane p);
  float  luminate(Plane p);
  Vertex normal(Plane p);
  void   drawTriangle(Plane p, bool wire = false);
  bool   verticesBelongToLine(Vertex *first, Vertex *second, Line l);
  
  vector<Vertex> vertex;
  vector<Vertex> rvertex;
  vector<Plane>  plane;
  vector<Line>   line;

  WMCanvas		*canvas;
  Vertex		lightsource;
  bool			lines, planes;  
  int			xoff, yoff, zoff;
  int			mode;
  unsigned int	color;
  float			shading;
};

const char default_cube[] = 
{ ""
"# The original cube by Robert Kling\n"
"WMCUBE_COORDINATES\n"
"1  -180 -180  180\n"
"2   180 -180  180\n"
"3   180  180  180\n"
"4  -180  180  180\n"
"5  -180 -180 -180\n"
"6   180 -180 -180\n"
"7   180  180 -180\n"
"8  -180  180 -180\n"
"WMCUBE_LINES\n"
"1 2\n" 
"2 3\n"
"3 4\n"
"4 1\n"
"5 6\n"
"6 7\n"
"7 8\n"
"8 5\n"
"1 5\n"
"2 6\n"
"3 7\n"
"4 8\n"
"WMCUBE_PLANES\n"
"1 2 3\n"
"1 3 4\n"
"2 6 7\n"
"2 7 3\n"
"5 1 4\n"
"5 4 8\n"
"5 2 1\n"
"5 6 2\n"
"4 3 8\n"
"3 7 8\n"
"6 5 8\n"
"6 8 7"
};

#endif
