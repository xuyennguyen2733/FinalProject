#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cyCodebase/cyMatrix.h>
#include <cyCodebase/cyVector.h>
#include <cyCodebase/cyTriMesh.h>
#include <cyCodebase/cyGL.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <lodepng/lodepng.h>
#include <lodepng/lodepng.cpp>

float viewHeight = 540;
float viewWidth = 960;

// Meshes
std::vector<cy::Vec3f> tank;
std::vector<cy::Vec2f> tankTexCoord;
std::vector<cy::Vec3f> tankNormCoord;

std::vector<cy::Vec3f> waterSurface;
std::vector<cy::Vec2f> waterTexCoord;
std::vector<cy::Vec3f> waterSurfaceNorm;

// Mesh attributes
float height;
float width;
float length;
float thickness;

// Texture attributes
std::vector<unsigned char> tankTexture;
unsigned tankTexWidth, tankTexHeight;
std::string tankTexName;

// Camera
cy::Vec3f camPos = cy::Vec3f(0.0f, 20.0f, 30.0f);
cy::Vec3f camTar = cy::Vec3f(0.0f, 0.0f, 0.0f);

// Light
cy::Vec3f light = cy::Vec3f(20.0f, 20.0f, 50.0f);

// Programs
cy::GLSLProgram tankProg;
cy::GLSLProgram waterProg;

// Matrices
cy::Matrix4f tankViewMatrix;
cy::Matrix4f tankProjMatrix;
cy::Matrix4f tankModelMatrix;
cy::Matrix4f tankMvp;
cy::Matrix4f tankMv;
cy::Matrix3f tankMn;
cy::Matrix4f waterViewMatrix;
cy::Matrix4f waterProjMatrix;
cy::Matrix4f waterModelMatrix;
cy::Matrix4f waterMvp;
cy::Matrix4f waterMv;
cy::Matrix3f waterMn;

// Buffers
GLuint tankFaceBuffer;
GLuint tankTexBuffer;
GLuint tankNormBuffer;
GLuint waterFaceBuffer;
GLuint waterNormBuffer;
GLuint waterTexBuffer;

// Vertex Array Objects
GLuint tankVertexArrayObject;
GLuint waterVertexArrayObject;

// Texture IDs
GLuint tankTexID;

// Mouse statistics
float oldX = 0;
float oldY = 0;
float centerX = viewWidth / 2;
float centerY = viewHeight / 2;
int mouseButton = 0;
int state = 0;
bool ctrlDown = false;

int main(int argc, char** argv);

static double D2R(int degrees);
static cy::Vec3f Mat3Vec3Mul(cy::Matrix3f m, cy::Vec3f v);
static void SetUpWaterTank(float width, float length, float height, float thickness);
static void SetUpWaterSurface(float width, float length, float height);
static void SetUpCamera();
static void DrawTank();
static void DrawWater();
static void InitPrograms();
static void LoadTextures();
static void LoadMeshes();
static cy::Vec3f CalcNorm(cy::Vec3f normAt, cy::Vec3f dir1, cy::Vec3f dir2);


void display() {
    //// Clear the viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    DrawTank();
    DrawWater();

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
    glBindVertexArray(tankVertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, tank.size());
}

