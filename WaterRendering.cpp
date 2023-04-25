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
// Xuyen Nguyen
// Devin White

float viewHeight = 540;
float viewWidth = 960;
int limit = 0;

// Meshes
std::vector<cy::Vec3f> tank;
std::vector<cy::Vec2f> tankTexCoord;
std::vector<cy::Vec3f> tankNormCoord;

std::vector<cy::Vec3f> waterSurface;
std::vector<cy::Vec2f> waterTexCoord;
std::vector<cy::Vec3f> waterSurfaceNorm;

std::vector<cy::Vec3f> waterPlane;
std::vector<cy::Vec3f> waterPlaneNorm;
std::vector<cy::Vec2f> waterPlaneTexCoord;

std::vector<cy::Vec3f> environmentVertCoords; // environment map


// Mesh attributes
float height;
float width;
float length;
float thickness;

// wave attributes
int timer = 0;
float theta = 0.0f; //between 0 and 360
int increment = 1; // 1 or -1

// Texture attributes
std::vector<unsigned char> tankTexture;
std::vector<unsigned char> tankNormalTex;
std::vector<unsigned char> tankDepthTex;
std::vector<unsigned char> waterTexture;
std::vector<unsigned char> waterNormalTex;
std::vector<unsigned char> waterDepthTex;
unsigned tankTexWidth, tankTexHeight, waterTexWidth, waterTexHeight, waterNormalWidth, 
waterNormalHeight, waterDepthWidth, waterDepthHeight,
tankNormalWidth, tankNormalHeight, tankDepthWidth, tankDepthHeight;
std::string tankTexName;
std::string tankNormalName;
std::string tankDepthName;
std::string waterTexName;
std::string waterNormalName;
std::string waterDepthName;
float texCoordDisp = 0.0f; // displacement of texture mapping
float waveScale = 0.0f; // degrees at which the water changes

// Camera
cy::Vec3f camPos = cy::Vec3f(0.0f, 10.0f, 30.0f);
cy::Vec3f camTar = cy::Vec3f(0.0f, 0.0f, 0.0f);

// Light
cy::Vec3f light = cy::Vec3f(10.0f, 50.0f, 50.0f);

// Programs
cy::GLSLProgram tankProg;
cy::GLSLProgram waterProg;
cy::GLSLProgram environmentProg;

//  environment Map
cy::GLTextureCubeMap envmap;

cy::GLRenderTexture2D renderBuffer;

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
cy::Matrix4f waterRefModelMatrix;
cy::Matrix4f waterMvp;
cy::Matrix4f waterMv;
cy::Matrix3f waterMn;
cy::Matrix4f environmentViewMatrix;
cy::Matrix4f environmentProjMatrix;
cy::Matrix4f environmentModelMatrix;
cy::Matrix4f environmentMvp;
cy::Matrix4f environmentMv;
cy::Matrix3f environmentMn;
cy::Matrix4f lightMatrix;

cy::Matrix4f inittankViewMatrix;
cy::Matrix4f inittankProjMatrix;
cy::Matrix4f inittankModelMatrix;
cy::Matrix4f inittankMvp;
cy::Matrix4f inittankMv;
cy::Matrix3f inittankMn;
cy::Matrix4f initwaterViewMatrix;
cy::Matrix4f initwaterProjMatrix;
cy::Matrix4f initwaterModelMatrix;
cy::Matrix4f initwaterRefModelMatrix;
cy::Matrix4f initwaterMvp;
cy::Matrix4f initwaterMv;
cy::Matrix3f initwaterMn;
cy::Matrix4f initenvironmentViewMatrix;
cy::Matrix4f initenvironmentProjMatrix;
cy::Matrix4f initenvironmentModelMatrix;
cy::Matrix4f initenvironmentMvp;
cy::Matrix4f initenvironmentMv;
cy::Matrix3f initenvironmentMn;


// Buffers
GLuint tankFaceBuffer;
GLuint tankTexBuffer;
GLuint tankNormBuffer;
GLuint waterFaceBuffer;
GLuint waterNormBuffer;
GLuint waterTexBuffer;
GLuint environmentBuffer;

// Vertex Array Objects
GLuint tankVertexArrayObject;
GLuint waterVertexArrayObject;
GLuint environmentVertexArrayObject;

