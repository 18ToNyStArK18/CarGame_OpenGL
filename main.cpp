#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <string.h>
#include <SFML/Audio.hpp>

#include "file_utils.h"
GLuint ShaderProgram;


float prevtime = 0.0f;

//car variables
float CAR_X=50.0f,CAR_Y=0,CAR_Z=0;
float Velocity;
float Angle=-90.0f;
float Angle_STEP = 2.0f;
float Velocity_step = 0.01f;
const float MAX_VELOCITY = 20.f;
GLuint carBodyVAO,carBodyVBO,carWinVAO,carWinVBO;
#define NUM_NPC  20
struct NPCCar {
    float x, z;       
    float angle;      
    float r, g, b;    
};
NPCCar npcCars[NUM_NPC];

static const float NPC_COLORS[NUM_NPC][3] = {
    {1.0f, 0.2f, 0.2f},   // red
    {0.2f, 1.0f, 0.2f},   // green
    {1.0f, 0.6f, 0.0f},   // orange
    {0.8f, 0.0f, 0.8f},   // purple
    {0.0f, 0.8f, 0.8f},   // cyan
    {1.0f, 1.0f, 0.0f},   // yellow
    {1.0f, 0.4f, 0.7f},   // pink
    {0.4f, 0.8f, 0.2f},   // lime
    {0.2f, 0.4f, 1.0f},   // blue
    {1.0f, 0.9f, 0.7f},   // cream
                          // #define NUM_NPC  10
};
//track variables

GLuint trackVerticalVAO,trackVerticalVBO;
GLuint trackTopArcVBO , trackTopArcVAO , trackBottomArcVAO , trackBottomArcVBO;
const int   ARC_SEGS  = 30;        
const float ARC_CX    = 9.0f;      
const float ARC_OUTER = 9.0f;
const float ARC_INNER = 6.0f;

//for the camera
float camX = 45.0f, camY = 8.0f, camZ = -20.0f;
float camZStep = 0.5f; 
float camYaw = 0.0f;
float camPitch = -15.0f;

GLuint gModelLocation, gViewLocation, gProjectionLocation;

static void AddShader(GLuint ShaderProgram, const char* pShaderText , GLenum ShaderType){
    GLuint ShaderObj = glCreateShader(ShaderType);
    if(ShaderObj == 0)
        exit(0);
    const GLchar *p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj , 1 , p , Lengths);
    glCompileShader(ShaderObj);
    GLint stat;
    glGetShaderiv(ShaderObj,GL_COMPILE_STATUS,&stat);
    if(!stat){
        fprintf(stderr , "Error in compiling the shaders");
        exit(1);
    }
    glAttachShader(ShaderProgram , ShaderObj);
}


static void CompileShaders() {

    ShaderProgram = glCreateProgram();
    if (ShaderProgram == 0)
        exit(0);

    string vs, fs;

    if (!ReadFile("shader.vs", vs))
        exit(1);
    if (!ReadFile("shader.fs", fs))
        exit(1);

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint stat;
    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &stat);
    if (!stat) {
        fprintf(stderr, "Error in linking shader program\n");
        exit(1);
    }
    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &stat);
    if (!stat) {
        fprintf(stderr, "Error in linking shader program\n");
        exit(1);
    }

    glUseProgram(ShaderProgram);
    gModelLocation = glGetUniformLocation(ShaderProgram, "gModel");
    gViewLocation = glGetUniformLocation(ShaderProgram, "gView");
    gProjectionLocation = glGetUniformLocation(ShaderProgram, "gProjection");
}
void createViewMatrix(float *m, float eyeX, float eyeY, float eyeZ,
        float centerX, float centerY, float centerZ, float upX,
        float upY, float upZ) {
    float fx = centerX - eyeX, fy = centerY - eyeY, fz = centerZ - eyeZ;
    float flen = sqrt(fx * fx + fy * fy + fz * fz);
    fx /= flen;
    fy /= flen;
    fz /= flen;
    if (fabs(fy) > 0.999f) {
        upX = 0.0f;
        upY = 0.0f;
        upZ = -1.0f;
    }

    float rx = fy * upZ - fz * upY;
    float ry = fz * upX - fx * upZ;
    float rz = fx * upY - fy * upX;
    float rlen = sqrt(rx * rx + ry * ry + rz * rz);
    rx /= rlen;
    ry /= rlen;
    rz /= rlen;

    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;

    memset(m, 0, 16 * sizeof(float));
    m[0] = rx;
    m[1] = ux;
    m[2] = -fx;
    m[3] = 0;
    m[4] = ry;
    m[5] = uy;
    m[6] = -fy;
    m[7] = 0;
    m[8] = rz;
    m[9] = uz;
    m[10] = -fz;
    m[11] = 0;
    m[12] = -(rx * eyeX + ry * eyeY + rz * eyeZ);
    m[13] = -(ux * eyeX + uy * eyeY + uz * eyeZ);
    m[14] = (fx * eyeX + fy * eyeY + fz * eyeZ);
    m[15] = 1;
}

