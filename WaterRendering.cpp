#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cyMatrix.h>
#include <cyVector.h>
#include <cyTriMesh.h>
#include <cyGL.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <lodepng.h>
#include <lodepng.cpp>

float viewHeight = 540;
float viewWidth = 960;

// Meshes
std::vector<cy::Vec3f> tank;
std::vector<cy::Vec3f> waterSurface;

// Mesh attributes
float height;
float width;
float length;
float thickness;

// Camera
cy::Vec3f camPos = cy::Vec3f(0.0f, 20.0f, 30.0f);
cy::Vec3f camTar = cy::Vec3f(0.0f, 0.0f, 0.0f);

// Programs
cy::GLSLProgram tankProg;

// Matrices
cy::Matrix4f tankViewMatrix;
cy::Matrix4f tankProjMatrix;
cy::Matrix4f tankModelMatrix;
cy::Matrix4f tankMvp;
cy::Matrix4f tankMv;
cy::Matrix3f tankMn;

// Buffers
GLuint tankFaceBuffer;

// Vertex Array Objects
GLuint vao;

// Texture IDs

// Mouse statistics
float oldX = 0;
float oldY = 0;
float centerX = viewWidth / 2;
float centerY = viewHeight / 2;
int mouseButton = 0;
int state = 0;

int main(int argc, char** argv);

static double D2R(int degrees);
static cy::Vec3f Mat3Vec3Mul(cy::Matrix3f m, cy::Vec3f v);
static void SetUpWaterTank(float width, float length, float height, float thickness);
static void SetUpWaterSurface(float width, float length, float height);
static void SetUpCamera();
static void DrawTank();
static void InitPrograms();


void display() {
    //// Clear the viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    DrawTank();

    // Swap buffers
        /**
         * Double buffering:
         * - Front buffer: only shows fully rendered images
         * - Secondary (back) buffer not visible
         * - Renders images in back buffer then swap the buffers when done (back becomes front and vice versa)*/
    glutSwapBuffers();

}

static void DrawTank() {
    glUseProgram(tankProg.GetID());
    tankProg.Bind();
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, tank.size());
}

void keypress(unsigned char key, int x, int y) {
    // Handle keyboard input here
    switch (key)
    {
    case 27: //ESC
        glutLeaveMainLoop();
        break;
    }
    glutPostRedisplay();
}

void specialKeypress(int key, int x, int y) {
    switch (key)
    {
    case GLUT_KEY_UP:
        //do something here
        break;
    case GLUT_KEY_DOWN:
        //do something here
        break;
    case GLUT_KEY_LEFT:
        
        break;
    case GLUT_KEY_RIGHT:
        
        //do something here
        break;
    }
}

void click(int button, int state, int x, int y) {
    // Handle mouse buttons here
    mouseButton = button;
    oldX = x;
    oldY = y;
}

void drag(int x, int y) {
    // Handle mouse motion here
    // while a button is down
    bool inViewPort = (x >= 0 && x <= viewWidth && y >= 0 && y <= viewHeight);

    if (mouseButton == GLUT_RIGHT_BUTTON && inViewPort) {
        bool towardCenter = (x <= oldX && x > centerX) || (x >= oldX && x < centerX);
        if (towardCenter) {
            //tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, -1.0f));
            camPos *= 1.02;
        }
        else {
            //tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, 1.0f));
            camPos *= 0.98;
        }
    }
    else if (mouseButton == GLUT_LEFT_BUTTON && inViewPort) {

        float directionX = 0.0f;
        float directionY = 0.0f;
        /*if (y < oldY) directionY = -1.0f;
        else if (y > oldY) directionY = 1.0f;
        if (x < oldX) directionX = -1.0f;
        else if (x > oldX) directionX = 1.0f;*/
        directionY = (y - oldY) * 0.1;
        directionX = (x - oldX) * 0.1;

        float rotAngleX = (directionX * D2R(2.0f));
        float rotAngleY = (directionY * D2R(2.0f));
        
        cy::Matrix3f Rx = cy::Matrix3f::RotationY(rotAngleX);
        //cy::Matrix3f Ry = cy::Matrix3f::RotationX(rotAngleY);

        camPos = Mat3Vec3Mul(Rx, camPos);
        //camPos = Mat3Vec3Mul(Ry, camPos);

    }
    SetUpCamera();
    glUseProgram(tankProg.GetID());
    tankProg["mvp"] = tankMvp;
    tankProg["MV"] = tankMv;
    tankProg["MN"] = tankMn;
    oldX = x;
    oldY = y;
}

void hover(int x, int y) {
    // Handle mouse motion here
    // while a button is NOT down
}

void reshape(int x, int y) {
    // Do what you want
    // when window size changes
}

