#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "wmapp/wmcanvas.h"
#include "WmcObject.h"

using namespace std;

#define DEBUG    0
#define PDEBUG() if (DEBUG)

#define CHECK(a, b, c)     (((b) < (a)) || ((b) > (c))) ? true : false
#define ROUND(a)           ((a) - floor((a)) >= 0.5) ? (int)ceil((a)) : (int)floor((a))
#define PI                 3.14159265368979
#define PI2                6.283185307
#define invPI              0.318309885
#define WMC_COMMENT_CHAR   '#'
#define VERTEX_SCALE       200.0

/**********************************
 Color related stuff
*/

#define REFLEX       0.04 // Do the reflexy thing if the shading is less than this
#define REFLEX_COLOR 0xAACEEE
#define WIRE_COLOR   0x20B2AE
#define CYAN         0x20B2AE
#define SHADING      90.0

/*******************************************************************

 WmcObject::WmcObject()
 
 Creates a new (default)

********************************************************************/
WmcObject::WmcObject()
{
  const char *filename = { "/tmp/default_cube.wmc" };
  FILE *fp = fopen(filename, "w");
  fwrite(default_cube, sizeof(unsigned char), strlen(default_cube), fp);
  fclose(fp);
  	
  load(filename);

  lightsource.x = 0;
  lightsource.y = -1;
  lightsource.z = 1;

  xoff = 26;
  yoff = 20;
  zoff = 3000;

  color = CYAN;
  shading = SHADING / 100.0;
}

/*******************************************************************

 WmcObject::WmcObject(const char *filename)

********************************************************************/
WmcObject::WmcObject(const char *filename)
{
  load(filename);

  lightsource.x = 0;
  lightsource.y = -1;
  lightsource.z = 1;

  xoff = 26;
  yoff = 20;
  zoff = 3000;

  color = CYAN;
  shading = SHADING / 100.0;
}

/*******************************************************************

 WmcObject::~WmcObject()

********************************************************************/
WmcObject::~WmcObject()
{
}

void WmcObject::setColorShading(unsigned icolor, float ishading)
{
	color = icolor;
	shading = ishading / 100.0;;
}

void WmcObject::setMode(int m)
{
  PDEBUG() printf("setting mode %d\n", m);
  switch (m)
    {
    case WIREFRAME:
      PDEBUG() printf("setting wireframe mode\n", m);
      if (lines) mode = WIREFRAME;
      break;
    case SOLID:
      PDEBUG() printf("setting solid mode\n", m);
      if (planes) mode = SOLID;
      break;
    case SOLID_WIRE:
      PDEBUG() printf("setting solid wire mode\n", m);
      if (planes && lines) mode = SOLID_WIRE;
      break;
    default:
      break;
    }
}

void WmcObject::setYOffset(int pixels)
{
  yoff = pixels;
}

void WmcObject::modifyZOffset(int idepth)
{
  if (zoff + idepth >= 2 * VERTEX_SCALE) 
    zoff += idepth;
}

void WmcObject::setLightSource(float xc, float yc, float zc)
{
  lightsource.x = xc;
  lightsource.y = yc;
  lightsource.z = zc;
}


/*******************************************************************

 int WmcObject::freadline(FILE *fp, int maxlen, char *dest)

 Reads a line of chars of maximum 'maxlen' length from file 'fp' 
 into buffer 'dest'. Discards comments (returns length 0) otherwise
 return the true length of the row.

********************************************************************/
int WmcObject::freadline(FILE *fp, int maxlen, char *dest)
{
  int cnt = 0;
  bzero(dest, maxlen);
  if (feof(fp)) return 0;
  fread(&dest[cnt++], 1, 1, fp);
  while ((dest[cnt - 1] != '\n') && (!feof(fp)) && (cnt < maxlen)) fread(&dest[cnt++], 1, 1, fp);
  dest[cnt - 1] = 0;
  if (dest[0] == WMC_COMMENT_CHAR) return 0;
  return cnt - 1;
}