// Texture IDs
GLuint tankTexID;
GLuint tankDepthMap;
GLuint tankNormalMap;
GLuint waterTexID;
GLuint waterDepthMap;
GLuint waterNormalMap;

// Mouse statistics
float oldX = 0;
float oldY = 0;
float centerX = viewWidth / 2;
float centerY = viewHeight / 2;
int mouseButton = 0;
int state = 0;
bool ctrlDown = false;
float layers = 64.0f;
bool hasDispMap = true;
bool isReflective = true;
bool isTransparent = true;

int main(int argc, char** argv);

static float D2R(int degrees);
static cy::Vec3f Mat3Vec3Mul(cy::Matrix3f m, cy::Vec3f v);
static void SetUpWaterTank(float width, float length, float height, float thickness);
static void SetUpWaterSurface(float width, float length, float height);
static void SetUpCamera();
static void DrawTank();
void DrawEnvironment();
static void DrawWater();
void resetPosition();
static void InitPrograms();
static void LoadTextures();
static void LoadMeshes();
static cy::Vec3f CalcNorm(cy::Vec3f normAt, cy::Vec3f dir1, cy::Vec3f dir2);
void initRenderBuffer();

void display() 
{
    
    // 250
    if (timer % 100 == 0)
    {
        glUseProgram(waterProg.GetID());
        waterProg["texCoordDisp"] = texCoordDisp;
        texCoordDisp++;
        if (theta < 60)
        {
            increment = 1;
        }
        else if (theta > 120)
        {
            increment = -1;
        }
        if (waveScale < 90) {
            waveScale+=1.0f;
        }
        theta += increment;
        
        waterProg["sinTheta"] = cos(theta);
        float scale = cos(D2R(waveScale));
        if (scale < 1.0e-05) {
            scale = 0.0f;
        }
        waterProg["waveScale"] = scale;

    }
    if (timer > 2500)
    {
        timer = 1;
    }
    timer++;
    if (texCoordDisp == waterTexWidth) {
        texCoordDisp = 0.0f;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
    DrawEnvironment();
    glDepthMask(GL_TRUE);
    renderBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(tankProg.GetID());
    DrawTank();
    renderBuffer.Unbind();
    renderBuffer.BuildTextureMipmaps();
    // Clear the viewport
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    DrawWater();
    DrawTank();
    

    // Swap buffers
        /**
         * Double buffering:
         * - Front buffer: only shows fully rendered images
         * - Secondary (back) buffer not visible
         * - Renders images in back buffer then swap the buffers when done (back becomes front and vice versa)*/
    glutSwapBuffers();

}

void DrawEnvironment()
{
    glUseProgram(environmentProg.GetID());
    environmentProg.Bind();
    glBindVertexArray(environmentVertexArrayObject);
    //glBindBuffer(GL_ARRAY_BUFFER, environmentBuffer);
    glDrawArrays(GL_TRIANGLES, 0, environmentVertCoords.size());
}

static void DrawTank() {
    glUseProgram(tankProg.GetID());
    tankProg.Bind();
    glBindVertexArray(tankVertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, tank.size());
}


static void DrawWater() {
    glUseProgram(waterProg.GetID());
    waterProg["isReflective"] = isReflective;
    waterProg["isTransparent"] = isTransparent;
    glActiveTexture(GL_TEXTURE4);
    //waterProg.Bind();
    GLuint sampler = glGetUniformLocation(waterProg.GetID(), "text");
    glUniform1i(sampler, 4);
    glBindVertexArray(waterVertexArrayObject);
    glDrawArrays(GL_PATCHES, 0, 4);
}

void keypress(unsigned char key, int x, int y) {
    // Handle keyboard input hereopenScene
    switch (key)
    {
    case 27: //ESC
        glutLeaveMainLoop();
        break;
    case 32: //space
        resetPosition();

        break;
    case 49:
        isTransparent = !isTransparent;
        break;
    case 50:
        isReflective = !isReflective;
        break;
    }   

    glutPostRedisplay();
}

void resetPosition()
{
    limit = 0;
    tankViewMatrix = inittankViewMatrix;
    tankProjMatrix = inittankProjMatrix;
    tankModelMatrix = inittankModelMatrix;
    tankMvp = inittankMvp;
    tankMv = inittankMv;
    tankMn = inittankMn;
    waterViewMatrix = initwaterViewMatrix;
    waterProjMatrix = initwaterProjMatrix;
    waterModelMatrix = initwaterModelMatrix;
    waterRefModelMatrix = initwaterRefModelMatrix;
    waterMvp = initwaterMvp;
    waterMv = initwaterMv;
    waterMn = initwaterMn;
    environmentViewMatrix = initenvironmentViewMatrix;
    environmentProjMatrix = initenvironmentProjMatrix;
    environmentModelMatrix = initenvironmentModelMatrix;
    environmentMvp = initenvironmentMvp;
    environmentMv = initenvironmentMv;
    environmentMn = initenvironmentMn;

    glUseProgram(tankProg.GetID());
    tankProg.Bind();
    tankProg["mvp"] = inittankMvp;
    tankProg["mv"] = inittankMv;
    tankProg["mn"] = inittankMn;
    //tankProg["light"] = light;
    tankProg["lightMatrix"] = lightMatrix;

    glUseProgram(waterProg.GetID());
    waterProg.Bind();
    waterProg["mvp"] = initwaterMvp;
    waterProg["mv"] = initwaterMv;
    waterProg["mn"] = initwaterMn;
    waterProg["cameraPos"] = camPos;
    waterProg["modelMatrix"] = initwaterRefModelMatrix;
    waterProg["texCoordMvp"] = initwaterMvp;
    //waterProg["light"] = light;
    waterProg["lightMatrix"] = lightMatrix;

    glUseProgram(environmentProg.GetID());
    environmentProg.Bind();
    environmentProg["mvp"] = initenvironmentMvp;
    environmentProg["mv"] = initenvironmentMv;
    environmentProg["mn"] = initenvironmentMn;

    oldX = 0;
    oldY = 0;
    centerX = viewWidth / 2;
    centerY = viewHeight / 2;
    mouseButton = 0;
    state = 0;
    ctrlDown = false;
    layers = 64.0f;
    hasDispMap = true;
    isReflective = true;
    isTransparent = true;
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
        if (layers > 1) layers--;
        break;
    case GLUT_KEY_RIGHT:
        if (layers < 64) layers++;
        //do something here
        break;
    }
    if (hasDispMap) {
        glUseProgram(waterProg.GetID());
        waterProg["layer"] = layers;
    }
}