void idle() {
    // Handle animations here
    glClearColor(0.5, 0.27, 0.35, 0.5);

    // Tell GLUT to redraw
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    // Initializations
    glutInitContextVersion(4, 5);
    glutInit(&argc, argv);

    // Create a window
    glutInitWindowSize(viewWidth, viewHeight);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(
        GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH
    );
    glutCreateWindow("Final Project - Water Rendering");

    glewInit();
    glutInitContextFlags(GLUT_DEBUG);
    CY_GL_REGISTER_DEBUG_CALLBACK;

    // Register display callback function
    glutDisplayFunc(display);
    glutKeyboardFunc(keypress);
    glutMouseFunc(click);
    glutSpecialFunc(specialKeypress);
    glutMotionFunc(drag);
    glutPassiveMotionFunc(hover);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    // Set the background color RGBA
    glClearColor(0, 0, 0, 0);

    // OpenGL Initializations
    width = 10.0f;
    length = 20.0f;
    height = 10.0f;
    thickness = 1.0f;
    SetUpWaterTank(width, length, height, thickness);
    SetUpWaterSurface(width-thickness*2,length-thickness*2,height);
    SetUpCamera();
    InitPrograms();


    // Call main loop
    glutMainLoop();

    return 0;
}

static double D2R(int degrees) {
    return (degrees * M_PI) / 180;
}

static cy::Vec3f Mat3Vec3Mul(cy::Matrix3f m, cy::Vec3f v) {
    float tol = 1e-7;
    cy::Vec3f result = cy::Vec3f(0.0f,0.0f,0.0f);
    result.x = m.Column(0).Dot(v);
    //result.x = abs(result.x) < tol ? 0 : result.x;
    result.y = m.Column(1).Dot(v);
    //result.y = abs(result.y) < tol ? 0 : result.y;
    result.z = m.Column(2).Dot(v);
    //result.z = abs(result.z) < tol ? 0 : result.z;
    return result;
}

static void SetUpWaterTank(float width, float length, float height, float thickness) {
    // length along the X axis
    // height along the Y axis
    // width along the Z axis
    
    // Outer walls:

    float wCoordOuter = width / 2;
    float lCoordOuter = length / 2;
    float hCoordOuter = height;
    
    // bottom - XZ-plane
    // coord format: (+-length/2,0,+-width/2)
    
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left

    // front - XY-plane
    // coord format: (+-length/2,height or 0,width/2)
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // bottom left 

    // back - XY-plane
    // coord format: (+-length/2,height or 0,-width/2)
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 

    // left - YZ-plane
    // coord format: (-length/2,height or 0,+-width/2)
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 

    // right - YZ-plane
    // coord format: (length/2,height or 0,+-width/2)
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // bottom left
    
    // Inner walls:
    float wCoordInner = width / 2 - thickness;
    float lCoordInner = length / 2 - thickness;
    float hCoordInner = height;

    // bottom - XZ-plane
    // coord format: (+-length/2,0,+-width/2)

    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom left
    tank.push_back(cy::Vec3f(lCoordInner, thickness, -wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom left

    // front - XY-plane
    // coord format: (+-length/2,height or thickness,width/2)
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, wCoordInner)); // bottom left 

    // back - XY-plane
    // coord format: (+-length/2,height or thickness,-width/2)
    tank.push_back(cy::Vec3f(lCoordInner, thickness, -wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordInner, thickness, -wCoordInner)); // bottom left 

    // left - YZ-plane
    // coord format: (-length/2,height or thickness,+-width/2)
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom left 

    // right - YZ-plane
    // coord format: (length/2,height or thickness,+-width/2)
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordInner, thickness, -wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // bottom left

    // Upper walls - XZ-plane
    // bottom
    // coord format: (length/2,height or thickness,+-width/2)
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left
    
    // right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left

    // top
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left

    // left
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left
}

static void SetUpWaterSurface(float width, float length, float height) {
    //tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left 
    //tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // bottom right
    //tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top right
    //tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top right
    //tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, -wCoordInner)); // top left
    //tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left
}

static void SetUpCamera() {
    tankViewMatrix = cy::Matrix4f::View(camPos, camTar, cy::Vec3f(0.0f, 1.0f, 0.0f));
    tankProjMatrix = cy::Matrix4f::Perspective(D2R(60.0f), viewWidth / viewHeight, 0.1f, 1000.0f);
    tankModelMatrix = cy::Matrix4f::RotationZ(D2R(0.0f));
    tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
    tankMv = tankViewMatrix * tankModelMatrix;
    tankMn = tankMv.GetSubMatrix3();
    tankMn.Invert();
    tankMn.Transpose();
}

static void InitPrograms() {
    tankProg.BuildFiles("tank_vert.txt", "tank_frag.txt");
    tankProg.Bind();
    tankProg["mvp"] = tankMvp;
    tankProg["mv"] = tankMv;
    tankProg["mn"] = tankMn;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &tankFaceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tankFaceBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        tank.size() * sizeof(cy::Vec3f),
        &tank[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tankFaceBuffer);

    tankProg.SetAttribBuffer("pos", tankFaceBuffer, 3);
}