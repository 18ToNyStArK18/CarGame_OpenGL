#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <string.h>
#include <SFML/Audio.hpp>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "file_utils.h"
GLuint ShaderProgram;

bool bullettime = false;
float prevtime = 0.0f;
//windmill
float windmillAngle = 0.0f;       
float windmillSpeed = 45.0f;      
GLuint windmillVAO, windmillVBO;
int windmillVertCount = 0;

//wall
const float SCENE_X_MIN = -15.0f;
const float SCENE_X_MAX = 65.0f;
const float SCENE_Z_MIN = -30.0f;
const float SCENE_Z_MAX = 120.0f;

GLuint wallVAO, wallVBO;
int wallVertCount = 0;

//car variables
float CAR_X=50.0f,CAR_Y=0,CAR_Z=0;
float Velocity;
float Angle=0.0f;
float Angle_STEP = 2.0f;
float Velocity_step = 0.1f;
const float MAX_VELOCITY = 20.f;
struct CAR {
    GLuint carVAO,carVBO;
    GLuint caruvVBO;
    GLuint texture;
    int vertices_count;
};
CAR carModel;
CAR npcModel;
#define NUM_NPC  10
struct NPCCar {
    float x, z;       
    float angle;      
    float r, g, b;    
};
NPCCar npcCars[NUM_NPC];
static const float NPC_COLORS[10][3] = {
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
};
bool boost = false;
float boost_time = 0;

const float BUILD_HALF_W  = 2.5f;   
const float BUILD_HALF_L  = 2.5f;

//track variables

GLuint trackVerticalVAO,trackVerticalVBO;
GLuint trackTopArcVBO , trackTopArcVAO , trackBottomArcVAO , trackBottomArcVBO;
const int   ARC_SEGS  = 30;        
const float ARC_CX    = 9.0f;      
const float ARC_OUTER = 9.0f;
const float ARC_INNER = 6.0f;

//for the hitboxes
const float PLAYER_HALF_W = 0.5f;   // player car X
const float PLAYER_HALF_L = 1.0f;   // player car Z
const float NPC_HALF_W    = 0.4f;   // npc car X
const float NPC_HALF_L    = 0.9f;   // npc car Z
bool showHitboxes = false;
GLuint hitboxVAO, hitboxVBO;


//for the cameras
int cameraMode = 5;
float groundViewYaw = 0.0f;


//for the building

struct build {
    GLuint VAO , VBO;
    GLuint uvVBO;
    float x,z;
    float sx,sy,sz;
    vector<float> vertices;
    vector<pair<float,float>> uvs;
    int vertices_count;
    GLuint textureID;
};
const int NUM_OF_BUILDS =  5;

build buildings[NUM_OF_BUILDS];
vector <float> buildheights = {7.59411,8.98195,18.5,7.3438,9.3355};
struct BuildingHitbox {
    float offsetX; 
    float offsetZ; 
    float halfW;   
    float halfL;   
};

const BuildingHitbox buildHitboxes[NUM_OF_BUILDS] = {
    {-0.53f, -0.14f, 1.74f, 1.55f}, 
    { 0.18f,  0.11f, 3.32f, 2.79f}, 
    { 0.01f,  0.07f, 2.10f, 1.51f}, 
    { 0.04f,  0.00f, 0.98f, 0.96f}, 
    {-0.27f, -0.02f, 1.25f, 1.51f}  
};

//for the lights
struct SwingLight {
    float angle;        
    float speed;        
    float r, g, b;      
    float buildX, buildY ,buildZ; 
};
GLuint lightVAO,lightVBO,lightEBO;
int lightindices;

int streetvertices;
#define NUM_SWING_LIGHTS 5

SwingLight swingLights[NUM_SWING_LIGHTS];

GLuint gNumLightsLoc;
GLuint gLightPosLoc[10];
GLuint gLightColorLoc[10];
// streetlights
#define NUM_STREET_LIGHTS 5
GLuint streetlightVAO , streetlightVBO , streelightuvVBO;
struct street{
    float x,y,z;
    float angle;
};
street StreetLights[NUM_STREET_LIGHTS];

static const float LIGHT_COLORS[5][3] = {
    {1.0f, 0.3f, 0.3f},  
    {0.3f, 1.0f, 0.3f}, 
    {0.3f, 0.3f, 1.0f},  
    {1.0f, 1.0f, 0.3f},  
    {0.8f, 0.3f, 1.0f},  
};

bool collided = false;

GLuint gModelLocation, gViewLocation, gProjectionLocation,gTextureLocation,gUseTexture,gLightPosLocation,gViewPosLocation,gUseShine,gisLight;

int buildingVertexCount1 = 0;