/*******************************************************************
 
 bool WmcObject::loadVertices(const char *filename)

********************************************************************/
bool WmcObject::loadVertices(const char *filename)
{
  Vertex vtmp;
  int n = 0;
  char tmp[64], tmp2[64];
  FILE *fp;
  float max = 0;
  
  PDEBUG() printf("Loading vertices...\n");

  if ((fp = fopen(filename,"r")) == NULL)
    throw runtime_error("WmcObject::load: object file not found.");
    
  do
    {
      freadline(fp, 63, tmp);
      if (feof(fp)) return false;
    }
  while (!strstr(tmp,"WMCUBE_COORDINATES"));

  do
    {
      int len = freadline(fp, 63, tmp);
      //printf("\"%s\"\n",tmp);

      if (!strstr(tmp,"WMCUBE_PLANES") && !strstr(tmp,"WMCUBE_LINES") && (len > 5))
	{
	  if (sizeof(float) == sizeof(double))
	    sscanf(tmp,"%63s %lf %lf %lf", tmp2, &vtmp.x, &vtmp.y, &vtmp.z);
	  else
	    sscanf(tmp,"%63s %f %f %f", tmp2, &vtmp.x, &vtmp.y, &vtmp.z);

	  n = atoi(tmp2);

	  // Save maximum scalar product for scaling
	  if (sqrt(vtmp.x*vtmp.x + vtmp.y*vtmp.y + vtmp.z*vtmp.z) > max)
	    max = sqrt(vtmp.x * vtmp.x + vtmp.y*vtmp.y + vtmp.z*vtmp.z);

	  vertex.push_back(vtmp);
	  //printf("%d: %f %f %f\n", n, vertex.back().x, vertex.back().y, vertex.back().z);

	  if (n != vertex.size())
	    {
	      fclose(fp);
	      throw runtime_error("WmcObject::load: Error in objectfile: the"
				  "coordinates must be listed in order 1..n.");
	    }
	}
    }
	
  while (!strstr(tmp,"WMCUBE_LINES") && !strstr(tmp,"WMCUBE_PLANES") && (!feof(fp)));

  // Scale the vertices to 0..200
  for (int i = 0; i < vertex.size(); i++)
    {
      vertex.at(i).x = VERTEX_SCALE / max * vertex.at(i).x;
      vertex.at(i).y = VERTEX_SCALE / max * vertex.at(i).y;
      vertex.at(i).z = VERTEX_SCALE / max * vertex.at(i).z;
      rvertex.push_back(vertex.at(i));
    }

  fclose(fp);

  return true;
}

/*******************************************************************

 bool WmcObject::loadLines(const char *filename)

********************************************************************/
bool WmcObject::loadLines(const char *filename)
{
  Line ltmp;
  char tmp[64];
  int iltmp[3];
  FILE *fp;

  if ((fp = fopen(filename,"r")) == NULL)
    throw runtime_error("WmcObject::load: object file not found.\n");

  do
    {
      freadline(fp, 63, tmp);
      if (feof(fp)) return false;
    }
  while (!strstr(tmp,"WMCUBE_LINES"));

  PDEBUG() printf("Loading lines...\n");

  do
    {
      int len = freadline(fp, 63, tmp);

      if (!strstr(tmp,"WMCUBE_COORDINATES") && !strstr(tmp,"WMCUBE_PLANES") && (len > 2))
	{
	  sscanf(tmp, "%d %d", &iltmp[0], &iltmp[1]);
	  
	  if (CHECK(0, iltmp[0] - 1, vertex.size()) || CHECK(0, iltmp[1] - 1, vertex.size()))
	    { 
	      fclose(fp);
	      throw runtime_error("WmcObject::load: Error in objectfile (WMCUBE_LINES section):"
				  "invalid coordinates.");
	    }
	  
	  ltmp.a = &rvertex[iltmp[0] - 1];
	  ltmp.b = &rvertex[iltmp[1] - 1];
	  line.push_back(ltmp);
	  //printf("%d %d\n",  iltmp[0], iltmp[1]);
	}
    }
  while (!strstr(tmp,"WMCUBE_COORDINATES") && !strstr(tmp,"WMCUBE_PLANES") && !feof(fp));

  fclose(fp);

  return true;
}