static void DrawWater() {
    glUseProgram(waterProg.GetID());
    waterProg.Bind();
    glBindVertexArray(waterVertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, waterSurface.size());
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
            if (ctrlDown) {
                camPos *= 1.02;
                SetUpCamera();
            }
            else {
                tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, -1.0f));
                tankMv = tankViewMatrix * tankModelMatrix;
                tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
                tankMn = tankMv.GetSubMatrix3();
                tankMn.Invert();
                tankMn.Transpose();

                waterViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, -1.0f));
                waterMv = waterViewMatrix * waterModelMatrix;
                waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
                waterMn = waterMv.GetSubMatrix3();
                waterMn.Invert();
                waterMn.Transpose();
            }
            //
            
        }
        else 
        {
            if (ctrlDown) {
                camPos *= 0.98;
                SetUpCamera();
            }
            else {
                tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, 1.0f));
                tankMv = tankViewMatrix * tankModelMatrix;
                tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
                tankMn = tankMv.GetSubMatrix3();
                tankMn.Invert();
                tankMn.Transpose();

                waterViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, 1.0f));
                waterMv = waterViewMatrix * waterModelMatrix;
                waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
                waterMn = waterMv.GetSubMatrix3();
                waterMn.Invert();
                waterMn.Transpose();
            }
            //
            
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
        cy::Matrix3f Ry = cy::Matrix3f::RotationX(rotAngleY);
        if (ctrlDown) {
            camPos = Mat3Vec3Mul(Rx, camPos);
            camPos = Mat3Vec3Mul(Ry, camPos);
            SetUpCamera();
        }
        else {
            tankViewMatrix *= tankViewMatrix.RotationY(D2R(2.0f) * directionX);
            tankViewMatrix *= Ry;
            tankMv = tankViewMatrix * tankModelMatrix;
            tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
            tankMn = tankMv.GetSubMatrix3();
            tankMn.Invert();
            tankMn.Transpose();

            waterViewMatrix *= waterViewMatrix.RotationY(D2R(2.0f) * directionX);
            waterViewMatrix *= Ry;
            waterMv = waterViewMatrix * waterModelMatrix;
            waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
            waterMn = waterMv.GetSubMatrix3();
            waterMn.Invert();
            waterMn.Transpose();
        }
    }
    
    glUseProgram(tankProg.GetID());
    tankProg["mvp"] = tankMvp;
    tankProg["mv"] = tankMv;
    tankProg["mn"] = tankMn;
    //tankProg["light"] = light;

    glUseProgram(waterProg.GetID());
    waterProg["mvp"] = waterMvp;
    waterProg["mv"] = waterMv;
    waterProg["mn"] = waterMn;
    //waterProg["light"] = light;
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
    viewHeight = y;
    viewWidth = x;
    centerX = viewWidth / 2;
    centerY = viewHeight / 2;
    glViewport(1, 1, viewWidth, viewHeight);
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
    width = 20.0f;
    length = 30.0f;
    height = 10.0f;
    thickness = 1.0f;
    //SetUpWaterTank(width, length, height, thickness);
    LoadMeshes();
    //SetUpWaterSurface(width-thickness*2,length-thickness*2,height-2.0f);
    SetUpCamera();
    InitPrograms();
    LoadTextures();

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



static void LoadMeshes() 
{
    cy::TriMesh tankMesh;
    bool success = tankMesh.LoadFromFileObj("./container.obj");
    tankMesh.ComputeBoundingBox();
    tankMesh.ComputeNormals();
    for (unsigned int i = 0; i < tankMesh.NF(); i++) {
        tank.push_back(tankMesh.V(tankMesh.F(i).v[0]));
        tank.push_back(tankMesh.V(tankMesh.F(i).v[1]));
        tank.push_back(tankMesh.V(tankMesh.F(i).v[2]));
    }
    for (unsigned int i = 0; i < tankMesh.NF(); i++) {
        tankTexCoord.push_back(cy::Vec2f(tankMesh.VT(tankMesh.FT(i).v[0])));
        tankTexCoord.push_back(cy::Vec2f(tankMesh.VT(tankMesh.FT(i).v[1])));
        tankTexCoord.push_back(cy::Vec2f(tankMesh.VT(tankMesh.FT(i).v[2])));
    }
    for (unsigned int i = 0; i < tankMesh.NF(); i++) {
        tankNormCoord.push_back(tankMesh.VN(tankMesh.FN(i).v[0]));
        tankNormCoord.push_back(tankMesh.VN(tankMesh.FN(i).v[1]));
        tankNormCoord.push_back(tankMesh.VN(tankMesh.FN(i).v[2]));
    }

    cy::TriMesh waterMesh;
    success = waterMesh.LoadFromFileObj("./water_surface.obj");
    waterMesh.ComputeBoundingBox();
    waterMesh.ComputeNormals();
    for (unsigned int i = 0; i < waterMesh.NF(); i++) {
        waterSurface.push_back(waterMesh.V(waterMesh.F(i).v[0]));
        waterSurface.push_back(waterMesh.V(waterMesh.F(i).v[1]));
        waterSurface.push_back(waterMesh.V(waterMesh.F(i).v[2]));
    }
    for (unsigned int i = 0; i < waterMesh.NF(); i++) {
        waterTexCoord.push_back(cy::Vec2f(waterMesh.VT(waterMesh.FT(i).v[0])));
        waterTexCoord.push_back(cy::Vec2f(waterMesh.VT(waterMesh.FT(i).v[1])));
        waterTexCoord.push_back(cy::Vec2f(waterMesh.VT(waterMesh.FT(i).v[2])));
    }
    for (unsigned int i = 0; i < waterMesh.NF(); i++) {
        waterSurfaceNorm.push_back(waterMesh.VN(waterMesh.FN(i).v[0]));
        waterSurfaceNorm.push_back(waterMesh.VN(waterMesh.FN(i).v[1]));
        waterSurfaceNorm.push_back(waterMesh.VN(waterMesh.FN(i).v[2]));
    }
}