struct Vertex3D { float x, y, z; };

void playCollsionsound() {
    static sf::Music music;
    if (music.getStatus() != sf::Music::Playing) {
        music.openFromFile("collsion.mp3");
        music.play();
    }
}
void playHornsound() {
    static sf::Music music;
    if (music.getStatus() != sf::Music::Playing) {
        music.openFromFile("horn.mp3");
        music.play();
    }
}

void initSwingLights() {
    for (int i = 0; i < NUM_SWING_LIGHTS; i++) {
        swingLights[i].angle    = 0.0f;
        swingLights[i].speed    = 30.0f;  
        swingLights[i].r        = LIGHT_COLORS[i][0];
        swingLights[i].g        = LIGHT_COLORS[i][1];
        swingLights[i].b        = LIGHT_COLORS[i][2];
        swingLights[i].buildX   = ((i+1)%2)*(50) + (i%2)*(-15);
        swingLights[i].buildZ   = i * 20.0f;
        swingLights[i].buildY = buildheights[i];
    }
}
void drawLightMarker(float lx, float ly, float lz, float r, float g, float b) {
    GLint colorLoc = glGetUniformLocation(ShaderProgram, "objectColor");
    glUniform1i(gUseTexture, 0);
    glUniform1i(gUseShine,   0);
    glUniform3f(colorLoc, r, g, b);

    float m[16];
    float size = 0.3f;   
    memset(m, 0, sizeof(m));
    m[0]  = size;
    m[5]  = size;
    m[10] = size;
    m[12] = lx;
    m[13] = ly;
    m[14] = lz;
    m[15] = 1.0f;

    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, m);
    glBindVertexArray(lightVAO);
    glDrawElements(GL_TRIANGLES,lightindices,GL_UNSIGNED_INT,0);
}
void initNPCCars()
{
    const float LEFT_X_MIN  =  0.5f;   const float LEFT_X_MAX  =  6.5f;
    const float RIGHT_X_MIN = 45.5f;   const float RIGHT_X_MAX = 51.5f;
    const float Z_MIN       =  5.0f;   const float Z_MAX       = 85.0f;

    for (int i = 0; i < NUM_NPC; i++) {

        int strip = rand() % 2;

        float xMin = (strip == 0) ? LEFT_X_MIN  : RIGHT_X_MIN;
        float xMax = (strip == 0) ? LEFT_X_MAX  : RIGHT_X_MAX;

        float t = (float)(rand() % 1000) / 1000.0f;
        npcCars[i].x = xMin + t * (xMax - xMin);

        t = (float)(rand() % 1000) / 1000.0f;
        npcCars[i].z = Z_MIN + t * (Z_MAX - Z_MIN);

        npcCars[i].angle = 180.0f-180.0*strip;
        npcCars[i].r     = NPC_COLORS[i%10][0];
        npcCars[i].g     = NPC_COLORS[i%10][1];
        npcCars[i].b     = NPC_COLORS[i%10][2];
    }
}
bool checkCollision(float ax, float az, float bx, float bz) {
    return (fabs(ax - bx) < (PLAYER_HALF_W + NPC_HALF_W)) &&
        (fabs(az - bz) < (PLAYER_HALF_L + NPC_HALF_L));
}

