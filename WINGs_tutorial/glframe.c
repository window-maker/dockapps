#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <WINGs/WINGs.h>
#include <WINGs/WINGsP.h>
#include <stdio.h>

#define CONTENTH  300
#define CONTENTW  300
#define CONTENTMARGIN 30


struct couple{
  WMWindow *window;
  WMFrame *frame;
} datacouple;


float red=252.0/256, green=88.0/256, blue=16.0/256;
float redb=252.0/256, greenb=242.0/256, blueb=80.0/256;

int Attr[] = {	GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		GLX_DOUBLEBUFFER,
		None};
Display *  display;
float alpha=0;

void init(void) 
{
glClearColor (256/256, 256/256, 256/256, 0.0);
glPolygonMode(GL_FRONT, GL_FILL);
glPolygonMode(GL_BACK, GL_FILL);
glEnable(GL_DEPTH_TEST);
glShadeModel(GL_SMOOTH);

glEnable(GL_LIGHTING);
GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat position[] = { 2.0f, -0.1f, 2.0f, 1.0f };
GLfloat position2[] = { -2.0f, -0.26f, -4.0f, 1.0f };
glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
//glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
glMateriali(GL_FRONT, GL_SHININESS, 98);
glLightfv(GL_LIGHT0, GL_POSITION, position2);
//glLightfv(GL_LIGHT1, GL_POSITION, position2);

glEnable(GL_LIGHT0);
//glEnable(GL_LIGHT1);
glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
//glColorMaterial(GL_FRONT, GL_SPECULAR);
glEnable(GL_COLOR_MATERIAL);
glShadeModel(GL_SMOOTH);
glCullFace( GL_BACK );
glEnable( GL_CULL_FACE );

glEnable(GL_POLYGON_SMOOTH); 
/*glEnable(GL_BLEND); 
glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE); 
//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);*/
 
}

void normvector(float a, float aa, float aaa, float b, float bb, float bbb, float c, float cc, float ccc,float *result){
  float v1[3];float v2[3];float tmp;

  v1[0]=(b-a);v1[1]=(bb-aa);v1[2]=(bbb-aaa);
  v2[0]=(b-c);v2[1]=(bb-cc);v2[2]=(bbb-ccc);
  result[0]=(v1[1]*v2[2]-v1[2]*v2[1]);
  result[1]=(v1[2]*v2[0]-v1[0]*v2[2]);
  result[2]=(v1[0]*v2[1]-v1[1]*v2[0]);
  tmp=sqrt(result[0]*result[0]+result[1]*result[1]+result[2]*result[2]);
  result[0]/=tmp;
  result[1]/=tmp;
  result[2]/=tmp;
}


void redraw(XEvent * v,void *xw){
Window win;
float z[3];

win = *(Window *)xw;

glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   
glPushMatrix();	glShadeModel(GL_SMOOTH);

glRotatef(alpha, 0, 1, 0);
if (alpha > 360) alpha =alpha-360;

glBegin(GL_TRIANGLES);        

glColor3f(redb,greenb,blueb);
normvector(-0.85f, 0.0f, 0.0f,0.0f, 0.0f, 0.85f,0.0f, 0.6f, 0.0f,z); 
glNormal3fv(z);
glVertex3f(-0.85f, 0.0f, 0.0f);       
glVertex3f(0.0f, 0.0f, 0.85f);         
glVertex3f(0.0f, 0.6f, 0.0f);

normvector(0.0f,  0.0f,0.85f,0.85f, 0.0f, 0.0f,0.0f,  0.60f,0.0f,z);
glNormal3fv(z);
glVertex3f(0.0f, 0.0f, 0.85f);
glVertex3f(0.85f, 0.0f, 0.0f);       
glVertex3f(0.0f, 0.6f, 0.0f);

glColor3f(red,green,blue);   
normvector(0.85f, 0.0f, 0.0f,0.0f, 0.0f, -0.85f,0.0f, 0.6f, 0.0f,z);
glNormal3fv(z);
glVertex3f(0.85f, 0.0f, 0.0f);
glVertex3f(0.0f, 0.0f, -0.85f);
glVertex3f(0.0f, 0.6f, 0.0f);          

normvector(0.0f, 0.0f, -0.85f,-0.85f, 0.0f, 0.0f,0.0f, 0.6f, 0.0f,z);
glNormal3fv(z);	
glVertex3f(0.0f, 0.0f, -0.85f);
glVertex3f(-0.85f, 0.0f, 0.0f);        
glVertex3f(0.0f, 0.6f, 0.0f);

glColor3f(redb,greenb,blueb);  
normvector(-0.85f, 0.0f, 0.0f,0.0f, -1.0f, 0.0f,0.0f, 0.0f, 0.85f,z);
glNormal3fv(z);
glVertex3f(-0.85f, 0.0f, 0.0f);      
glVertex3f(0.0f, -1.0f, 0.0f);
glVertex3f(0.0f, 0.0f, 0.85f);
 
normvector(0.0f, 0.0f, 0.85f, 0.0f, -1.0f, 0.0f,0.85f, 0.0f, 0.0f ,z);
glNormal3fv(z); 
glVertex3f(0.0f, 0.0f, 0.85f);      
glVertex3f(0.0f, -1.0f, 0.0f);
glVertex3f(0.85f, 0.0f, 0.0f);

glColor3f(red,green,blue);
normvector(0.85f, 0.0f, 0.0f,0.0f, -1.0f, 0.0f, 0.0f, 0.0,-0.85f ,z);
glNormal3fv(z); 
glVertex3f(0.85f, 0.0f, 0.0f);      
glVertex3f(0.0f,-1.0f, 0.0f);
glVertex3f(0.0f, 0.0f, -0.85f);

normvector(0.0f, 0.0f, -0.85f,0.0f, -1.0f, 0.0f, -0.85f, 0.0f,0.0f ,z);
glNormal3fv(z); 
glVertex3f(0.0f, 0.0f, -0.85f);     
glVertex3f(0.0f, -1.0f, 0.0f);
glVertex3f(-0.85f, 0.0f, 0.0f);
      
glEnd();

glPopMatrix();
glXSwapBuffers(display, win);
}