void createModelMatrix(float *m, float tx, float ty, float tz,
        float angleDegrees, float sx, float sy, float sz) {
    memset(m, 0, 16 * sizeof(float));
    float rad = angleDegrees * M_PI / 180.0f;
    float c = cos(rad), s = sin(rad);

    // column 0  (X basis, scaled by sx)
    m[0] = c * sx;
    m[1] = 0;
    m[2] = -s * sx;
    m[3] = 0;

    // column 1  (Y basis, scaled by sy)
    m[4] = 0;
    m[5] = 1 * sy;
    m[6] = 0;
    m[7] = 0;

    // column 2  (Z basis, scaled by sz)
    m[8]  = s * sz;
    m[9]  = 0;
    m[10] = c * sz;
    m[11] = 0;

    // column 3  (translation – unchanged)
    m[12] = tx;
    m[13] = ty;
    m[14] = tz;
    m[15] = 1;
}

void setIdentity(float *m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void createProjectionMatrix(float *m, float l, float r, float b, float t,
        float n, float f) {
    memset(m, 0, 16 * sizeof(float));

    m[0] = (2.0f * n) / (r - l);

    m[5] = (2.0f * n) / (t - b);

    m[8] = (r + l) / (r - l);
    m[9] = (t + b) / (t - b);
    m[10] = -(f + n) / (f - n);
    m[11] = -1.0f;

    m[14] = -(2.0f * f * n) / (f - n);
}

void initNPCCars()
{
    const float LEFT_X_MIN  =  0.5f;   const float LEFT_X_MAX  =  6.5f;
    const float RIGHT_X_MIN = 45.5f;   const float RIGHT_X_MAX = 51.5f;
    const float Z_MIN       =  5.0f;   const float Z_MAX       = 85.0f;

    for (int i = 0; i < NUM_NPC; i++) {

        // randomly pick left or right strip
        int strip = rand() % 2;

        float xMin = (strip == 0) ? LEFT_X_MIN  : RIGHT_X_MIN;
        float xMax = (strip == 0) ? LEFT_X_MAX  : RIGHT_X_MAX;

        // rand() gives int 0..RAND_MAX, divide to get 0..1 float
        float t = (float)(rand() % 1000) / 1000.0f;   // 0.0 → 0.999
        npcCars[i].x = xMin + t * (xMax - xMin);

        t = (float)(rand() % 1000) / 1000.0f;
        npcCars[i].z = Z_MIN + t * (Z_MAX - Z_MIN);

        npcCars[i].angle = -90.0f;
        npcCars[i].r     = NPC_COLORS[i%10][0];
        npcCars[i].g     = NPC_COLORS[i%10][1];
        npcCars[i].b     = NPC_COLORS[i%10][2];
    }
}

void display(){

    float currentTime = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    float dt = currentTime = prevtime;
    prevtime = currentTime;

    float rad = Angle * M_PI / 180.0f;
    if(dt < 0.05)
        dt = 0.05f;
    CAR_X += Velocity *cos(rad)*dt;
    CAR_Z -= Velocity * sin(rad)*dt;
    camX += Velocity *cos(rad)*dt;
    camZ -= Velocity *sin(rad) *dt;
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    glUseProgram(ShaderProgram);

    GLint colorLoc = glGetUniformLocation(ShaderProgram, "objectColor");
    float matrix[16];
    float proj[16];
    float n = 1.0f, f = 100.0f;
    float r = 0.45f, l = -0.45f;
    float t = 0.45f, b = -0.45f;
    createProjectionMatrix(proj, l, r, b, t, n, f);
    glUniformMatrix4fv(gProjectionLocation, 1, GL_FALSE, proj);
    // this to create the view matrix for the camera
    float view[16];
    float yawRad = Angle * M_PI / 180.0f;
    float pitchRad = camPitch * M_PI / 180.0f;
    float dirX = -cos(pitchRad) * sin(yawRad);
    float dirY = sin(pitchRad);
    float dirZ = -cos(pitchRad) * (-cos(yawRad));
    createViewMatrix(view, camX, camY, camZ,                // eye
            camX + dirX, camY + dirY, camZ + dirZ, // look-at target
            0.0f, 1.0f, 0.0f);                     // up
    glUniformMatrix4fv(gViewLocation, 1, GL_FALSE, view);
    float identity[16];
    setIdentity(identity);
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, identity);

    //for the car 
    createModelMatrix(matrix,CAR_X,0.0,CAR_Z,Angle,1.0f,1.0f,1.0f);
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);   // blue car

    //for the body of the car
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
    glBindVertexArray(carBodyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 200);

    //for the windows of the car
    glUniform3f(colorLoc, 0.8588f, 0.8823f, 0.8901f);
    glBindVertexArray(carWinVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //for the track
    createModelMatrix(matrix,0.0f,0.0f,0.0f,0.0f,3.0f,3.0f,3.0f);
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
    glUniform3f(colorLoc, 0.1960f,0.1960f,0.1960f);
    glBindVertexArray(trackVerticalVAO);
    glDrawArrays(GL_TRIANGLES, 0, 20);
    glBindVertexArray(trackTopArcVAO);
    glDrawArrays(GL_TRIANGLES, 0, ARC_SEGS * 6);
    glBindVertexArray(trackBottomArcVAO);
    glDrawArrays(GL_TRIANGLES, 0, ARC_SEGS * 6);


    //npc npcCars
    for (int i = 0; i < NUM_NPC; i++) {

        createModelMatrix(matrix,npcCars[i].x,0.0f,npcCars[i].z,npcCars[i].angle,1.0f, 1.0f, 1.0f);   // scale = 1
        glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);

        glUniform3f(colorLoc, npcCars[i].r, npcCars[i].g, npcCars[i].b);
        glBindVertexArray(carBodyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 138);

        glUniform3f(colorLoc, 0.8588f, 0.8823f, 0.8901f);
        glBindVertexArray(carWinVAO);
        glDrawArrays(GL_TRIANGLES, 0, 48);
    }
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        fprintf(stderr, "GL error: %u\n", err);

    glutSwapBuffers();
    glutPostRedisplay();
}