void checkCollisions() {

    if (CAR_X - PLAYER_HALF_W < SCENE_X_MIN || 
            CAR_X + PLAYER_HALF_W > SCENE_X_MAX ||
            CAR_Z - PLAYER_HALF_L < SCENE_Z_MIN || 
            CAR_Z + PLAYER_HALF_L > SCENE_Z_MAX) {

        Velocity = 0.0f;
        boost = false;
        if(!collided) {
            playCollsionsound();
        }
        collided = true;
        return; 
    }
    for (int i = 0; i < NUM_OF_BUILDS; i++) {
        float baseBuildX = 40.0f - (i % 2) * 25.0f;
        float baseBuildZ = i * 20.0f;

        float trueCenterX = baseBuildX + buildHitboxes[i].offsetX;
        float trueCenterZ = baseBuildZ + buildHitboxes[i].offsetZ;

        bool hitBuilding = (fabs(CAR_X - trueCenterX) < (PLAYER_HALF_W + buildHitboxes[i].halfW)) &&
            (fabs(CAR_Z - trueCenterZ) < (PLAYER_HALF_L + buildHitboxes[i].halfL));

        if (hitBuilding) {
            Velocity = 0.0f;
            boost = false;
            if(!collided) playCollsionsound();
            collided = true;
            return;
        }
    }
    for (int i = 0; i < NUM_NPC; i++) {
        if (checkCollision(CAR_X, CAR_Z, npcCars[i].x, npcCars[i].z)) {
            Velocity = 0.0f;
            boost = false;

            if(!collided)
                playCollsionsound();
            collided = true;
            return; 
        }
    }
}
//this function is to create a vertex array from the obj file exported from the blender
vector<float> loadObjToFlatArray(string filepath) {
    vector<Vertex3D> temp_vertices;
    vector<Vertex3D> temp_normals;
    vector<float> out_vertices;
    vector<pair<float,float>> temp_uv;

    ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return out_vertices;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            Vertex3D v;
            ss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        }
        else if (prefix == "vn") {
            Vertex3D n;
            ss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        }
        else if (prefix == "vt") {
            float u, v; ss >> u >> v;
            temp_uv.push_back({u, v});
        }
        else if (prefix == "f") {
            std::string v1, v2, v3, v4;
            ss >> v1 >> v2 >> v3;

            if (!(ss >> v4)) { v4 = ""; }

            auto getIndices = [](const std::string& token) -> pair<int,int> {
                // the format is the vi/vti/vni
                // vertexindex/texturecord/normal
                size_t s1 = token.find('/');
                int vi = std::stoi(token.substr(0, s1)) - 1;
                int ni = -1;
                if (s1 != string::npos) {
                    size_t s2 = token.find('/', s1 + 1);
                    if (s2 != string::npos && s2 + 1 < token.size())
                        ni = std::stoi(token.substr(s2 + 1)) - 1;
                }
                return {vi, ni};
            };
            auto pushVert = [&](const std::string& token) {
                auto [vi, ni] = getIndices(token);
                out_vertices.push_back(temp_vertices[vi].x);
                out_vertices.push_back(temp_vertices[vi].y);
                out_vertices.push_back(temp_vertices[vi].z);
                if (ni >= 0 && ni < (int)temp_normals.size()) {
                    out_vertices.push_back(temp_normals[ni].x);
                    out_vertices.push_back(temp_normals[ni].y);
                    out_vertices.push_back(temp_normals[ni].z);
                } else {
                    out_vertices.push_back(0.0f);   
                    out_vertices.push_back(1.0f);
                    out_vertices.push_back(0.0f);
                }
            };
            pushVert(v1); pushVert(v2); pushVert(v3);
            if (!v4.empty()) {
                pushVert(v1); pushVert(v3); pushVert(v4);
            }

        }
    }
    return out_vertices;
}
vector<pair<float,float>> loadUVToFlatArray(string filepath) {


    //file has the uv verices after the prefix vt
    //in the line which has the f as the prefix it contains something like this a/b where a indicated the vertex a uses the uv at bth index;
    vector<pair<float,float>> temp_uv;

    vector<pair<float,float>> out_uv;

    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Failed to open " << filepath << endl;
        return out_uv;
    }

    auto getUVIndex = [](const string& token) -> int {
        size_t s1 = token.find('/');
        if (s1 == string::npos) return -1;        
        size_t s2 = token.find('/', s1 + 1);
        string uvStr = token.substr(s1 + 1, s2 - s1 - 1);
        if (uvStr.empty()) return -1;             
        return stoi(uvStr) - 1;                      
    };

    auto pushUV = [&](const string& token) {
        int ti = getUVIndex(token);
        if (ti >= 0 && ti < (int)temp_uv.size())
            out_uv.push_back(temp_uv[ti]);
        else
            out_uv.push_back({0.0f, 0.0f});
    };

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string prefix;
        ss >> prefix;

        if (prefix == "vt") {
            float u, v;
            ss >> u >> v;
            temp_uv.push_back({u, v});
        }
        else if (prefix == "f") {
            string t1, t2, t3, t4;
            ss >> t1 >> t2 >> t3;
            pushUV(t1); pushUV(t2); pushUV(t3);  
            if (ss >> t4) {                      
                pushUV(t1); pushUV(t3); pushUV(t4);
            }
        }
    }
    return out_uv;
}

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
    gTextureLocation = glGetUniformLocation(ShaderProgram,"gTexture");
    gUseTexture         = glGetUniformLocation(ShaderProgram, "useTexture");
    gViewPosLocation  = glGetUniformLocation(ShaderProgram, "viewPos");
    gUseShine         = glGetUniformLocation(ShaderProgram,"useShine");
    gNumLightsLoc = glGetUniformLocation(ShaderProgram, "numLights");
    gisLight = glGetUniformLocation(ShaderProgram,"isLight");
    char buf[64];
    for (int i = 0; i < 10; i++) {
        sprintf(buf, "lightPos[%d]",   i); gLightPosLoc[i]   = glGetUniformLocation(ShaderProgram, buf);
        sprintf(buf, "lightColor[%d]", i); gLightColorLoc[i] = glGetUniformLocation(ShaderProgram, buf);
    }
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
        float angleDegrees, float angleDegrees2, float sx, float sy, float sz) {
    memset(m, 0, 16 * sizeof(float));

    float rad = angleDegrees * M_PI / 180.0f;    // y-axis rotation
    float c = cos(rad), s = sin(rad);

    float rad2 = angleDegrees2 * M_PI / 180.0f;  // z-axis rotation
    float c2 = cos(rad2), s2 = sin(rad2);

    // Column 0: X basis vector
    m[0] = c2 * c * sx;
    m[1] = s2 * c * sx;
    m[2] = -s * sx;
    m[3] = 0;

    // Column 1: Y basis vector
    m[4] = -s2 * sy;
    m[5] = c2 * sy;
    m[6] = 0;
    m[7] = 0;

    // Column 2: Z basis vector
    m[8]  = c2 * s * sz;
    m[9]  = s2 * s * sz;
    m[10] = c * sz;
    m[11] = 0;

    // Column 3: Translation
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


GLuint loadTexture(string path) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);   
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture: %s\n", path.c_str());
        return 0;
    }
    GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return id;
}
void drawHitbox(float cx, float cy, float cz, float hw, float hl, float height,
        float r, float g, float b,int flag) {
    GLint colorLoc = glGetUniformLocation(ShaderProgram, "objectColor");
    glUniform3f(colorLoc, r, g, b);
    glUniform1i(gUseTexture, 0);
    glUniform1i(gUseShine,   0);

    float m[16];
    if(flag)    
        createModelMatrix(m,cx,cy,cz,Angle,0.0f,hw,height,hl); 
    else    
        createModelMatrix(m,cx,cy,cz,0.0f,0.0f,hw,height,hl); 
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, m);
    glBindVertexArray(hitboxVAO);
    glDrawArrays(GL_LINES, 0, 24);
}