/*******************************************************************

 bool WmcObject::loadPlanes(const char *filename)

********************************************************************/
bool WmcObject::loadPlanes(const char *filename)
{
  Plane ptmp;
  char tmp[64];
  int iptmp[3];
  FILE *fp;

  if ((fp = fopen(filename,"r")) == NULL)
    throw runtime_error("WmcObject::load: object file not found.");

  do
    {
      freadline(fp, 63, tmp);
      if (feof(fp)) return false;
    }
  while (!strstr(tmp,"WMCUBE_PLANES"));

  PDEBUG() printf("Loading planes...\n");

  do
    {
      int len = freadline(fp, 63, tmp);

      if (!strstr(tmp,"WMCUBE_COORDINATES") && !strstr(tmp,"WMCUBE_LINES") && (len > 4))
	{
	  sscanf(tmp,"%d %d %d", &iptmp[0], &iptmp[1], &iptmp[2]);

	  if (CHECK(0,    iptmp[0] - 1, rvertex.size())
	      || CHECK(0, iptmp[1] - 1, rvertex.size())
	      || CHECK(0, iptmp[2] - 1, rvertex.size())) 
	    { 
	      fclose(fp);
	      throw runtime_error("WmcObject::load: Error in objectfile (WMCUBE_PLANES section): "
				  "invalid coordinates.");
	    }
	  
	  ptmp.a = &rvertex[iptmp[0] - 1];
	  ptmp.b = &rvertex[iptmp[1] - 1];
	  ptmp.c = &rvertex[iptmp[2] - 1];
	  ptmp.ab = -1;
	  ptmp.bc = -1;
	  ptmp.ac = -1;
	  plane.push_back(ptmp);
	  //printf("%d %d %d\n", iptmp[0], iptmp[1], iptmp[2]); fflush(stdout);
	}
    }
  while (!strstr(tmp,"WMCUBE_LINES") && !strstr(tmp,"WMCUBE_COORDINATES") && !feof(fp));

  fclose(fp);

  if (lines)
    {
      // Check which lines to draw for SOLID_WIRE mode

      for (int i = 0; i < plane.size(); i++)
	for (int j = 0; j < line.size(); j++)
	  {
	    if (verticesBelongToLine(plane[i].a, plane[i].b, line[j])) plane[i].ab = j;
	    if (verticesBelongToLine(plane[i].b, plane[i].c, line[j])) plane[i].bc = j;
	    if (verticesBelongToLine(plane[i].a, plane[i].c, line[j])) plane[i].ac = j;
	  }
    }
 
  return true;
}

bool WmcObject::verticesBelongToLine(Vertex *first, Vertex *second, Line l)
{
  if ((first == l.a && second == l.b) || (second == l.a && first == l.b))
    return true;
  
  return false;
}

/*******************************************************************

 bool WmcObject::load(const char *filename)

********************************************************************/
bool WmcObject::load(const char *filename)
{
  try 
    {
      loadVertices(filename);
      lines  = loadLines(filename);
      planes = loadPlanes(filename);
    }
  catch (exception &e)
    {
      cout << e.what() << endl; 
    }

  if (!lines && !planes)
    throw runtime_error("WmcObject::load: no WMCUBE_PLANES or WMCUBE_LINES section found in file.");

  if (planes && lines)
  	mode = SOLID_WIRE;
  else if (planes)
  	mode = SOLID;
  else
  	mode = WIREFRAME;
  
  return true;
}

