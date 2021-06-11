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

int window;
float advance = 0.0f;
float sideways = 0.0f;
GLuint texfloor;
GLuint texwall;
int animating = 1;

int moving = 0;     /* flag that is true while mouse moves */
int begin_x = 0;        /* x value of mouse movement */
int begin_y = 0;      /* y value of mouse movement */
GLfloat angle_y = 0;  /* angle of spin around y axis of scene, in degrees */
GLfloat angle_x = 0;  /* angle of spin around x axis  of scene, in degrees */

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
    switch (key) {


    }
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
    default:
        break;
    case 'w':     /* <cursor up> */
        advance -= 0.2f;
        glutPostRedisplay();
        break;
    case 'a':     /* <cursor up> */
        sideways -= 0.2f;
        glutPostRedisplay();
        break;
    case 's':     /* <cursor down> */
        advance += 0.2f;
        glutPostRedisplay();
        break;
    case 'd':     /* <cursor down> */
        sideways += 0.2f;
        glutPostRedisplay();
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
    default:
        break;
    }
    glEnd();
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

void drawSideWallRoom()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
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

void drawPassageWalls()
{
    glTranslatef(-2.0f, 0.0f, 0.0f);
    //Left
    for (int i = 0; i < 10; i++)
    {
        drawPlain(2);
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
    glTranslatef(-2.0f, 0.0f, 2.0f);
    drawPlain(1);
    glTranslatef(0.0f, 4.0f, 0.0f);
    drawPlain(1);
    //Ceiling
    glTranslatef(0.0f, 4.0f, 2.0f);
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

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texfloor);

    glTranslatef(0, 0, -advance);
    glTranslatef(-sideways, 0, 0);
    
    glPushMatrix();
    //Drawing the floor
    glTranslatef(-4.0f, 0.0f, -4.0f); //Setting Start point
    drawFloorCeilingRoom();
    glTranslatef(4.0f, 0.0f, -4.0f); //Resetting back to start

    glBindTexture(GL_TEXTURE_2D, texwall);

    //Drawing Ceiling
    glTranslatef(-4.0f, 12.0f, -4.0f); //Setting Start point
    drawFloorCeilingRoom();
    glTranslatef(4.0f, -12.0f, -8.0f); //Resetting back to start

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
    drawSideWallRoom();
    glTranslatef(6.0f, -12.0f, -4.0f); //Resetting back to start

    //Drawing Right Wall
    glTranslatef(6.0f, 0.0f, 4.0f); //Setting Start point
    drawSideWallRoom();
    glTranslatef(-6.0f, -12.0f, -4.0f); //Resetting back to start

    glBindTexture(GL_TEXTURE_2D, texfloor);
    glTranslatef(0.0f, 0.0f, -4.0f); //Setting Start point
    drawPassageFloor();
    glBindTexture(GL_TEXTURE_2D, texwall);
    drawPassageWalls();
    
    
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
}

void init(int width, int height)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    resize(width, height);

    GLsizei w, h;
    tgaInfo* info = 0;
    int mode;

    info = tgaLoad("textures/stonefloor.tga");

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
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // Upload the texwall bitmap. 
    w = info->width;
    h = info->height;

    reportGLError("before uploading texwall");
    GLint format = (mode == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format,
        GL_UNSIGNED_BYTE, info->imageData);
    reportGLError("after uploading texwall");

    tgaDestroy(info);


    //OTHER texwall HERE


    info = tgaLoad("textures/stonewall2.tga");

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
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

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
    glutSpecialFunc(&specialKeyPressed);
    init(640, 480);
    glutTimerFunc(15, timer, 1);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutFullScreen();
    glutMainLoop();
    return 0;
}
