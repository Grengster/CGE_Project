#ifdef __APPLE__
#include <GLUT/glut.h> 
#include <OpenGL/gl.h>  
#include <OpenGL/glu.h>  
#else
#include <GL/glut.h> 
#include <GL/gl.h>  
#include <GL/glu.h>  
#endif
#include <stdlib.h> // for exit
#include <stdio.h>
#include "tga.h"
#include <iostream>
#include <cstring>
#include <math.h>


/* some math.h files don't define pi... */
#ifndef M_PI
#define M_PI 3.141592653
#endif

#define RAD(x) (((x)*M_PI)/180.)

#ifdef __STRICT_ANSI__
#define sinf(x) ((float)sin((x)))
#define cosf(x) ((float)cos((x)))
#define atan2f(x, y) ((float)atan2((x), (y)))
#endif 

GLfloat WHITE[] = { 1, 1, 1 };
GLfloat RED[] = { 1, 0, 0 };
GLfloat GREEN[] = { 0, 1, 0 };
GLfloat MAGENTA[] = { 1, 0, 1 };



int window;
float advance = 0.0f;
float sideways = 0.0f;
float objectMovement = 0.0f;
GLuint texfloor;
GLuint texwall;
GLuint texscare;
int animating = 1;
bool lightsOn = true;

int moving = 0;     /* flag that is true while mouse moves */
int begin_x = 0;        /* x value of mouse movement */
int begin_y = 0;      /* y value of mouse movement */
GLfloat angle_y = 0;  /* angle of spin around y axis of scene, in degrees */
GLfloat angle_x = 0;  /* angle of spin around x axis  of scene, in degrees */
double r, g, b;
double l = 380;

void spectral_color(double& r, double& g, double& b, double l) // RGB <- lambda l = < 380,780 > [nm]
{
    if (l < 380.0) r = 0.00;
    else if (l < 400.0) r = 0.05 - 0.05 * sin(M_PI * (l - 366.0) / 33.0);
    else if (l < 435.0) r = 0.31 * sin(M_PI * (l - 395.0) / 81.0);
    else if (l < 460.0) r = 0.31 * sin(M_PI * (l - 412.0) / 48.0);
    else if (l < 540.0) r = 0.00;
    else if (l < 590.0) r = 0.99 * sin(M_PI * (l - 540.0) / 104.0);
    else if (l < 670.0) r = 1.00 * sin(M_PI * (l - 507.0) / 182.0);
    else if (l < 730.0) r = 0.32 - 0.32 * sin(M_PI * (l - 670.0) / 128.0);
    else              r = 0.00;
    if (l < 454.0) g = 0.00;
    else if (l < 617.0) g = 0.78 * sin(M_PI * (l - 454.0) / 163.0);
    else              g = 0.00;
    if (l < 380.0) b = 0.00;
    else if (l < 400.0) b = 0.14 - 0.14 * sin(M_PI * (l - 364.0) / 35.0);
    else if (l < 445.0) b = 0.96 * sin(M_PI * (l - 395.0) / 104.0);
    else if (l < 510.0) b = 0.96 * sin(M_PI * (l - 377.0) / 133.0);
    else              b = 0.00;
}

bool xmax = false, ymax = false, zmax = false;

void CheckCoordsX(double& x)
{
    if (xmax)
    {
        if (x < 0)
            xmax = false;
        x -= 0.01f;
    }
    else
    {
        if (x > 10)
            xmax = true;
        x += 0.01f;
    }
}

void CheckCoordsZ(double& z)
{
    if (zmax)
    {
        if (z < -1)
            zmax = false;
        z -= 0.02f;
    }
    else
    {
        if (z > 7.5)
            zmax = true;
        z += 0.02f;
    }
}