void click(int button, int state, int x, int y) {
    // Handle mouse buttons here
    mouseButton = button;
    oldX = x;
    oldY = y;
}

void drag(int x, int y) 
{
    // Handle mouse motion here
    // while a button is down
    bool inViewPort = (x >= 0 && x <= viewWidth && y >= 0 && y <= viewHeight);

    if (mouseButton == GLUT_RIGHT_BUTTON && inViewPort) 
    {
        bool towardCenter = (x <= oldX && x > centerX) || (x >= oldX && x < centerX);
            if (towardCenter) 
            {
                if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
                {
                    tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, -0.2f, -1.0f));
                    tankMv = tankViewMatrix * tankModelMatrix;
                    tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
                    tankMn = tankMv.GetSubMatrix3();
                    tankMn.Invert();
                    tankMn.Transpose();

                    waterViewMatrix.AddTranslation(cy::Vec3f(0.0f, -0.2f, -1.0f));
                    waterMv = waterViewMatrix * waterModelMatrix;
                    waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
                    waterMn = waterMv.GetSubMatrix3();
                    waterMn.Invert();
                    waterMn.Transpose();
                }
                else 
                {
                    if (limit > 0)
                    {
                        tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, -0.6f));
                        tankMv = tankViewMatrix * tankModelMatrix;
                        tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
                        tankMn = tankMv.GetSubMatrix3();
                        tankMn.Invert();
                        tankMn.Transpose();

                        waterViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, -0.6f));
                        waterMv = waterViewMatrix * waterModelMatrix;
                        waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
                        waterMn = waterMv.GetSubMatrix3();
                        waterMn.Invert();
                        waterMn.Transpose();

                        environmentViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, -0.1f));
                        environmentMv = environmentViewMatrix * environmentModelMatrix;
                        environmentMvp = environmentProjMatrix * environmentViewMatrix * environmentModelMatrix;
                        environmentMn = environmentMv.GetSubMatrix3();
                        environmentMn.Invert();
                        environmentMn.Transpose();
                        limit--;
                    }

                }

            }
            else
            {
                if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
                {
                    tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.2f, 1.0f));
                    tankMv = tankViewMatrix * tankModelMatrix;
                    tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
                    tankMn = tankMv.GetSubMatrix3();
                    tankMn.Invert();
                    tankMn.Transpose();

                    waterViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.2f, 1.0f));
                    waterMv = waterViewMatrix * waterModelMatrix;
                    waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
                    waterMn = waterMv.GetSubMatrix3();
                    waterMn.Invert();
                    waterMn.Transpose();
                }
                else 
                {
                    if (limit <= 100)
                    {
                        tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, 0.6f));
                        tankMv = tankViewMatrix * tankModelMatrix;
                        tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
                        tankMn = tankMv.GetSubMatrix3();
                        tankMn.Invert();
                        tankMn.Transpose();

                        waterViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, 0.6f));
                        waterMv = waterViewMatrix * waterModelMatrix;
                        waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
                        waterMn = waterMv.GetSubMatrix3();
                        waterMn.Invert();
                        waterMn.Transpose();

                        environmentViewMatrix.AddTranslation(cy::Vec3f(0.0f, 0.0f, 0.1f));
                        environmentMv = environmentViewMatrix * environmentModelMatrix;
                        environmentMvp = environmentProjMatrix * environmentViewMatrix * environmentModelMatrix;
                        environmentMn = environmentMv.GetSubMatrix3();
                        environmentMn.Invert();
                        environmentMn.Transpose();
                        limit++;

                    }
                }
            }
    }
    else if (mouseButton == GLUT_LEFT_BUTTON && inViewPort) 
    {

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
        if (glutGetModifiers() == GLUT_ACTIVE_ALT)
        {
            environmentViewMatrix *= environmentViewMatrix.RotationY(D2R(2.0f) * directionX);
            //waterViewMatrix *= Ry;
            environmentMv = waterViewMatrix * environmentModelMatrix;
            environmentMvp = environmentProjMatrix * environmentViewMatrix * environmentModelMatrix;
            waterMn = environmentMv.GetSubMatrix3();
            environmentMn.Invert();
            environmentMn.Transpose();

            waterRefModelMatrix *= waterRefModelMatrix.RotationZ(D2R(2.0f) * directionX);
            waterRefModelMatrix *= waterRefModelMatrix.RotationY(D2R(2.0f) * directionX);
        }
        else if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
        {
            lightMatrix *= lightMatrix.RotationY(D2R(-2.0f) * directionX);
        }
        else
        {
            if (ctrlDown) 
            {
                camPos = Mat3Vec3Mul(Rx, camPos);
                //camPos = Mat3Vec3Mul(Ry, camPos);
                SetUpCamera();
            }
            else
            {
                tankViewMatrix *= tankViewMatrix.RotationY(D2R(2.0f) * directionX);
                //tankViewMatrix *= Ry;
                tankMv = tankViewMatrix * tankModelMatrix;
                tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
                tankMn = tankMv.GetSubMatrix3();
                tankMn.Invert();
                tankMn.Transpose();

                waterViewMatrix *= waterViewMatrix.RotationY(D2R(2.0f) * directionX);
                //waterViewMatrix *= Ry;
                waterMv = waterViewMatrix * waterModelMatrix;
                waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
                waterMn = waterMv.GetSubMatrix3();
                waterMn.Invert();
                waterMn.Transpose();

                environmentViewMatrix *= environmentViewMatrix.RotationY(D2R(2.0f) * directionX);
                //waterViewMatrix *= Ry;
                environmentMv = waterViewMatrix * environmentModelMatrix;
                environmentMvp = environmentProjMatrix * environmentViewMatrix * environmentModelMatrix;
                waterMn = environmentMv.GetSubMatrix3();
                environmentMn.Invert();
                environmentMn.Transpose();

                waterRefModelMatrix *= waterRefModelMatrix.RotationZ(D2R(2.0f) * directionX);
            }
        }
    }
    
    glUseProgram(tankProg.GetID());
    tankProg["mvp"] = tankMvp;
    tankProg["mv"] = tankMv;
    tankProg["mn"] = tankMn;
    //tankProg["light"] = light;
    tankProg["lightMatrix"] = lightMatrix;

    glUseProgram(waterProg.GetID());
    waterProg["mvp"] = waterMvp;
    waterProg["mv"] = waterMv;
    waterProg["mn"] = waterMn;
    waterProg["cameraPos"] = camPos;
    waterProg["modelMatrix"] = waterRefModelMatrix;
    waterProg["texCoordMvp"] = waterMvp;
    //waterProg["light"] = light;
    waterProg["lightMatrix"] = lightMatrix;

    glUseProgram(environmentProg.GetID());
    environmentProg["mvp"] = environmentMvp;
    environmentProg["mv"] = environmentMv;
    environmentProg["mn"] = environmentMn;

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