setsize(unsigned int width, unsigned int height)
{
glViewport(0, 0, width, height);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();                 
glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);    
glMatrixMode (GL_MODELVIEW);
}

void DoRotate(void *self,void *xwindow){
 XEvent event;int i=0;
 alpha+=15; 
redraw(NULL,(Window *)xwindow); 
}

void redo(XEvent * event,void *xw){
	switch (event->type)
		{
		case Expose:
		 if (event->xexpose.count!=0) break;
		 redraw(event,&event->xexpose.window);
		 break;
		case ConfigureNotify: setsize(event->xconfigure.width, event->xconfigure.height); // assuming there will be an expose afterwards
		  break;	
		}
}

void closeAll(WMWidget *self,void *data){
  WMDestroyWidget(self);
  exit(0);
}

static void resizeHandler(void *data, WMNotification *notif){
 struct couple *tmp;tmp=(struct couple *)data;
  WMSize size = WMGetViewSize(WMWidgetView(tmp->window));   
  WMResizeWidget(tmp->frame, size.width -2*CONTENTMARGIN, size.height-2*CONTENTMARGIN);
}

void getargs(int argc, char **argv){
if (argc>3) {
   redb=red=(float)atoi(argv[1])/256;
  greenb=green=(float)atoi(argv[2])/256;
  blueb=blue=(float)atoi(argv[3])/256;
}
 if (argc>6){
   redb=(float)atoi(argv[4])/256;
  greenb=(float)atoi(argv[5])/256;
  blueb=(float)atoi(argv[6])/256;
  }
}

int main (int argc, char **argv){

WMFrame *glframe;
WMScreen *screen;
WMWindow *window;
WMButton *Button;


/*    Xlib and glX variables    */
Window 	win;
XVisualInfo	*xvVisualInfo;
Colormap	cmColorMap;
XSetWindowAttributes 	winAttr;
GLXContext       glXContext;

getargs(argc,argv);
WMInitializeApplication("GLWindow", &argc, argv);
display = XOpenDisplay("");
screen = WMCreateScreen(display, DefaultScreen(display));

 if(!glXQueryExtension(display, NULL, NULL)){wwarning("X server does not have GLX\n"); return 0; }

window = WMCreateWindow(screen, "Main");
WMResizeWidget(window, CONTENTW+2*CONTENTMARGIN, CONTENTH+2*CONTENTMARGIN*CONTENTH/CONTENTW);
WMSetWindowAspectRatio(window, CONTENTW,CONTENTH,CONTENTW,CONTENTH);
WMSetWindowCloseAction(window, closeAll, NULL);
WMSetWindowTitle(window,"GL Frame");
WMRealizeWidget(window); 
datacouple.window=window;
WMSetViewNotifySizeChanges(WMWidgetView(window), True);
WMAddNotificationObserver(resizeHandler, &datacouple, WMViewSizeDidChangeNotification, WMWidgetView(window));


glframe = WMCreateFrame(window);
datacouple.frame=glframe;
WMResizeWidget(glframe, CONTENTW, CONTENTH);
WMMoveWidget(glframe, CONTENTMARGIN,CONTENTMARGIN);
WMRealizeWidget(glframe); 

Button=WMCreateButton(window, WBTMomentaryPush);
WMSetButtonAction(Button, DoRotate,&win);
WMSetButtonText(Button,"Turn");
WMMoveWidget(Button, CONTENTMARGIN,2);
WMRealizeWidget(Button);
WMMapWidget(Button);

/*  get the frame's X window value  */
win =W_VIEW_DRAWABLE(WMWidgetView(glframe));
WMCreateEventHandler(WMWidgetView(glframe), ExposureMask|StructureNotifyMask,redo,&win);

xvVisualInfo = glXChooseVisual(display, DefaultScreen(display), Attr);
 if(xvVisualInfo == NULL) {wwarning("No visualinfo\n");return 0;}

cmColorMap = XCreateColormap(display,RootWindow(display, DefaultScreen(display)), xvVisualInfo->visual, AllocNone);

winAttr.colormap = cmColorMap;
winAttr.border_pixel = 0;
winAttr.background_pixel = 0;
winAttr.event_mask = ExposureMask | ButtonPressMask  |StructureNotifyMask| KeyPressMask;

XChangeWindowAttributes(display,win,CWBorderPixel | CWColormap | CWEventMask,&winAttr);
glXContext = glXCreateContext(display, xvVisualInfo, None, True);
  if(!glXContext) {wwarning("glX cannot create rendering context\n");return 0;} 

glXMakeCurrent(display, win, glXContext);

WMMapWidget(glframe);
init();
setsize(CONTENTW,CONTENTH);
WMMapWidget(window);

WMScreenMainLoop(screen);

return 0;
}