class Ball {
    double radius;
    GLfloat* color;
    double maximumHeight;
    double x;
    double y;
    double z;
    int direction;
public:
    Ball(double r, GLfloat* c, double h, double x, double z) :
        radius(r), color(c), maximumHeight(h), direction(-1),
        y(h), x(x), z(z) {
    }
    void update() {
        y += direction * 0.01;
        if (y > maximumHeight) {
            y = maximumHeight; direction = -1;
        }
        else if (y < radius) {
            y = radius; direction = 1;
        }
        spectral_color(r, g, b, l);
        GLfloat temp[] = { r, g, b };
        if (l > 780) {
            l = 380;
        }
        else {
            l += 0.25;
        }
        
        
        

        CheckCoordsX(x);
        CheckCoordsZ(z);
        glPushMatrix();
        GLfloat light_diffuse[] = { 1, 1, 1, 1.0 };
        GLfloat light_position[] = { x, y, z, 1 };
        if (this->color == RED)
        {
            glPushMatrix();
            glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glPopMatrix();
            
        }
        if(!lightsOn)
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, light_diffuse);
        else
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, temp);
        //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, temp);
        //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, temp);
        glTranslated(x, y, z);
        glutSolidSphere(radius, 30, 30);
        glPopMatrix();
    }

};

Ball balls[] = {
  Ball(1, GREEN, 7, 0, 0),
  Ball(0.5f, RED, 5, 1, 2),
  Ball(0.25f, WHITE, 3, 5, 4),
  Ball(0.75f, GREEN, 2, 2, 5)
};


void reportGLError(const char* msg)
{
    GLenum errCode;
    const GLubyte* errString;
    while ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        fprintf(stderr, "OpenGL Error: %s %s\n", msg, errString);
    }
    return;
}

void resize(int width, int height)
{
    // prevent division by zero
    if (height == 0) { height = 1; }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void specialKeyPressed(unsigned char key, int x, int y)
{

}

void keyPressed(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        glutDestroyWindow(window);
        exit(0);
        break;
    case 'z':
        animating = animating ? 0 : 1;
        glutPostRedisplay();
        break;

    case 'h':     
        lightsOn = true;
        break;
    case 'j':     
        lightsOn = false;
        break;
    case 'w':     
        if ((advance > -1.0f && sideways > 5.0f) || (sideways >= 1.0f && sideways <= 5.0f && advance >= -8.5f) || (sideways <= -1.0f && advance >= -8.5f && advance < -1.0f) || (advance >= -47.0f && sideways <= 1.0f && sideways >= -1.0f) || (advance < -47.0f && advance > -49.0f && sideways < 1.0f && sideways > -40.0f))
        {
            advance -= 0.5f;
            glutPostRedisplay();
        }
        break;
    case 'a':     
        if ((sideways > -5.0f && advance > -8.0f) || (advance <= -8.0f && advance >= -47.0f && sideways > -1.0f) || (advance < -47.0f && sideways > -40.0f))
        {
            sideways -= 0.5f;
            glutPostRedisplay();
        }
        break;
    case 's':     /* <cursor down> */
        if (!(advance >= 1.0f || (advance > -48.0f && sideways < -1.0f)))
        {
            advance += 0.5f;
            glutPostRedisplay();
        }
        break;
    case 'd':     /* <cursor down> */
        if ((advance >= -1.0f) || (sideways <= 5.0f && advance < -1.0f && advance > -8.0f) || (advance <= -8.0f && sideways < 1.0f))
        {
            sideways += 0.5f;
            glutPostRedisplay();
        }
        break;
    case 'e':
        if (objectMovement < 3.9f)
        {
            objectMovement += 0.1f;
            glutPostRedisplay();
        }
        break;
    case 'q':
        if (objectMovement > -3.9f)
        {
            objectMovement -= 0.1f;
            glutPostRedisplay();
        }
        glutPostRedisplay();
        break;
    default:
        break;
    }
}



void drawPlain(int i)
{
    glBegin(GL_QUADS);

    // front face
    switch (i)
    {
    case 1: //FACE/BACK
    {
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -2.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(2.0f, -2.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(2.0f, 2.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 2.0f, 0.0f);
        break;
    }
    case 2: //LEFT/RIGHT
    {
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.0f, -2.0f, -2.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.0f, -2.0f, 2.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.0f, 2.0f, 2.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.0f, 2.0f, -2.0f);
        break;
    }
    case 3: //BOTTOM/TOP
    {
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-2.0f, -2.0f, -2.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, -2.0f, -2.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, -2.0f, 2.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.0f, -2.0f, 2.0f);
        break;
    }
    case 4: //LEFT/RIGHT (SMALL)
    {
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.0f, -2.0f, -2.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.0f, -2.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.0f, 2.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.0f, 2.0f, -2.0f);
        break;
    }
    default:
        break;
    }
    glEnd();
}