int main(int argc, char** argv) 
{
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
    LoadMeshes();
    SetUpCamera();
    InitPrograms();
    LoadTextures();
    initRenderBuffer();
    // Call main loop
    glutMainLoop();

    return 0;
}

static float D2R(int degrees) 
{
    return (degrees * M_PI) / 180.0f;
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
    
    // Tank
    cy::TriMesh tankMesh;
    bool success = tankMesh.LoadFromFileObj("./container.obj");
    tankMesh.ComputeBoundingBox();
    tankMesh.ComputeNormals();
    tank.reserve(tankMesh.NF());
    tankTexCoord.reserve(tankMesh.NF());
    tankNormCoord.reserve(tankMesh.NF());
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
    // Water
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

    // pos
    waterPlane.push_back(cy::Vec3f(-14.0769768, 4.91360092, 9.55753136));
    waterPlane.push_back(cy::Vec3f(14.0769768, 4.91360092, 9.55753136));
    waterPlane.push_back(cy::Vec3f(14.0769768, 4.91360092, -9.55753136));
    waterPlane.push_back(cy::Vec3f(-14.0769768, 4.91360092, -9.55753136));

    // Norms
    waterPlaneNorm.push_back(cy::Vec3f(0, 1, 0));
    waterPlaneNorm.push_back(cy::Vec3f(0, 1, 0));
    waterPlaneNorm.push_back(cy::Vec3f(0, 1, 0));
    waterPlaneNorm.push_back(cy::Vec3f(0, 1, 0));

    // texCoords
    waterPlaneTexCoord.push_back(cy::Vec2f(0.358830988 - 0.3, 0.309625000 - 0.3)); // bottom left
    waterPlaneTexCoord.push_back(cy::Vec2f(0.639760017 + 0.3, 0.309625000 - 0.3)); // bottom right
    waterPlaneTexCoord.push_back(cy::Vec2f(0.639760017 + 0.3, 0.692068994 + 0.3)); // top right
    waterPlaneTexCoord.push_back(cy::Vec2f(0.358830988 - 0.3, 0.692068994 + 0.3)); //top left


    // Environment Mapping
    cy::TriMesh environmentMesh;
    success = environmentMesh.LoadFromFileObj("./cube.obj");
    environmentMesh.ComputeNormals();
    environmentMesh.ComputeBoundingBox();
    std::vector<std::string> cubeTextureImg;
    envmap.Initialize();
    cubeTextureImg.push_back("./cubemap_posx.png");
    cubeTextureImg.push_back("./cubemap_negx.png");
    cubeTextureImg.push_back("./cubemap_posy.png");
    cubeTextureImg.push_back("./cubemap_negy.png");
    cubeTextureImg.push_back("./cubemap_posz.png");
    cubeTextureImg.push_back("./cubemap_negz.png");

    for (int i = 0; i < environmentMesh.NF(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            environmentVertCoords.push_back(environmentMesh.V(environmentMesh.F(i).v[j]));
        }
    }

    for (int i = 0; i < 6; ++i)
    {
        //Cube
        std::vector<unsigned char> cubeTexture;
        unsigned envW, envH;
        //load image from file
        lodepng::decode(cubeTexture, envW, envH, cubeTextureImg[i]);
        envmap.SetImageRGBA((cy::GLTextureCubeMap::Side)i, &cubeTexture[0], envW, envH);
    }    //set image data
    envmap.BuildMipmaps();
    envmap.SetSeamless();
    envmap.Bind(6);
}