/*******************************************************************
 
 void WmcObject::rotate(int ixang, int iyang, int izang)

 Rotates the object from it's current configuration by ixang, iyang
 and izang degrees.

********************************************************************/
void WmcObject::rotate(float ixang, float iyang, float izang)
{
  static float xang = 0;
  static float yang = 0;
  static float zang = 0;
  float tx, ty, tz;

  xang = (xang + ixang > PI2) ? xang + ixang - PI2 : xang + ixang;
  yang = (yang + iyang > PI2) ? yang + iyang - PI2 : yang + iyang;
  zang = (zang + izang > PI2) ? zang + izang - PI2 : zang + izang;
	
  for (int i = 0; i < vertex.size(); i++)
    {
      tx = cos(yang) * vertex[i].x - sin(yang) * vertex[i].z;
      tz = sin(yang) * vertex[i].x + cos(yang) * vertex[i].z;
      ty = cos(zang) * vertex[i].y - sin(zang) * tx;

      rvertex[i].x = cos(zang) * tx + sin(zang) * vertex[i].y;
      rvertex[i].y = sin(xang) * tz + cos(xang) * ty;
      rvertex[i].z = cos(xang) * tz - sin(xang) * ty;
    }

  // Calculate plane color before distorting it with perspective
  for (int i = 0; i < plane.size(); i++)
    plane[i].color = luminate(plane[i]);
    
  // Add perspective
  for (int i = 0; i < vertex.size(); i++)
    {   
      rvertex[i].x = 256 * rvertex[i].x / (2 * rvertex[i].z - zoff) + xoff;
      rvertex[i].y = 256 * rvertex[i].y / (2 * rvertex[i].z - zoff) + yoff;
    }
}

/*******************************************************************

 void WmcObject::draw(WMCanvas *icanvas)
 
 Draws the WmcObject on the WMCanvas icanvas. Defaults to planes if
 available, otherwise wireframe is used. Buffered mode is used
 so canvas->display() must be called at the end.

********************************************************************/
void WmcObject::draw(WMCanvas *icanvas)
{
  canvas = icanvas;

  canvas->setbuffered(true);
  
  switch (mode)
  {
  	case WIREFRAME:
  	canvas->setcolor((WMColor::WMColor)WIRE_COLOR);
  	for (int i = 0; i < line.size(); i++)
		canvas->draw_line((int)(line[i].a->x), (int)(line[i].a->y),(int)(line[i].b->x), (int)(line[i].b->y));
  	break;
  	
  	case SOLID:
  	sort(plane);
	for (int i = 0; i < plane.size(); i++)
		if (visible(plane[i])) drawTriangle(plane[i]);
  	break;
  	
  	case SOLID_WIRE:
  	sort(plane);
	for (int i = 0; i < plane.size(); i++)
		if (visible(plane[i])) drawTriangle(plane[i], true);
  	break;
  }

  canvas->display();
}

/*******************************************************************

 bool WmcObject::visible(Plane p)
 
 Returns true if the plane is visible (has positive z-component).

********************************************************************/
bool WmcObject::visible(Plane p)
{
  //return normal(p).z > 0.0; // Also works but no need computing x- and y-components
  return ((p.a->x - p.c->x) * (p.b->y - p.c->y) - (p.b->x - p.c->x) * (p.a->y - p.c->y)) > 0.0;
}

/*******************************************************************

 Vertex WmcObject::normal(Plane p)
 
 Returns plane p's normal.

********************************************************************/
Vertex WmcObject::normal(Plane p)
{
  Vertex ret;
  float x1 = p.a->x - p.c->x, y1 = p.a->y - p.c->y, z1 = p.a->z - p.c->z;
  float x2 = p.b->x - p.c->x, y2 = p.b->y - p.c->y, z2 = p.b->z - p.c->z;
  ret.x = y1 * z2 - y2 * z1;
  ret.y = x2 * z1 - x1 * z2;
  ret.z = x1 * y2 - y1 * x2;

  return ret;
}