void display(){


    float currentTime = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    for (int i = 0; i < NUM_SWING_LIGHTS; i++) {
        // the swing should happen between -30 to 30
        swingLights[i].angle = 30.0f * sinf(currentTime * (0.5f + i * 0.1f));
    }

    float dt = currentTime - prevtime;
    if(bullettime)
        dt = 0.01;
    windmillAngle += windmillSpeed * dt;
    if (windmillAngle > 360.0f) 
        windmillAngle -= 360.0f;
    prevtime = currentTime;
    if(currentTime - boost_time >= 1.0f)
        boost = false;

    float rad = (-90.0f+Angle) * M_PI / 180.0f;
    if(dt < 0.03)
        dt = 0.03f;
    if(bullettime)
        dt = 0.003;
    if(!boost){
        CAR_X += Velocity *cos(rad)*dt;
        CAR_Z -= Velocity * sin(rad)*dt;
    }
    else{
        CAR_X += min(5*Velocity,10.0f) *cos(rad)*dt;
        CAR_Z -= min(5*Velocity,10.0f) * sin(rad)*dt;

    }
    checkCollisions();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    glUseProgram(ShaderProgram);


    glUniform1i(gNumLightsLoc, NUM_SWING_LIGHTS);
    GLint colorLoc = glGetUniformLocation(ShaderProgram, "objectColor");
    float matrix[16];
    float proj[16];
    float n = 1.0f, f = 200.0f;
    float r = 0.45f, l = -0.45f;
    float t = 0.45f, b = -0.45f;
    if(boost){
        f += 5.5f;
        n -= 0.5f;
    }
    createProjectionMatrix(proj, l, r, b, t, n, f);
    glUniformMatrix4fv(gProjectionLocation, 1, GL_FALSE, proj);
    // this to create the view matrix for the camera
    float view[16];
    float eyeX, eyeY, eyeZ;
    float targetX, targetY, targetZ;
    float upX = 0.0f, upY = 1.0f, upZ = 0.0f;
    float fwdX = cos(rad);
    float fwdZ = -sin(rad);
    switch (cameraMode) {

        case 1:{  
                   //this is placed at the center at high pointing downwards
                   eyeX = 27.0f;  eyeY =199.0f;  eyeZ = 45.0f;
                   targetX = 27.0f; targetY = 0.0f; targetZ = 45.0f;
                   upX = 0.0f; upY = 0.0f; upZ = -1.0f;
                   break;
               }
        case 2:{ 
                   eyeX   = CAR_X - fwdX * 0.5f;   // slightly behind the nose
                   eyeY   = CAR_Y + 0.65f;          // near the room
                   eyeZ   = CAR_Z - fwdZ * 0.5f;
                   targetX = CAR_X + fwdX * 15.0f;
                   targetY = CAR_Y - 0.3f;          // tilt slightly down so that we can wee the bottom
                   targetZ = CAR_Z + fwdZ * 15.0f;
                   upX = 0.0f; upY = 1.0f; upZ = 0.0f;
                   break;
               }
        case 3: { 
                    //placed beside a builing and can change the fov using the keys
                    float gvRad = groundViewYaw * M_PI / 180.0f;
                    eyeX = 42.0f;  eyeY = 3.0f;  eyeZ = 3.0f;
                    targetX = eyeX + cosf(gvRad) * 20.0f;
                    targetY = 1.0f;
                    targetZ = eyeZ + sinf(gvRad) * 20.0f;
                    upX = 0.0f; upY = 1.0f; upZ = 0.0f;
                    break;
                }

        case 4: { 
                    //placed at the light and moves with the light
                    float sRad = swingLights[0].angle * M_PI / 180.0f;
                    float lx = swingLights[0].buildX;          
                    float ly = swingLights[0].buildY - cosf(sRad) * 2.0f;
                    float lz = swingLights[0].buildZ + sinf(sRad) * 8.0f;
                    eyeX = lx;  eyeY = ly;  eyeZ = lz;
                    targetX = lx;  targetY = 0.0f;  targetZ = swingLights[0].buildZ;
                    upX = 0.0f; upY = 0.0f; upZ = -1.0f; 
                    break;
                }

        default:
        case 5:
                {
                    //bird cam view the default one
                    float followDistance = 12.0f;
                    float cameraHeight   =  5.0f;
                    eyeX = CAR_X - fwdX * followDistance;
                    eyeY = CAR_Y + cameraHeight;
                    eyeZ = CAR_Z - fwdZ * followDistance;
                    targetX = CAR_X;
                    targetY = CAR_Y + 2.0f;
                    targetZ = CAR_Z;
                    upX = 0.0f; upY = 1.0f; upZ = 0.0f;
                }
                break;
    }

    createViewMatrix(view, eyeX, eyeY, eyeZ, targetX, targetY, targetZ, upX, upY, upZ);
    glUniformMatrix4fv(gViewLocation, 1, GL_FALSE, view);
    glUniform3f(gViewPosLocation,  eyeX,  eyeY, eyeZ);   // camera pos
    float identity[16];
    setIdentity(identity);
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, identity);
    for (int i = 0; i < NUM_SWING_LIGHTS; i++) {
        float rad = swingLights[i].angle * M_PI / 180.0f;
        float lx = swingLights[i].buildX;
        float ly = swingLights[i].buildY +2.0 - cos(rad)*2 ;
        float lz = swingLights[i].buildZ + sin(rad) * 8;

        glUniform3f(gLightPosLoc[i],   lx, ly, lz);
        glUniform3f(gLightColorLoc[i], swingLights[i].r, swingLights[i].g,swingLights[i].b);
        glUniform1i(gisLight,1);
        drawLightMarker(lx, ly, lz, swingLights[i].r, swingLights[i].g, swingLights[i].b);
        glUniform1i(gisLight,0);
    }
    if (showHitboxes) {
        drawHitbox(CAR_X, 0.0f, CAR_Z,
                PLAYER_HALF_W, PLAYER_HALF_L, 0.6f,
                0.0f, 1.0f, 0.0f,1);

        for (int i = 0; i < NUM_NPC; i++) {
            drawHitbox(npcCars[i].x, 0.0f, npcCars[i].z,
                    NPC_HALF_W, NPC_HALF_L, 0.6f,
                    1.0f, 0.0f, 0.0f,0);
        }
        for (int i = 0; i < NUM_OF_BUILDS; i++) {
            float baseBuildX = 40.0f - (i % 2) * 25.0f;
            float baseBuildZ = i * 20.0f;
            
            float trueCenterX = baseBuildX + buildHitboxes[i].offsetX;
            float trueCenterZ = baseBuildZ + buildHitboxes[i].offsetZ;

            drawHitbox(trueCenterX, 0.0f, trueCenterZ,
                    buildHitboxes[i].halfW, buildHitboxes[i].halfL, 2.0f,
                    0.0f, 0.5f, 1.0f, 0); 
        }
    }

    glUniform1i(gUseTexture, 1);
    glUniform1i(gTextureLocation, 0);

    //for the car 
    glUniform1i(gUseShine,1);
    createModelMatrix(matrix,CAR_X,0.40f,CAR_Z,Angle,0.0f,0.5f,0.5f,0.5f);
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, carModel.texture);
    glBindVertexArray(carModel.carVAO);
    glDrawArrays(GL_TRIANGLES, 0,carModel.vertices_count );
    glUniform1i(gUseShine,0);



    //for all the buildings
    for(int i=0;i<NUM_OF_BUILDS;i++){
        glUniform1i(gUseTexture,1); 
        createModelMatrix(matrix,40.0f - (i%2)*25.0f,0.0,0.0f+i*20.0f,0.0,0.0f,0.50f,.50f,.50f);
        glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buildings[i].textureID);
        glBindVertexArray(buildings[i].VAO);
        glDrawArrays(GL_TRIANGLES, 0, buildings[i].vertices_count);

        float bx = 40.0f - (i%2)*25.0f;
        float bz = i * 20.0f;
        float by = buildheights[i];

        glUniform1i(gUseTexture, 0);
        glUniform1i(gUseShine, 0);
        glUniform1i(gisLight,0);
        glUniform3f(colorLoc, 0.6f, 0.6f, 0.6f);  

        createModelMatrix(matrix, bx, by, bz,0.0f, windmillAngle,1.0f, 1.0f, 0.1f);
        glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
        glBindVertexArray(windmillVAO);
        glDrawArrays(GL_TRIANGLES, 0, windmillVertCount);


        glUniform1i(gUseTexture, 0);
        glUniform3f(colorLoc, 0.1960f,0.1960f,0.1960f);
        createModelMatrix(matrix,40.0f - (i%2)*40.0f + ((i+1)%2)*17,0.0,0.0f+i*20.0f,((i+1)%2)*180,0.0f,1.0f,1.00f,1.0f);
        glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
        glBindVertexArray(streetlightVAO);
        glDrawArrays(GL_TRIANGLES,0,streetvertices);
        glUniform3f(gLightPosLoc[i+5], 40.0f - (i%2)*37.25f + ((i+1)%2)*14.25  , 5.5f, i*20.0f);
        glUniform3f(gLightColorLoc[i+5],1.0f ,0.8784f ,0.5530f);
        glUniform1i(gisLight,1);
        drawLightMarker(40.0f - (i%2)*37.25f + ((i+1)%2)*14.25  , 5.5f, i*20.0f, 1.0f, 0.8784, 0.5530);
        glUniform1i(gisLight,0);
    }



    //npc cars
    glUniform1i(gUseShine,1);
    for (int i = 0; i < NUM_NPC; i++) {

        createModelMatrix(matrix,npcCars[i].x,0.0f,npcCars[i].z,npcCars[i].angle,0.0f,.50f, .50f, .50f);   // scale = 1
        glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
        glUniform3f(colorLoc,npcCars[i].r,npcCars[i].g,npcCars[i].b);
        glBindVertexArray(npcModel.carVAO);
        glDrawArrays(GL_TRIANGLES, 0,npcModel.vertices_count );


    }
    //for the track
    createModelMatrix(matrix,0.0f,0.0f,0.0f,0.0f,0.0f,3.0f,3.0f,3.0f);
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);
    glBindVertexArray(trackVerticalVAO);
    glUniform3f(colorLoc, 0.1960f,0.1960f,0.1960f);
    glDrawArrays(GL_TRIANGLES, 0, 20);
    glBindVertexArray(trackTopArcVAO);
    glDrawArrays(GL_TRIANGLES, 0, ARC_SEGS * 6);
    glBindVertexArray(trackBottomArcVAO);
    glDrawArrays(GL_TRIANGLES, 0, ARC_SEGS * 6);

    glUniform1i(gUseShine,0);


    //for the wall
    glUniform1i(gUseTexture, 0);
    glUniform1i(gUseShine, 0);
    glUniform3f(colorLoc, 0.667, 0.290, 0.267);

    setIdentity(matrix);
    glUniformMatrix4fv(gModelLocation, 1, GL_FALSE, matrix);

    glBindVertexArray(wallVAO);
    glDrawArrays(GL_TRIANGLES, 0, wallVertCount);
    // -----------------------------



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
    if(!collided){
        if(key == 'f' || key == 'F'){
            Velocity += Velocity_step;
            if(Velocity > MAX_VELOCITY)
                Velocity = MAX_VELOCITY;
        }    
        if(key == 's' || key == 'S'){
            Velocity -= Velocity_step;
        }
        if(key == 'R' || key == 'r'){
            Angle -= Angle_STEP;
        }
        if(key == 'l' || key == 'L'){
            Angle+= Angle_STEP;
        }
        if(key == 'b' || key == 'B')
            showHitboxes = !showHitboxes;
        if(key == 't' || key == 'T')
            bullettime = !bullettime;

        if(key == 'h' || key == 'H')
            playHornsound();

        if(key == ' ' && !boost){
            boost = true;
            boost_time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
        }
        if( key =='u'){
            CAR_Y += 0.3f;
        }
        if(key == 'U'){
            CAR_Y -= 0.3f;
        }
        if(key >= '1' && key <= '5')
            cameraMode = key - '0';
        if(cameraMode == 3){
            if(key == ',') groundViewYaw = fmaxf(groundViewYaw - 3.0f, -30.0f);
            if(key == '.') groundViewYaw = fminf(groundViewYaw + 3.0f,  30.0f);
        }
        if (key == 'W') {                          
            windmillSpeed += 10.0f;
        }
        if (key == 'w') {                          
            windmillSpeed -= 10.0f;
            if (windmillSpeed < 0.0f) windmillSpeed = 0.0f;
        }
    }
    if(collided && (key == 'r' || key == 'R')){
        CAR_X = 50.0f;
        CAR_Y = 0.0f;
        CAR_Z = 0.0f;
        initNPCCars();
        collided = false;
        Angle = 0.0f;
        cameraMode = 5;
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
void initWindmill() {
    const int SPOKES = 5;
    const float RIM_R = 1.0f;       // outer radius
    const float SPOKE_W = 0.06f;    // half-width of each spoke

    vector<float> verts;

    auto pushTri = [&](float x0,float y0, float x1,float y1, float x2,float y2) {
        verts.insert(verts.end(), {x0,y0,0,  0,0,1,
                x1,y1,0,  0,0,1,
                x2,y2,0,  0,0,1});
    };

    for (int s = 0; s < SPOKES; s++) {
        float a = s * (2.0f * M_PI / SPOKES);
        float cx = cosf(a), cy = sinf(a);
        float px = -sinf(a) * SPOKE_W, py = cosf(a) * SPOKE_W;
        pushTri( px,  py,  cx*RIM_R + px, cy*RIM_R + py,  cx*RIM_R - px, cy*RIM_R - py);
        pushTri( px,  py,  cx*RIM_R - px, cy*RIM_R - py, -px, -py);
    }


    windmillVertCount = verts.size() / 6;

    glGenVertexArrays(1, &windmillVAO);
    glGenBuffers(1, &windmillVBO);
    glBindVertexArray(windmillVAO);
    glBindBuffer(GL_ARRAY_BUFFER, windmillVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}
void initWalls() {
    vector<float> verts;
    float h = 6.0f;    
    float t = 1.0f;    

    auto addBox = [&](float x1, float x2, float y1, float y2, float z1, float z2) {
        float box[] = {
            x1,y1,z2, x2,y1,z2, x2,y2,z2,  x1,y1,z2, x2,y2,z2, x1,y2,z2, // front
            x2,y1,z1, x1,y1,z1, x1,y2,z1,  x2,y1,z1, x1,y2,z1, x2,y2,z1, // back
            x1,y1,z1, x1,y1,z2, x1,y2,z2,  x1,y1,z1, x1,y2,z2, x1,y2,z1, // left
            x2,y1,z2, x2,y1,z1, x2,y2,z1,  x2,y1,z2, x2,y2,z1, x2,y2,z2, // right
            x1,y2,z2, x2,y2,z2, x2,y2,z1,  x1,y2,z2, x2,y2,z1, x1,y2,z1, // top
            x1,y1,z1, x2,y1,z1, x2,y1,z2,  x1,y1,z1, x2,y1,z2, x1,y1,z2  // bottom
        };
        verts.insert(verts.end(), begin(box), end(box));
    };

    addBox(SCENE_X_MIN - t, SCENE_X_MIN, 0.0f, h, SCENE_Z_MIN - t, SCENE_Z_MAX + t); // left 
    addBox(SCENE_X_MAX, SCENE_X_MAX + t, 0.0f, h, SCENE_Z_MIN - t, SCENE_Z_MAX + t); // right 
    addBox(SCENE_X_MIN, SCENE_X_MAX, 0.0f, h, SCENE_Z_MIN - t, SCENE_Z_MIN);         // back 
    addBox(SCENE_X_MIN, SCENE_X_MAX, 0.0f, h, SCENE_Z_MAX, SCENE_Z_MAX + t);         // front 

    wallVertCount = verts.size() / 3;

    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}
void initAllBuffers()
{
    initWindmill();
    initWalls();
    auto buildSphere = [](float radius, int stackCount, int sectorCount,
            GLuint &VAO, GLuint &VBO, GLuint &EBO,
            int &indexCount) {
        std::vector<float> vertices;
        float x, y, z, xy;
        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = M_PI / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = M_PI / 2 - i * stackStep; // pi/2 to -pi/2
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }

        // Generate indices (CCW)
        std::vector<int> indices;
        int k1, k2;
        for (int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                // k1 => k2 => k1+1
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        indexCount = (int)indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int),
                indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    };

    buildSphere(1.05f, 18, 36, lightVAO, lightVBO, lightEBO, lightindices);


    float boxLines[] = {
        -1,0,-1,  1,0,-1,
        1,0,-1,  1,0, 1,
        1,0, 1, -1,0, 1,
        -1,0, 1, -1,0,-1,
        -1,1,-1,  1,1,-1,
        1,1,-1,  1,1, 1,
        1,1, 1, -1,1, 1,
        -1,1, 1, -1,1,-1,
        -1,0,-1, -1,1,-1,
        1,0,-1,  1,1,-1,
        1,0, 1,  1,1, 1,
        -1,0, 1, -1,1, 1,
    };
    glGenVertexArrays(1, &hitboxVAO);
    glGenBuffers(1, &hitboxVBO);
    glBindVertexArray(hitboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hitboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxLines), boxLines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    vector <float> carvertices = loadObjToFlatArray("models/car.obj");
    vector <pair<float,float>> carUV = loadUVToFlatArray("models/car.obj");
    glGenVertexArrays(1, &carModel.carVAO);
    glGenBuffers(1, &carModel.carVBO);
    glBindVertexArray(carModel.carVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carModel.carVBO);
    glBufferData(GL_ARRAY_BUFFER, carvertices.size() * sizeof(float), carvertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &carModel.caruvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, carModel.caruvVBO);
    glBufferData(GL_ARRAY_BUFFER, carUV.size() * sizeof(pair<float,float>), carUV.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    carModel.texture = loadTexture("textures/car_diffuse.png");

    carModel.vertices_count = carvertices.size()/6;

    vector <float> npcvertices = loadObjToFlatArray("models/car2.obj");
    vector <pair<float,float>> npcUV = loadUVToFlatArray("models/car2.obj");
    glGenVertexArrays(1, &npcModel.carVAO);
    glGenBuffers(1, &npcModel.carVBO);
    glBindVertexArray(npcModel.carVAO);
    glBindBuffer(GL_ARRAY_BUFFER, npcModel.carVBO);
    glBufferData(GL_ARRAY_BUFFER, npcvertices.size() * sizeof(float), npcvertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &npcModel.caruvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, npcModel.caruvVBO);
    glBufferData(GL_ARRAY_BUFFER, npcUV.size() * sizeof(pair<float,float>), npcUV.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);


    npcModel.vertices_count = npcvertices.size()/6;

    vector <float> streetlight = loadObjToFlatArray("models/streetlight.obj");
    vector <pair<float,float>> streetlightUV = loadUVToFlatArray("models/streetlight.obj");
    glGenVertexArrays(1, &streetlightVAO);
    glGenBuffers(1, &streetlightVBO);
    glBindVertexArray(streetlightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, streetlightVBO);
    glBufferData(GL_ARRAY_BUFFER, streetlight.size() * sizeof(float), streetlight.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &streelightuvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, streelightuvVBO);
    glBufferData(GL_ARRAY_BUFFER, streetlightUV.size() * sizeof(pair<float,float>), streetlightUV.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    streetvertices = streetlight.size()/6;


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

    string textures[] = {
        "textures/build1.jpg",
        "textures/build2.jpg",
        "textures/build3.jpg",
        "textures/build4.jpg",
        "textures/build5.jpg"
    };
    //for the buildings
    for(int i=0;i<NUM_OF_BUILDS;i++){
        string path = "models/building";
        path += to_string(i+1);
        path += ".obj";
        buildings[i].vertices = loadObjToFlatArray(path);
        buildings[i].vertices_count = buildings[i].vertices.size()/6;
        buildings[i].uvs = loadUVToFlatArray(path);

        //for the positions
        glGenVertexArrays(1, &buildings[i].VAO);
        glGenBuffers(1, &buildings[i].VBO);
        glBindVertexArray(buildings[i].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, buildings[i].VBO);
        glBufferData(GL_ARRAY_BUFFER, buildings[i].vertices.size() * sizeof(float), buildings[i].vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &buildings[i].uvVBO);
        glBindBuffer(GL_ARRAY_BUFFER, buildings[i].uvVBO);
        glBufferData(GL_ARRAY_BUFFER, buildings[i].uvs.size() * sizeof(pair<float,float>), buildings[i].uvs.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);

        buildings[i].textureID = loadTexture(textures[i]);
        glBindVertexArray(0);
    }

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
    initSwingLights();
    glEnable(GL_DEPTH_TEST);
    CompileShaders();
    initAllBuffers();
    glutMainLoop();


}