static void SetUpWaterSurface(float width, float length, float height) {
    float wCoord = width / 2;
    float lCoord = length / 2;
    float hCoord = height;
    waterSurface.push_back(cy::Vec3f(-lCoord, hCoord, wCoord)); // bottom left 
    waterSurface.push_back(cy::Vec3f(lCoord, hCoord, wCoord)); // bottom right
    waterSurface.push_back(cy::Vec3f(lCoord, hCoord, -wCoord)); // top right
    waterSurface.push_back(cy::Vec3f(lCoord, hCoord, -wCoord)); // top right
    waterSurface.push_back(cy::Vec3f(-lCoord, hCoord, -wCoord)); // top left
    waterSurface.push_back(cy::Vec3f(-lCoord, hCoord, wCoord)); // bottom left

    cy::Vec3f botLeftNorm = CalcNorm(cy::Vec3f(-lCoord, hCoord, wCoord), cy::Vec3f(lCoord, hCoord, wCoord), cy::Vec3f(-lCoord, hCoord, -wCoord));
    cy::Vec3f botRightNorm = CalcNorm(cy::Vec3f(lCoord, hCoord, wCoord), cy::Vec3f(-lCoord, hCoord, wCoord), cy::Vec3f(lCoord, hCoord, -wCoord));
    cy::Vec3f topLeftNorm = CalcNorm(cy::Vec3f(-lCoord, hCoord, -wCoord), cy::Vec3f(lCoord, hCoord, -wCoord), cy::Vec3f(-lCoord, hCoord, wCoord));
    cy::Vec3f topRightNorm = CalcNorm(cy::Vec3f(lCoord, hCoord, -wCoord), cy::Vec3f(-lCoord, hCoord, -wCoord), cy::Vec3f(lCoord, hCoord, wCoord));

    //normals
    waterSurfaceNorm.push_back(botLeftNorm); // bottom left 
    waterSurfaceNorm.push_back(botRightNorm); // bottom right
    waterSurfaceNorm.push_back(topRightNorm); // top right
    waterSurfaceNorm.push_back(topRightNorm); // top right
    waterSurfaceNorm.push_back(topLeftNorm); // top left
    waterSurfaceNorm.push_back(botLeftNorm); // bottom left
}

static void SetUpCamera() {
    // water tank
    tankViewMatrix = cy::Matrix4f::View(camPos, camTar, cy::Vec3f(0.0f, 1.0f, 0.0f));
    tankProjMatrix = cy::Matrix4f::Perspective(D2R(60.0f), viewWidth / viewHeight, 0.1f, 1000.0f);
    tankModelMatrix = cy::Matrix4f::RotationZ(D2R(0.0f));
    tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
    tankMv = tankViewMatrix * tankModelMatrix;
    tankMn = tankMv.GetSubMatrix3();
    tankMn.Invert();
    tankMn.Transpose();

    // water surface
    waterViewMatrix = cy::Matrix4f::View(camPos, camTar, cy::Vec3f(0.0f, 1.0f, 0.0f));
    waterProjMatrix = cy::Matrix4f::Perspective(D2R(60.0f), viewWidth / viewHeight, 0.1f, 1000.0f);
    waterModelMatrix = cy::Matrix4f::RotationZ(D2R(0.0f));
    waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
    waterMv = waterViewMatrix * waterModelMatrix;
    waterMn = waterMv.GetSubMatrix3();
    waterMn.Invert();
    waterMn.Transpose();
}