void DrawCube()
{
    drawPlain(1);
    glTranslatef(-2.0f, 0.0f, 0.0f);
    drawPlain(4);
    glTranslatef(4.0f, 0.0f, 0.0f);
    drawPlain(4);
    glTranslatef(-2.0f, 0.0f, -2.0f);
    drawPlain(1);
    glTranslatef(0.0f, 0.0f, 2.0f);
}

void drawFloorCeilingRoom()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            drawPlain(3);
            glTranslatef(4.0f, 0.0f, 0.0f);
        }
        glTranslatef(-12.0f, 0.0f, 4.0f);
    }
}

void drawFrontWallRoom()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i == 0 && j == 1)
            {
                glTranslatef(4.0f, 0.0f, 0.0f);
            }
            else
            {
                drawPlain(1);
                glTranslatef(4.0f, 0.0f, 0.0f);
            }
        }
        glTranslatef(-12.0f, 4.0f, 0.0f);
    }
}

void drawBackWallRoom()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            drawPlain(1);
            glTranslatef(4.0f, 0.0f, 0.0f);
        }
        glTranslatef(-12.0f, 4.0f, 0.0f);
    }
}

void drawSideWallRoom(int right)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (right == 1 && i == 0 && j == 0)
            {
                glTranslatef(0.0f, 0.0f, -4.0f);
                continue;
            }
            drawPlain(2);
            glTranslatef(0.0f, 0.0f, -4.0f);
        }
        glTranslatef(0.0f, 4.0f, 12.0f);
    }
}

void drawPassageFloor()
{
    glTranslatef(0.0f, 0.0f, -4.0f);
    for (int i = 0; i < 10; i++)
    {
        drawPlain(3);
        glTranslatef(0.0f, 0.0f, -4.0f);
    }
    glTranslatef(0.0f, 0.0f, 40.0f);
}

void drawPassageWalls(int closed, int ending)
{
    glTranslatef(-2.0f, 0.0f, 0.0f);
    //Left
    for (int i = 0; i < 10; i++)
    {
        if (closed == 1)
        {
            drawPlain(2);
        }
        else if (i != 9)
        {
            drawPlain(2);
        }
        glTranslatef(0.0f, 4.0f, 0.0f);
        drawPlain(2);
        glTranslatef(0.0f, -4.0f, -4.0f);
    }
    glTranslatef(4.0f, 0.0f, 40.0f); //Reset
    //Right
    for (int i = 0; i < 10; i++)
    {
        drawPlain(2);
        glTranslatef(0.0f, 4.0f, 0.0f);
        drawPlain(2);
        glTranslatef(0.0f, -4.0f, -4.0f);
    }
    //Back Wall
    if (ending == 1)
    {
        glTranslatef(-2.0f, 0.0f, 2.0f);
        drawPlain(1);
        glTranslatef(0.0f, 4.0f, 0.0f);
        drawPlain(1);
        glTranslatef(2.0f, -4.0f, -2.0f);
    }
    //Ceiling
    glTranslatef(-2.0f, 8.0f, 4.0f);
    for (int i = 0; i < 10; i++)
    {
        drawPlain(3);
        glTranslatef(0.0f, 0.0f, 4.0f);
    }
    glTranslatef(0.0f, -8.0f, 0.0f); //Reset
}