static void reshape(int w,int h){
    glViewport(0,0,w,h);

}

void mouse(int button , int state , int x , int y){


}

void keyboard(unsigned char key,int x,int y){

    if(key == 'w' || key == 'W'){
        Velocity += Velocity_step;
        if(Velocity > MAX_VELOCITY)
            Velocity = MAX_VELOCITY;
    }    
    if(key == 's' || key == 'S'){
        Velocity -= Velocity_step;
    }
    if(key == 'd' || key == 'D'){
        Angle -= Angle_STEP;
    }
    if(key == 'a' || key == 'A'){
        Angle += Angle_STEP;
    }

}

void InitGlut(int argc,char ** argv){

    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,800);
    glutCreateWindow("asmt1");

    //all callback functions registering
    glutDisplayFunc(display); // this handles the rendering
    glutReshapeFunc(reshape); // whenever the window size changes this callback function is called
    glutIdleFunc(display); // this callback function is called even if there is no window system events are happening
    glutKeyboardFunc(keyboard); // this callback function is called when there is keyboard interrup
    glutMouseFunc(mouse);  // when we get an interrupt from the mouse this callback is called

}

void initAllBuffers()
{

    float body[] = {

        0.2f,0.4f,0.6f,  0.6f,0.5f,0.6f,  0.6f,0.5f,0.2f,
        0.2f,0.4f,0.6f,  0.6f,0.5f,0.2f,  0.2f,0.4f,0.2f,
        0.2f,0.2f,0.6f,  0.6f,0.2f,0.6f,  0.6f,0.2f,0.2f,
        0.2f,0.2f,0.6f,  0.6f,0.2f,0.2f,  0.2f,0.2f,0.2f,
        0.2f,0.2f,0.6f,  0.2f,0.4f,0.6f,  0.2f,0.4f,0.2f,
        0.2f,0.2f,0.6f,  0.2f,0.4f,0.2f,  0.2f,0.2f,0.2f,
        0.6f,0.2f,0.6f,  0.6f,0.5f,0.6f,  0.6f,0.5f,0.2f,
        0.6f,0.2f,0.6f,  0.6f,0.5f,0.2f,  0.6f,0.2f,0.2f,
        0.2f,0.2f,0.6f,  0.6f,0.2f,0.6f,  0.6f,0.5f,0.6f,
        0.2f,0.2f,0.6f,  0.6f,0.5f,0.6f,  0.2f,0.4f,0.6f,
        0.2f,0.2f,0.2f,  0.6f,0.2f,0.2f,  0.6f,0.5f,0.2f,
        0.2f,0.2f,0.2f,  0.6f,0.5f,0.2f,  0.2f,0.4f,0.2f,
        0.6f,0.5f,0.6f,  0.6f,0.2f,0.6f,  1.8f,0.2f,0.6f,
        0.6f,0.5f,0.6f,  1.8f,0.2f,0.6f,  1.8f,0.5f,0.6f,
        0.6f,0.2f,0.6f,  0.6f,0.2f,0.2f,  1.8f,0.2f,0.2f,
        0.6f,0.2f,0.6f,  1.8f,0.2f,0.2f,  1.8f,0.2f,0.6f,
        0.6f,0.5f,0.2f,  0.6f,0.2f,0.2f,  1.8f,0.2f,0.2f,
        0.6f,0.5f,0.2f,  1.8f,0.2f,0.2f,  1.8f,0.5f,0.2f,
        1.8f,0.5f,0.6f,  1.8f,0.5f,0.2f,  2.1f,0.4f,0.2f,
        1.8f,0.5f,0.6f,  2.1f,0.4f,0.2f,  2.1f,0.4f,0.6f,
        2.1f,0.2f,0.6f,  2.1f,0.2f,0.2f,  1.8f,0.2f,0.2f,
        2.1f,0.2f,0.6f,  1.8f,0.2f,0.2f,  1.8f,0.2f,0.6f,
        2.1f,0.4f,0.6f,  2.1f,0.4f,0.2f,  2.1f,0.2f,0.2f,
        2.1f,0.4f,0.6f,  2.1f,0.2f,0.2f,  2.1f,0.2f,0.6f,
        1.8f,0.2f,0.6f,  1.8f,0.5f,0.6f,  2.1f,0.4f,0.6f,
        1.8f,0.2f,0.6f,  2.1f,0.4f,0.6f,  2.1f,0.2f,0.6f,
        1.8f,0.2f,0.2f,  1.8f,0.5f,0.2f,  2.1f,0.4f,0.2f,
        1.8f,0.2f,0.2f,  2.1f,0.4f,0.2f,  2.1f,0.2f,0.2f,
        0.7f,0.65f,0.6f,  0.7f,0.65f,0.2f,  1.7f,0.65f,0.2f,
        0.7f,0.65f,0.6f,  1.7,0.65f,0.2f,  1.7f,0.65f,0.6f,
        0.7f,0.65f,0.2f,  0.7f,0.5f,0.2f,  0.75f,0.5f,0.2f,
        0.7f,0.65f,0.2f,  0.75f,0.5f,0.2f, 0.77f,0.65f,0.2f,
        1.2f,0.65f,0.2f,  1.2f,0.5f,0.2f,  1.25f,0.5f,0.2f,
        1.2f,0.65f,0.2f,  1.25f,0.5f,0.2f, 1.27f,0.65f,0.2f,
        1.65f,0.65f,0.2f, 1.65f,0.5f,0.2f, 1.7f,0.5f,0.2f,
        1.65f,0.65f,0.2f, 1.7f,0.5f,0.2f,  1.7f,0.65f,0.2f,
        0.75f,0.65f,0.2f, 0.75f,0.63f,0.2f, 1.7f,0.63f,0.2f,
        0.75f,0.65f,0.2f, 1.7f,0.63f,0.2f,  1.7f,0.65f,0.2f,
        0.7f,0.65f,0.6f,  0.7f,0.5f,0.6f,  0.75f,0.5f,0.6f,
        0.7f,0.65f,0.6f,  0.75f,0.5f,0.6f, 0.77f,0.65f,0.6f,
        1.2f,0.65f,0.6f,  1.2f,0.5f,0.6f,  1.25f,0.5f,0.6f,
        1.2f,0.65f,0.6f,  1.25f,0.5f,0.6f, 1.27f,0.65f,0.6f,
        1.65f,0.65f,0.6f, 1.65f,0.5f,0.6f, 1.7f,0.5f,0.6f,
        1.65f,0.65f,0.6f, 1.7f,0.5f,0.6f,  1.7f,0.65f,0.6f,
        0.75f,0.65f,0.6f, 0.75f,0.63f,0.6f, 1.7f,0.63f,0.6f,
        0.75f,0.65f,0.6f, 1.7f,0.63f,0.6f,  1.7f,0.65f,0.6f,
    };

    glGenVertexArrays(1, &carBodyVAO);
    glGenBuffers(1, &carBodyVBO);
    glBindVertexArray(carBodyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carBodyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(body), body, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float windows[] = {

        // front window
        0.77f,0.63f,0.2f, 0.75f,0.5f,0.2f, 1.2f,0.5f,0.2f,
        0.77f,0.63f,0.2f, 1.2f,0.5f,0.2f,  1.22f,0.63f,0.2f,

        // rear window
        1.27f,0.63f,0.2f, 1.25f,0.5f,0.2f, 1.65f,0.5f,0.2f,
        1.27f,0.63f,0.2f, 1.65f,0.5f,0.2f, 1.67f,0.63f,0.2f,

        // front window
        0.77f,0.63f,0.6f, 0.75f,0.5f,0.6f, 1.2f,0.5f,0.6f,
        0.77f,0.63f,0.6f, 1.2f,0.5f,0.6f,  1.22f,0.63f,0.6f,

        // rear window
        1.27f,0.63f,0.6f, 1.25f,0.5f,0.6f, 1.65f,0.5f,0.6f,
        1.27f,0.63f,0.6f, 1.65f,0.5f,0.6f, 1.67f,0.63f,0.6f,

        // ── front windshield (angled quad) ──────────────────────
        0.6f,0.5f,0.6f,  0.6f,0.5f,0.2f,  0.7f,0.65f,0.2f,
        0.6f,0.5f,0.6f,  0.7f,0.65f,0.2f, 0.7f,0.65f,0.6f,

        // ── rear windshield (angled quad) ───────────────────────
        1.7f,0.65f,0.6f, 1.7f,0.65f,0.2f, 1.8f,0.5f,0.2f,
        1.7f,0.65f,0.6f, 1.8f,0.5f,0.2f,  1.8f,0.5f,0.6f,

        // front left corner (z=0.6 side)
        0.6f,0.5f,0.6f,  0.7f,0.65f,0.6f, 0.7f,0.5f,0.6f,
        // front right corner (z=0.2 side)
        0.6f,0.5f,0.2f,  0.7f,0.65f,0.2f, 0.7f,0.5f,0.2f,
        // rear left corner (z=0.2 side)
        1.7f,0.65f,0.2f, 1.8f,0.5f,0.2f,  1.7f,0.5f,0.2f,
        // rear right corner (z=0.6 side)
        1.7f,0.65f,0.6f, 1.8f,0.5f,0.6f,  1.7f,0.5f,0.6f,
    };

    glGenVertexArrays(1, &carWinVAO);
    glGenBuffers(1, &carWinVBO);
    glBindVertexArray(carWinVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carWinVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(windows), windows, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float trackVertical[] ={
        //left side
        0.0f,0.0f,0.0f,     3.0f,0.0f,0.0f,       0.0f,0.0f,30.0f,
        3.0f,0.0f,0.0f,     3.0f,0.0f,30.f,       0.0f,0.0f,30.0f,

        //right side
        15.0f,0.0f,0.0f,     18.0f,0.0f,0.0f,       15.0f,0.0f,30.0f,
        18.0f,0.0f,0.0f,     18.0f,0.0f,30.f,       15.0f,0.0f,30.0f,

    };
    glGenVertexArrays(1, &trackVerticalVAO);
    glGenBuffers(1, &trackVerticalVBO);
    glBindVertexArray(trackVerticalVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trackVerticalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(trackVertical), trackVertical, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float top[ARC_SEGS * 6 * 3];   // 6 verts per slice × 3 floats
    int vi = 0;
    for (int i = 0; i < ARC_SEGS; i++) {
        float a0 = (float) i      / ARC_SEGS * M_PI;
        float a1 = (float)(i + 1) / ARC_SEGS * M_PI;

        float ox0 = ARC_CX + ARC_OUTER * cosf(a0),  oz0 = -ARC_OUTER * sinf(a0);
        float ox1 = ARC_CX + ARC_OUTER * cosf(a1),  oz1 = -ARC_OUTER * sinf(a1);
        float ix0 = ARC_CX + ARC_INNER * cosf(a0),  iz0 = -ARC_INNER * sinf(a0);
        float ix1 = ARC_CX + ARC_INNER * cosf(a1),  iz1 = -ARC_INNER * sinf(a1);

        top[vi++]=ox0; top[vi++]=0; top[vi++]=oz0;
        top[vi++]=ix0; top[vi++]=0; top[vi++]=iz0;
        top[vi++]=ix1; top[vi++]=0; top[vi++]=iz1;
        top[vi++]=ox0; top[vi++]=0; top[vi++]=oz0;
        top[vi++]=ix1; top[vi++]=0; top[vi++]=iz1;
        top[vi++]=ox1; top[vi++]=0; top[vi++]=oz1;
        glBindVertexArray(0);
    }
    glGenVertexArrays(1, &trackTopArcVAO);
    glGenBuffers(1, &trackTopArcVBO);
    glBindVertexArray(trackTopArcVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trackTopArcVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(top), top, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float verts[ARC_SEGS * 6 * 3];
    vi = 0;
    for (int i = 0; i < ARC_SEGS; i++) {
        float a0 = (float) i      / ARC_SEGS * M_PI;
        float a1 = (float)(i + 1) / ARC_SEGS * M_PI;

        float ox0 = ARC_CX + ARC_OUTER * cosf(a0),  oz0 = 30.0f + ARC_OUTER * sinf(a0);
        float ox1 = ARC_CX + ARC_OUTER * cosf(a1),  oz1 = 30.0f + ARC_OUTER * sinf(a1);
        float ix0 = ARC_CX + ARC_INNER * cosf(a0),  iz0 = 30.0f + ARC_INNER * sinf(a0);
        float ix1 = ARC_CX + ARC_INNER * cosf(a1),  iz1 = 30.0f + ARC_INNER * sinf(a1);

        verts[vi++]=ox0; verts[vi++]=0; verts[vi++]=oz0;
        verts[vi++]=ix0; verts[vi++]=0; verts[vi++]=iz0;
        verts[vi++]=ix1; verts[vi++]=0; verts[vi++]=iz1;
        verts[vi++]=ox0; verts[vi++]=0; verts[vi++]=oz0;
        verts[vi++]=ix1; verts[vi++]=0; verts[vi++]=iz1;
        verts[vi++]=ox1; verts[vi++]=0; verts[vi++]=oz1;
    }

    glGenVertexArrays(1, &trackBottomArcVAO);
    glGenBuffers(1, &trackBottomArcVBO);
    glBindVertexArray(trackBottomArcVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trackBottomArcVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}


int main(int argc , char **argv){

    srand(time(0));
    InitGlut(argc,argv);
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    initNPCCars();
    glEnable(GL_DEPTH_TEST);
    CompileShaders();
    initAllBuffers();
    glutMainLoop();


}