static void InitPrograms() {
    // water tank
    tankProg.BuildFiles("tank_vert.txt", "tank_frag.txt");
    tankProg.Bind();
    tankProg["mvp"] = tankMvp;
    tankProg["mv"] = tankMv;
    tankProg["mn"] = tankMn;
    tankProg["light"] = light;

    glGenVertexArrays(1, &tankVertexArrayObject);
    glBindVertexArray(tankVertexArrayObject);

    glGenBuffers(1, &tankFaceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tankFaceBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        tank.size() * sizeof(cy::Vec3f),
        &tank[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tankFaceBuffer);

    tankProg.SetAttribBuffer("pos", tankFaceBuffer, 3);

    glGenBuffers(1, &tankTexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tankTexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        tankTexCoord.size() * sizeof(cy::Vec3f),
        &tankTexCoord[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tankTexBuffer);

    tankProg.SetAttribBuffer("txc", tankTexBuffer, 2);

    glGenBuffers(1, &tankNormBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tankNormBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        tankNormCoord.size() * sizeof(cy::Vec3f),
        &tankNormCoord[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tankNormBuffer);

    tankProg.SetAttribBuffer("norm", tankNormBuffer, 3);

    // water surface
    waterProg.BuildFiles("water_vert.txt", "water_frag.txt");
    waterProg.Bind();
    waterProg["mvp"] = waterMvp;
    waterProg["mv"] = waterMv;
    waterProg["mn"] = waterMn;
    waterProg["light"] = light;

    glGenVertexArrays(1, &waterVertexArrayObject);
    glBindVertexArray(waterVertexArrayObject);

    glGenBuffers(1, &waterFaceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, waterFaceBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        waterSurface.size() * sizeof(cy::Vec3f),
        &waterSurface[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, waterFaceBuffer);

    waterProg.SetAttribBuffer("pos", waterFaceBuffer, 3);

    glGenBuffers(1, &waterTexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, waterTexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        waterTexCoord.size() * sizeof(cy::Vec3f),
        &waterTexCoord[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, waterTexBuffer);

    waterProg.SetAttribBuffer("txc", waterTexBuffer, 2);

    glGenBuffers(1, &waterNormBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, waterNormBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        waterSurfaceNorm.size() * sizeof(cy::Vec3f),
        &waterSurfaceNorm[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, waterNormBuffer);

    waterProg.SetAttribBuffer("norm", waterNormBuffer, 3);
}

static void LoadTextures() {
    tankTexName = "./tile.png";
    bool textureLoaded = lodepng::decode(tankTexture, tankTexWidth, tankTexHeight, tankTexName);
    glGenTextures(1, &tankTexID);
    glBindTexture(GL_TEXTURE_2D, tankTexID);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        tankTexWidth,
        tankTexHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        &tankTexture[0]
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    GLint sampler = glGetUniformLocation(tankProg.GetID(), "textureID");
    glUseProgram(tankProg.GetID());
    glUniform1i(sampler, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tankTexID);

    GLint waterSampler = glGetUniformLocation(waterProg.GetID(), "textureID");
    glUseProgram(waterProg.GetID());
    glUniform1i(waterSampler, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tankTexID);

}

static cy::Vec3f CalcNorm(cy::Vec3f normAt, cy::Vec3f dir1, cy::Vec3f dir2) {
    cy::Vec3f norm = (dir1 - normAt).Cross(dir2 - normAt);
    norm.Normalize();
    return norm;
}

static void SetUpWaterTank(float width, float length, float height, float thickness) {
    // length along the X axis
    // height along the Y axis
    // width along the Z axis

    // Outer walls:

    float wCoordOuter = width / 2;
    float lCoordOuter = length / 2;
    float hCoordOuter = height;
    float thicknessTexCoord = thickness / length;

    // bottom - XZ-plane
    // coord format: (+-length/2,0,+-width/2)
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, -1.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, -1.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, -1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, -1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, -1.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, -1.0f, 0.0)); // bottom left


    // bottom texture coordinates
    tankTexCoord.push_back(cy::Vec2f(0.0f, width / length)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(1.0f, width / length)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(1.0f, 0.0f)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f, 0.0f)); // top right
    tankTexCoord.push_back(cy::Vec2f(0.0f, 0.0f)); // top left
    tankTexCoord.push_back(cy::Vec2f(0.0f, width / length)); // bottom left

    // front - XY-plane
    // coord format: (+-length/2,height or 0,width/2)
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // bottom left 

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // bottom left

    // front texture coordinates
    tankTexCoord.push_back(cy::Vec2f(0.0f, 0.0f)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(1.0f, 0.0f)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(0.0f, 0.0f)); // bottom left

    // back - XY-plane
    // coord format: (+-length/2,height or 0,-width/2)
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // bottom left

    // back texture coordinates
    tankTexCoord.push_back(cy::Vec2f(1.0f, 0.0f)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(0.0f, 0.0f)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(1.0f, 0.0f)); // bottom left

    // left - YZ-plane
    // coord format: (-length/2,height or 0,+-width/2)
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, 0.0f, -wCoordOuter)); // bottom left 

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // bottom left

    // left texture coordinates
    tankTexCoord.push_back(cy::Vec2f(width / length, 0.0f)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(0.0f, 0.0f)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(width / length, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(width / length, 0.0f)); // bottom left

    // right - YZ-plane
    // coord format: (length/2,height or 0,+-width/2)
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // top right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, 0.0f, wCoordOuter)); // bottom left

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // bottom left

    // right texture coordinates
    tankTexCoord.push_back(cy::Vec2f(1.0f, 0.0f)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(1 - width / length, 0.0f)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(1 - width / length, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1 - width / length, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(1.0f, 0.0f)); // bottom left

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

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left

    // bottom texture coordinates
    tankTexCoord.push_back(cy::Vec2f(-lCoordInner / length + 0.5, -wCoordInner / width + 0.5)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(lCoordInner / length + 0.5, -wCoordInner / width + 0.5)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(lCoordInner / length + 0.5, wCoordInner / width + 0.5)); // top right
    tankTexCoord.push_back(cy::Vec2f(lCoordInner / length + 0.5, wCoordInner / width + 0.5)); // top right
    tankTexCoord.push_back(cy::Vec2f(-lCoordInner / length + 0.5, wCoordInner / width + 0.5)); // top left
    tankTexCoord.push_back(cy::Vec2f(-lCoordInner / length + 0.5, -wCoordInner / width + 0.5)); // bottom left

    // front - XY-plane
    // coord format: (+-length/2,height or thickness,width/2)
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, wCoordInner)); // bottom left 

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, -1.0)); // bottom left

    // front texture coordinates
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, thicknessTexCoord)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, thicknessTexCoord)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, thicknessTexCoord)); // bottom left

    // back - XY-plane
    // coord format: (+-length/2,height or thickness,-width/2)
    tank.push_back(cy::Vec3f(lCoordInner, thickness, -wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordInner, thickness, -wCoordInner)); // bottom left 

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 0.0f, 1.0)); // bottom left

    // back texture coordinates
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, thicknessTexCoord)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, thicknessTexCoord)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, thicknessTexCoord)); // bottom left

    // left - YZ-plane
    // coord format: (-length/2,height or thickness,+-width/2)
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordInner, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordInner, thickness, -wCoordInner)); // bottom left 

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(1.0, 0.0f, 0.0)); // bottom left

    // left texture coordinates
    tankTexCoord.push_back(cy::Vec2f(width / length - thicknessTexCoord, thicknessTexCoord)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, thicknessTexCoord)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(width / length - thicknessTexCoord, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(width / length - thicknessTexCoord, thicknessTexCoord)); // bottom left

    // right - YZ-plane
    // coord format: (length/2,height or thickness,+-width/2)
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordInner, thickness, -wCoordInner)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordInner, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordInner, thickness, wCoordInner)); // bottom left

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(-1.0, 0.0f, 0.0)); // bottom left

    // right texture coordinates
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, thicknessTexCoord)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(1 - width / length + thicknessTexCoord, thicknessTexCoord)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(1 - width / length + thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1 - width / length + thicknessTexCoord, height / length)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length)); // top left
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, thicknessTexCoord)); // bottom left

    // Upper walls - XZ-plane
    // Adjacent to front
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left

    // front texture coordinates
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length + thicknessTexCoord)); // top left
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // bottom left

    // Adjacent to right
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, wCoordOuter)); // bottom left

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left

    // texture coordinates
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(1.0f - width / length, height / length)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(1.0f - width / length + thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f - width / length + thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length + thicknessTexCoord)); // top left
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // bottom left

    // adjacent to back
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, -wCoordInner)); // top right
    tank.push_back(cy::Vec3f(lCoordInner, hCoordOuter, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left

    // texture coordinates
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(1.0f - thicknessTexCoord, height / length + thicknessTexCoord)); // top left
    tankTexCoord.push_back(cy::Vec2f(1.0f, height / length)); // bottom left

    // adjacent to left
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left 
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, wCoordOuter)); // bottom right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, wCoordInner)); // top right
    tank.push_back(cy::Vec3f(-lCoordInner, hCoordOuter, -wCoordInner)); // top left
    tank.push_back(cy::Vec3f(-lCoordOuter, hCoordOuter, -wCoordOuter)); // bottom left

    //normal Coordinates
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top right
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // top left
    tankNormCoord.push_back(cy::Vec3f(0.0, 1.0f, 0.0)); // bottom left

    // texture coordinates
    tankTexCoord.push_back(cy::Vec2f(width / length, height / length)); // bottom left
    tankTexCoord.push_back(cy::Vec2f(0.0f, height / length)); // bottom right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(thicknessTexCoord, height / length + thicknessTexCoord)); // top right
    tankTexCoord.push_back(cy::Vec2f(width / length - thicknessTexCoord, height / length + thicknessTexCoord)); // top left
    tankTexCoord.push_back(cy::Vec2f(width / length, height / length)); // bottom left
}