void display()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(-sinf(RAD(angle_y)), sinf(RAD(angle_x)), cosf(RAD(angle_y)),
        0., 0., 0.,
        0., 1., 0.);

    glPushMatrix();

    glTranslatef(-6.0f, 0.0f, -6.0f);



    glPopMatrix();

    glTranslatef(0, 0, -advance);
    glTranslatef(-sideways, 0, 0);

    glTranslatef(-5.0f, 0.0f, -6.0f);

    for (int i = 0; i < sizeof balls / sizeof(Ball); i++) {
        balls[i].update();
    }

    glTranslatef(5.0f, 0.0f, 6.0f);
    glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texfloor);




    glTranslatef(0.0f, 0.0f, -4.0f);

    //Drawing the floor
    glTranslatef(-4.0f, 0.0f, -4.0f); //Setting Start point
    drawFloorCeilingRoom();
    glTranslatef(4.0f, 0.0f, -4.0f); //Resetting back to start

    glBindTexture(GL_TEXTURE_2D, texwall);
    //Drawing Ceiling
    glTranslatef(-4.0f, 12.0f, -8.0f); //Setting Start point
    drawFloorCeilingRoom();
    glTranslatef(4.0f, -12.0f, -4.0f); //Resetting back to start

    //Drawing Front Wall
    glTranslatef(-4.0f, 0.0f, -10.0f); //Setting Start point
    drawFrontWallRoom();
    glTranslatef(4.0f, -12.0f, 4.0f); //Resetting back to start

    //Drawing Back Wall
    glTranslatef(-4.0f, 0.0f, 8.0f); //Setting Start point
    drawBackWallRoom();
    glTranslatef(4.0f, -12.0f, -6.0f); //Resetting back to start

    //Drawing Left Wall
    glTranslatef(-6.0f, 0.0f, 4.0f); //Setting Start point
    drawSideWallRoom(0);
    glTranslatef(6.0f, -12.0f, -4.0f); //Resetting back to start

    //Drawing Right Wall
    glTranslatef(6.0f, 0.0f, 4.0f); //Setting Start point
    drawSideWallRoom(1);
    glTranslatef(-6.0f, -12.0f, -4.0f); //Resetting back to start

    glBindTexture(GL_TEXTURE_2D, texfloor);
    glTranslatef(0.0f, 0.0f, -4.0f); //Setting Start point
    drawPassageFloor();
    
    glTranslatef(0.0f, 0.0f, -36.0f); //Startpoint for extended passage
    glRotatef(90, 0, 1, 0);
    drawPassageFloor();

    glBindTexture(GL_TEXTURE_2D, texwall);
    drawPassageWalls(1, 1);
    glRotatef(270, 0, 1, 0);
    glTranslatef(0.0f, 0.0f, 36.0f);

    //glBindTexture(GL_TEXTURE_2D, texwall);
    drawPassageWalls(0, 1);

    //Drawing Obstacle
    glTranslatef(objectMovement, 0.0f, 0.0f);
    DrawCube();
    glTranslatef(-objectMovement, 0.0f, 0.0f);

    //Drawing endless path
    glTranslatef(4.0f, 0.0f, 8.0f); //Startpoint for endless passage
    glRotatef(270, 0, 1, 0);

    for (int i = 0; i < 7; i++)
    {

        glBindTexture(GL_TEXTURE_2D, texfloor);
        drawPassageFloor();
        glBindTexture(GL_TEXTURE_2D, texwall);
        drawPassageWalls(1, 0);
        glTranslatef(0.0f, 0.0f, -40.0f);
    }
    glTranslatef(0.0f, 0.0f, 280.0f); // Resetting back
    glRotatef(90, 0, 1, 0);

    if (sideways == 160)
        sideways = 120;

    if (advance < -45.0f && sideways < -15.0f)
    {
        glBindTexture(GL_TEXTURE_2D, texscare);
        glTranslatef(sideways-8.0f, 0.0f, -47.0f);
        DrawCube();
        glTranslatef(-sideways-8.0f, 0.0f, -47.0f);
    }
    
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
}