static void SetUpWaterSurface(float width, float length, float height) 
{
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

static void SetUpCamera() 
{

    // water tank
    tankViewMatrix = cy::Matrix4f::View(camPos, camTar, cy::Vec3f(0.0f, 1.0f, 0.0f));
    tankProjMatrix = cy::Matrix4f::Perspective(D2R(60.0f), viewWidth / viewHeight, 0.1f, 1000.0f);
    tankModelMatrix = cy::Matrix4f::RotationZ(D2R(0.0f));
    tankViewMatrix.AddTranslation(cy::Vec3f(0.0f, -50.0f, -200.0f));
    tankModelMatrix.SetScale(2.0);
    tankMvp = tankProjMatrix * tankViewMatrix * tankModelMatrix;
    tankMv = tankViewMatrix * tankModelMatrix;
    tankMn = tankMv.GetSubMatrix3();
    tankMn.Invert();
    tankMn.Transpose();


    // water surface
    waterViewMatrix = cy::Matrix4f::View(camPos, camTar, cy::Vec3f(0.0f, 1.0f, 0.0f));
    waterProjMatrix = cy::Matrix4f::Perspective(D2R(60.0f), viewWidth / viewHeight, 0.1f, 1000.0f);
    waterModelMatrix = cy::Matrix4f::RotationZ(D2R(0.0f));
    waterViewMatrix.AddTranslation(cy::Vec3f(0.0f, -50.0f, -200.0f));
    waterModelMatrix.SetScale(2.0);
    waterMvp = waterProjMatrix * waterViewMatrix * waterModelMatrix;
    waterMv = waterViewMatrix * waterModelMatrix;
    waterMn = waterMv.GetSubMatrix3();
    waterMn.Invert();
    waterMn.Transpose();

    waterRefModelMatrix = waterModelMatrix;

    // Environment Map

    cy::Vec3f envCamPos = cy::Vec3f(0.0f, 0.0f, 10.0f);
    environmentViewMatrix = cy::Matrix4f::View(envCamPos, camTar, cy::Vec3f(0.0f, 1.0f, 0.0f));
    environmentProjMatrix = cy::Matrix4f::Perspective(D2R(60.0f), viewWidth / viewHeight, 0.1f, 1000.0f);
    environmentModelMatrix = cy::Matrix4f::RotationZ(D2R(0.0f));
    environmentMvp = environmentProjMatrix * environmentViewMatrix * environmentModelMatrix;
    environmentMv = environmentViewMatrix * environmentModelMatrix;
    environmentMn = environmentMv.GetSubMatrix3();
    environmentMn.Invert();
    environmentMn.Transpose();

    // light matrix
    lightMatrix = cy::Matrix4f::View(light, cy::Vec3f(0.0f, 5.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    lightMatrix.SetRotationZ(D2R(10.0));

    inittankViewMatrix = tankViewMatrix;
    inittankProjMatrix = tankProjMatrix;
    inittankModelMatrix = tankModelMatrix;
    inittankMvp = tankMvp;
    inittankMv = tankMv;
    inittankMn = tankMn;
    initwaterViewMatrix = waterViewMatrix;
    initwaterProjMatrix = waterProjMatrix;
    initwaterModelMatrix = waterModelMatrix;
    initwaterRefModelMatrix = waterRefModelMatrix;
    initwaterMvp = waterMvp;
    initwaterMv = waterMv;
    initwaterMn = waterMn;
    initenvironmentViewMatrix = environmentViewMatrix;
    initenvironmentProjMatrix = environmentProjMatrix;
    initenvironmentModelMatrix = environmentModelMatrix;
    initenvironmentMvp = environmentMvp;
    initenvironmentMv = environmentMv;
    initenvironmentMn = environmentMn;


}

static void InitPrograms() 
{
    //Environment Map
    environmentProg.BuildFiles("shaders/env_vert.txt", "shaders/env_frag.txt");
    environmentProg["mvp"] = environmentMvp;
    environmentProg.Bind();
    glGenVertexArrays(1, &environmentVertexArrayObject);
    glBindVertexArray(environmentVertexArrayObject);

    glGenBuffers(1, &environmentBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, environmentBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * environmentVertCoords.size(), &environmentVertCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, environmentBuffer);
    environmentProg.SetAttribBuffer("pos", environmentBuffer, 3);

    // water tank
    tankProg.BuildFiles("shaders/tank_vert.txt", "shaders/tank_frag.txt");
    tankProg.Bind();
    tankProg["mvp"] = tankMvp;
    tankProg["mv"] = tankMv;
    tankProg["mn"] = tankMn;
    tankProg["light"] = light;
    tankProg["lightMatrix"] = lightMatrix;

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
    waterProg.BuildFiles("shaders/water_vert.txt", "shaders/water_frag.txt", NULL, "shaders/water_tessCtrl.txt", "shaders/water_tessEval.txt");
    waterProg.Bind();
    waterProg["mvp"] = waterMvp;
    waterProg["mv"] = waterMv;
    waterProg["mn"] = waterMn;
    waterProg["light"] = light;
    waterProg["layer"] = layers;
    waterProg["cameraPos"] = camPos;
    waterProg["modelMatrix"] = waterRefModelMatrix;
    waterProg["texCoordMvp"] = waterMvp;
    waterProg["lightMatrix"] = lightMatrix;

    glGenVertexArrays(1, &waterVertexArrayObject);
    glBindVertexArray(waterVertexArrayObject);

    glGenBuffers(1, &waterFaceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, waterFaceBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        waterPlane.size() * sizeof(cy::Vec3f),
        &waterPlane[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, waterFaceBuffer);

    waterProg.SetAttribBuffer("pos", waterFaceBuffer, 3);

    glGenBuffers(1, &waterNormBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, waterNormBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        waterPlaneNorm.size() * sizeof(cy::Vec3f),
        &waterPlaneNorm[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, waterNormBuffer);

    waterProg.SetAttribBuffer("norm", waterNormBuffer, 3);

    glGenBuffers(1, &waterTexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, waterTexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
        waterPlaneTexCoord.size() * sizeof(cy::Vec3f),
        &waterPlaneTexCoord[0],
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, waterTexBuffer);

    waterProg.SetAttribBuffer("txc", waterTexBuffer, 2);

    glPatchParameteri(GL_PATCH_VERTICES, 4); 
}

static void LoadTextures() 
{
    // tank textures
    glUseProgram(tankProg.GetID());
    glActiveTexture(GL_TEXTURE0);
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
    
    glUniform1i(sampler, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, tankTexID);

    // water textures
    glUseProgram(waterProg.GetID());

    glActiveTexture(GL_TEXTURE1);
    waterTexName = "./water/water3_texture.png";
    waterNormalName = "./water/water3_normal.png";
    waterDepthName = "./water/water3_depth.png";
    waterTexWidth = viewWidth * 2.0f;
    waterTexHeight = viewHeight * 2.0f;
    textureLoaded = lodepng::decode(waterTexture, waterTexWidth, waterTexHeight, waterTexName);
    textureLoaded = lodepng::decode(waterNormalTex, waterNormalWidth, waterNormalHeight, waterNormalName);
    textureLoaded = lodepng::decode(waterDepthTex, waterDepthWidth, waterDepthHeight, waterDepthName);
    
    glGenTextures(1, &waterTexID);
    glBindTexture(GL_TEXTURE_2D, waterTexID);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        waterTexWidth,
        waterTexHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        &waterTexture[0]
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    GLuint waterSampler = glGetUniformLocation(waterProg.GetID(), "waterTexID");
    glUniform1i(waterSampler, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, waterTexID);

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &waterNormalMap);
    glBindTexture(GL_TEXTURE_2D, waterNormalMap);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        waterNormalWidth,
        waterNormalHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        &waterNormalTex[0]
    ); 
    glGenerateMipmap(GL_TEXTURE_2D);
    waterSampler = glGetUniformLocation(waterProg.GetID(), "normalMap");
    glUniform1i(waterSampler, 2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, waterNormalMap);

    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &waterDepthMap);
    glBindTexture(GL_TEXTURE_2D, waterDepthMap);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        waterDepthWidth,
        waterDepthHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        &waterDepthTex[0]
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    waterSampler = glGetUniformLocation(waterProg.GetID(), "depthMap");
    glUniform1i(waterSampler, 3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, waterDepthMap);

}


static cy::Vec3f CalcNorm(cy::Vec3f normAt, cy::Vec3f dir1, cy::Vec3f dir2) {
    cy::Vec3f norm = (dir1 - normAt).Cross(dir2 - normAt);
    norm.Normalize();
    return norm;
}

void initRenderBuffer()
{
    if (!renderBuffer.Initialize(true, //create depth buffer
        3, //RGB
        1920 * 2, //texture width
        1080 * 2 //texture height
    ))
    {
        printf("Problem initializing\n");
    }
}