/*******************************************************************

 float WmcObject::luminate(Plane p)
 
 Returns the luminousity _DECREASE_ for plane p in the range 0..1.
 This is based solely on the angle between the plane's normal and
 the lightsource vector, i.e the distance between the lightsource
 and plane is _not_ taken into account (which would be easy to fake).

********************************************************************/
float WmcObject::luminate(Plane p)
{
  Vertex pn      = normal(p);
  float sp_pn  = sqrt(pn.x * pn.x + pn.y * pn.y + pn.z * pn.z);
  float sp_lum = sqrt(lightsource.x * lightsource.x + lightsource.y * lightsource.y 
			+ lightsource.z * lightsource.z);
  float pn_lum = pn.x * lightsource.x + pn.y * lightsource.y + pn.z * lightsource.z;
  
  return invPI * acos((pn_lum / (sp_pn * sp_lum)));
}

/*******************************************************************

 void WmcObject::sort(vector<Plane> &plane)
 
 Insertion-sorts a vector of planes by the sum of their z-components.

********************************************************************/
void WmcObject::sort(vector<Plane> &plane)
{
  int i, j, k;
  float key;
  float temparr[plane.size()];
  Plane ptmp;
  
  for (i = 0; i < plane.size(); i++)
    temparr[i] = plane[i].a->z + plane[i].b->z + plane[i].c->z;
  
  for (j = 1; j < plane.size(); j++)
    {
      ptmp = plane[j];
      key = temparr[j];
      i = j - 1;
      
      while ((i > -1) && (temparr[i] > key)) 
 	{
	  plane[i+1]   = plane[i];
 	  temparr[i+1] = temparr[i];
	  i--;
	} 
      
      plane[i+1]   = ptmp;							
      temparr[i+1] = key;
    }
}

/*******************************************************************

 void WmcObject::drawTriangle(Plane p)

 Draws a filled polygon using the builtin routine from wmapp which
 in turn uses the builtin routine in X. Sadly enough it is uglier
 than my own original routine due to some corner pixel that gets
 plotted sometime. Plane color is computed  using the global
 variables colX and colX_dec and 'p.color'. 

********************************************************************/
void WmcObject::drawTriangle(Plane p, bool wire)
{
  X::XPoint xp;
  vector<X::XPoint> points;
  unsigned char R = 0, G = 0, B = 0;
  unsigned composit = 0;

  xp.x = (int)p.a->x;
  xp.y = (int)p.a->y;
  points.push_back(xp);
  xp.x = (int)p.b->x;
  xp.y = (int)p.b->y;
  points.push_back(xp);
  xp.x = (int)p.c->x;
  xp.y = (int)p.c->y;
  points.push_back(xp);

  R = (unsigned char)(color >> 16);
  G = (unsigned char)(color >> 8 );
  B = (unsigned char)(color      );
  
  R -= (unsigned char)((float)R * p.color * shading);
  G -= (unsigned char)((float)G * p.color * shading);
  B -= (unsigned char)((float)B * p.color * shading);
  
  composit = (R << 16) + (G << 8) + B;

  // If the plane is almost purpendicular to the lightsource fake a 'reflex'
  if (p.color < REFLEX) composit = REFLEX_COLOR;

  canvas->setcolor((WMColor::WMColor)composit);
  canvas->fill_polygon(points);

  if (!wire) return;
  
  canvas->setcolor((WMColor::WMColor)WIRE_COLOR);
  
  if (p.ab > -1)
    canvas->draw_line((int)(line[p.ab].a->x), (int)(line[p.ab].a->y),
		      (int)(line[p.ab].b->x), (int)(line[p.ab].b->y));
  if (p.bc > -1)
    canvas->draw_line((int)(line[p.bc].a->x), (int)(line[p.bc].a->y),
		      (int)(line[p.bc].b->x), (int)(line[p.bc].b->y));
  if (p.ac > -1)
    canvas->draw_line((int)(line[p.ac].a->x), (int)(line[p.ac].a->y),
		      (int)(line[p.ac].b->x), (int)(line[p.ac].b->y));

}