void init(int width, int height)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);


    resize(width, height);

    GLsizei w, h;
    tgaInfo* info = 0;
    int mode;

    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::string projectLocation = "";
    for (int i = 0; i < MAX_PATH; i++)
    {
        if (path[i] == NULL)
            break;
        projectLocation += path[i];
    }
    std::size_t pos = projectLocation.find("Debug\\CGE_Project.exe");
    projectLocation = projectLocation.substr(0, pos);

    std::string str = projectLocation + "textures/stonefloor.tga";
    char* cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    // do stuff
    info = tgaLoad(cstr);
    delete[] cstr;

    if (info->status != TGA_OK) {
        fprintf(stderr, "error loading texwall image: %d\n", info->status);

        return;
    }
    if (info->width != info->height) {
        fprintf(stderr, "Image size %d x %d is not rectangular, giving up.\n",
            info->width, info->height);
        return;
    }

    mode = info->pixelDepth / 8;  // will be 3 for rgb, 4 for rgba
    glGenTextures(1, &texfloor);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, texfloor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Upload the texwall bitmap. 
    w = info->width;
    h = info->height;

    reportGLError("before uploading texwall");
    GLint format = (mode == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format,
        GL_UNSIGNED_BYTE, info->imageData);
    reportGLError("after uploading texwall");

    tgaDestroy(info);

    str = projectLocation + "textures/stonewall2.tga";
    cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    // do stuff
    info = tgaLoad(cstr);
    delete[] cstr;


    if (info->status != TGA_OK) {
        fprintf(stderr, "error loading texwall image: %d\n", info->status);

        return;
    }
    if (info->width != info->height) {
        fprintf(stderr, "Image size %d x %d is not rectangular, giving up.\n",
            info->width, info->height);
        return;
    }

    mode = info->pixelDepth / 8;  // will be 3 for rgb, 4 for rgba
    glGenTextures(1, &texwall);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, texwall);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Upload the texwall bitmap. 
    w = info->width;
    h = info->height;

    reportGLError("before uploading texwall");
    format = (mode == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format,
        GL_UNSIGNED_BYTE, info->imageData);
    reportGLError("after uploading texwall");

    tgaDestroy(info);


    str = projectLocation + "textures/surprise.tga";
    cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    // do stuff
    info = tgaLoad(cstr);
    delete[] cstr;

    if (info->status != TGA_OK) {
        fprintf(stderr, "error loading texwall image: %d\n", info->status);

        return;
    }
    if (info->width != info->height) {
        fprintf(stderr, "Image size %d x %d is not rectangular, giving up.\n",
            info->width, info->height);
        return;
    }

    mode = info->pixelDepth / 8;  // will be 3 for rgb, 4 for rgba
    glGenTextures(1, &texscare);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, texscare);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Upload the texwall bitmap. 
    w = info->width;
    h = info->height;

    reportGLError("before uploading texwall");
    format = (mode == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format,
        GL_UNSIGNED_BYTE, info->imageData);
    reportGLError("after uploading texwall");

    tgaDestroy(info);
}

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(15, timer, 1);
}


void mouse(int button, int state, int x, int y)
{
    switch (button) {
    case GLUT_LEFT_BUTTON:    /* spin scene around */
        if (state == GLUT_DOWN) {
            moving = 1;
            begin_x = x;
            begin_y = y;

        }
        else if (state == GLUT_UP) {
            moving = 0;
        }
        break;

    default:
        break;
    }

    glutPostRedisplay();
}


void mouseMotion(int x, int y) {

    if (moving) { /* mouse button is pressed */

      /* calculate new modelview matrix values */
        angle_y = angle_y + (x - begin_x);
        angle_x = angle_x + (y - begin_y);
        if (angle_x > 360.0) angle_x -= 360.0;
        else if (angle_x < -360.0) angle_x += 360.0;
        if (angle_y > 360.0) angle_y -= 360.0;
        else if (angle_y < -360.0) angle_y += 360.0;

        begin_x = x;
        begin_y = y;
        glutPostRedisplay();
    }
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(0, 0);
    window = glutCreateWindow("foo");
    glutDisplayFunc(&display);
    glutReshapeFunc(&resize);
    glutKeyboardFunc(&keyPressed);
    //glutSpecialFunc(&specialKeyPressed);
    init(640, 480);
    glutTimerFunc(15, timer, 1);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutFullScreen();
    glutMainLoop();
    return 0;
}
