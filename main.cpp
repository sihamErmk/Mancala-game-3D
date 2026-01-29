#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "MancalaGame.hpp"

// --- VARIABLES GLOBALES ---
unsigned int SCR_WIDTH = 1400;
unsigned int SCR_HEIGHT = 800;

// --- CAMERA ---
float camRadius = 24.0f;
float camYaw = -90.0f;
float camPitch = 65.0f;
Camera camera(glm::vec3(0.0f, 20.0f, 20.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorEnabled = true;

MancalaGame game;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- CONTROLE ECLAIRAGE ---
int lightingMode = 0; // 0 = Normal, 1 = Tamisé, 2 = Brillant
std::vector<std::string> lightingNames = {"Normal", "Tamise", "Brillant"};

// --- GESTION DES THEMES ---
struct Theme {
    std::string name;
    unsigned int boardTexID;
    unsigned int tableTexID;
    glm::vec3 bgColor;
    glm::vec3 seedColors[3]; // Couleurs spécifiques aux graines du thème
    glm::vec3 boardTint;     // Teinte générale du plateau
    float shininess;         // Brillance (bois mat, marbre brillant)
};

int currentThemeIdx = 0; // 0 = Bois, 1 = Bambou, 2 = Marbre
std::vector<Theme> themes;

struct SeedVisual { glm::vec3 offset; int colorType; };
std::vector<std::vector<SeedVisual>> pitSeedsVisuals;

unsigned int scoreTextureID;
unsigned int circleTextureID;

// --- PROTOTYPES ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
void UpdateWindowTitle(GLFWwindow* window);

glm::vec3 GetMouseRay(GLFWwindow* window, const glm::mat4& projection, const glm::mat4& view) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float x = (2.0f * xpos) / width - 1.0f;
    float y = 1.0f - (2.0f * ypos) / height;
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec3 ray_wor = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));
    return ray_wor;
}

// --- GENERATEURS DE TEXTURES ---

// 1. BOIS CLASSIQUE
unsigned int CreateWoodTexture() {
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float xCoord = (float)x / width * 12.0f; float yCoord = (float)y / height * 12.0f;
            float wood = sin(xCoord + 2.0f * sin(yCoord * 0.5f)) + 0.5f * sin(xCoord * 2.0f + yCoord);
            wood = (wood + 2.0f) / 4.0f;
            float noise = ((float)rand() / RAND_MAX) * 0.15f;
            float r = (0.85f * wood + 0.60f * (1.0f - wood) + noise) * 255;
            float g = (0.68f * wood + 0.45f * (1.0f - wood) + noise) * 255;
            float b = (0.45f * wood + 0.25f * (1.0f - wood) + noise) * 255;
            int index = (y * width + x) * 3;
            data[index] = (unsigned char)fmin(r, 255); data[index + 1] = (unsigned char)fmin(g, 255); data[index + 2] = (unsigned char)fmin(b, 255);
        }
    }
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return id;
}

// 2. BAMBOU VERT FONCÉ PROFOND
unsigned int CreateBambooTexture() {
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Fibres verticales fines
            float fiber = sin(x * 0.8f) * 0.08f + ((rand()%100)/100.0f) * 0.04f;
            // Noeuds horizontaux tous les 150 pixels
            float node = (y % 150 < 3) ? 0.15f : 0.0f;

            // Palette vert foncé profond et naturel
            float r = (0.15f - node + fiber) * 255;
            float g = (0.28f - node + fiber) * 255; // Vert dominant foncé
            float b = (0.12f - node + fiber) * 255;

            int index = (y * width + x) * 3;
            data[index] = (unsigned char)fmax(fmin(r, 255), 0);
            data[index + 1] = (unsigned char)fmax(fmin(g, 255), 0);
            data[index + 2] = (unsigned char)fmax(fmin(b, 255), 0);
        }
    }
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return id;
}

// 3. MARBRE BLEU ROYAL FONCÉ
unsigned int CreateMarbleTexture() {
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float xCoord = (float)x / width; float yCoord = (float)y / height;

            // Veines complexes du marbre bleu
            float turb = sin(xCoord * 10.0f + yCoord * 10.0f + 5.0f * ((float)rand()/RAND_MAX));
            float marble = fabs(sin(xCoord * 10.0f + turb));
            marble = pow(marble, 0.5f); // Contraste

            // Veines secondaires
            float veins = sin(xCoord * 25.0f + yCoord * 20.0f) * 0.1f;

            // Palette bleu royal foncé avec veines blanches/dorées
            float brightness = marble + veins;
            float r = (0.12f + 0.25f * brightness) * 255;  // Très peu de rouge
            float g = (0.15f + 0.30f * brightness) * 255;  // Un peu de vert
            float b = (0.35f + 0.40f * brightness) * 255;  // Bleu dominant foncé

            int index = (y * width + x) * 3;
            data[index] = (unsigned char)fmin(r, 255);
            data[index + 1] = (unsigned char)fmin(g, 255);
            data[index + 2] = (unsigned char)fmin(b, 255);
        }
    }
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    return id;
}

// --- TABLES DIVERSES ---
unsigned int CreateDarkTable() { // Pour le Bois
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int i = 0; i < width * height * 3; i+=3) {
        float n = ((rand()%100)/500.0f);
        data[i] = (0.2f + n) * 255; data[i+1] = (0.1f + n) * 255; data[i+2] = (0.05f + n) * 255;
    }
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D); return id;
}

unsigned int CreateMatTable() { // Pour le Bambou (Style Tatami vert foncé)
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float weave = (sin(x*0.5f) * sin(y*0.5f)) * 0.08f;
            // Table vert très foncé
            data[(y*width+x)*3] = (0.12f + weave) * 255;
            data[(y*width+x)*3+1] = (0.18f + weave) * 255; // Légèrement vert
            data[(y*width+x)*3+2] = (0.10f + weave) * 255;
        }
    }
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D); return id;
}

unsigned int CreateStoneTable() { // Pour le Marbre (Bleu nuit profond)
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int i = 0; i < width * height * 3; i+=3) {
        float n = ((rand()%100)/300.0f);
        data[i] = (0.08f + n * 0.5f) * 255;     // Très peu de rouge
        data[i+1] = (0.10f + n * 0.6f) * 255;   // Un peu de vert
        data[i+2] = (0.20f + n) * 255;          // Bleu dominant
    }
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D); return id;
}

// --- TEXTURES INTERFACE (Chiffres & Fond) ---
void FillBitmap(unsigned char* data, int startX, int width, int height, const int* pattern) {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 3; x++) {
            if (pattern[y * 3 + x] == 1) {
                for(int py = 0; py < 10; py++) {
                    for(int px = 0; px < 10; px++) {
                        int finalX = startX + x * 10 + px; int finalY = (4-y) * 10 + py + 5;
                        if (finalX < width && finalY < height) { int idx = (finalY * width + finalX) * 3; data[idx] = 255; data[idx+1] = 255; data[idx+2] = 255; }
                    }
                }
            }
        }
    }
}
unsigned int CreateScoreTexture() {
    const int width = 320; const int height = 64; unsigned char data[width * height * 3] = {0};
    int p0[] = {1,1,1, 1,0,1, 1,0,1, 1,0,1, 1,1,1}; int p1[] = {0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0};
    int p2[] = {1,1,1, 0,0,1, 1,1,1, 1,0,0, 1,1,1}; int p3[] = {1,1,1, 0,0,1, 1,1,1, 0,0,1, 1,1,1};
    int p4[] = {1,0,1, 1,0,1, 1,1,1, 0,0,1, 0,0,1}; int p5[] = {1,1,1, 1,0,0, 1,1,1, 0,0,1, 1,1,1};
    int p6[] = {1,1,1, 1,0,0, 1,1,1, 1,0,1, 1,1,1}; int p7[] = {1,1,1, 0,0,1, 0,0,1, 0,1,0, 0,1,0};
    int p8[] = {1,1,1, 1,0,1, 1,1,1, 1,0,1, 1,1,1}; int p9[] = {1,1,1, 1,0,1, 1,1,1, 0,0,1, 1,1,1};
    int* patterns[] = {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9};
    for(int i=0; i<10; i++) FillBitmap(data, i * 32, width, height, patterns[i]);
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}
unsigned int CreateCircleTexture() {
    const int size = 64; unsigned char data[size * size * 4];
    float cx = size / 2.0f; float cy = size / 2.0f; float rad = size / 2.0f - 2.0f;
    for(int y = 0; y < size; y++) {
        for(int x = 0; x < size; x++) {
            float dist = sqrt(pow(x - cx, 2) + pow(y - cy, 2)); int idx = (y * size + x) * 4;
            data[idx] = 255; data[idx+1] = 255; data[idx+2] = 255;
            if(dist < rad - 2.0f) data[idx+3] = 160;
            else if (dist < rad) data[idx+3] = (unsigned char)(160 * (1.0f - (dist - (rad - 2.0f)) / 2.0f));
            else data[idx+3] = 0;
        }
    }
    unsigned int id; glGenTextures(1, &id); glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}

// --- INITIALISATION DES THEMES ---
void InitThemes() {
    // 1. BOIS CLASSIQUE (Inchangé)
    Theme wood;
    wood.name = "Bois Classique";
    wood.boardTexID = CreateWoodTexture();
    wood.tableTexID = CreateDarkTable();
    wood.bgColor = glm::vec3(0.25f, 0.12f, 0.10f); // Table marron foncé
    wood.seedColors[0] = glm::vec3(0.9f, 0.85f, 0.8f); // Blanc
    wood.seedColors[1] = glm::vec3(0.2f, 0.12f, 0.08f); // Noir
    wood.seedColors[2] = glm::vec3(0.7f, 0.25f, 0.3f); // Rouge
    wood.boardTint = glm::vec3(0.85f, 0.68f, 0.45f);
    wood.shininess = 32.0f; // Brillance moyenne
    themes.push_back(wood);

    // 2. BAMBOU VERT FONCÉ (Plus sombre et plus vert)
    Theme bamboo;
    bamboo.name = "Foret de Bambou";
    bamboo.boardTexID = CreateBambooTexture();
    bamboo.tableTexID = CreateMatTable(); // Tatami vert foncé
    bamboo.bgColor = glm::vec3(0.10f, 0.15f, 0.08f); // Fond très sombre verdâtre
    bamboo.seedColors[0] = glm::vec3(0.15f, 0.35f, 0.15f); // Jade foncé
    bamboo.seedColors[1] = glm::vec3(0.70f, 0.75f, 0.70f); // Galet gris clair
    bamboo.seedColors[2] = glm::vec3(0.25f, 0.25f, 0.22f); // Pierre sombre
    bamboo.boardTint = glm::vec3(0.65f, 0.75f, 0.60f); // Teinte verte
    bamboo.shininess = 48.0f;
    themes.push_back(bamboo);

    // 3. MARBRE BLEU ROYAL FONCÉ (Plus sombre et plus bleu)
    Theme marble;
    marble.name = "Saphir Royal";
    marble.boardTexID = CreateMarbleTexture();
    marble.tableTexID = CreateStoneTable();
    marble.bgColor = glm::vec3(0.05f, 0.08f, 0.15f); // Fond bleu nuit très sombre
    marble.seedColors[0] = glm::vec3(0.90f, 0.80f, 0.30f); // Or brillant
    marble.seedColors[1] = glm::vec3(0.70f, 0.75f, 0.85f); // Argent/platine
    marble.seedColors[2] = glm::vec3(0.40f, 0.20f, 0.60f); // Améthyste profonde
    marble.boardTint = glm::vec3(0.85f, 0.90f, 1.0f); // Teinte bleutée claire
    marble.shininess = 96.0f; // Très brillant
    themes.push_back(marble);
}

void DrawScore(Shader& shader, int number, glm::vec3 position, bool isStore) {
    std::string s = std::to_string(number);
    float scale = isStore ? 0.9f : 0.6f; float spacing = 0.4f * scale;

    shader.setBool("isCircle", true); shader.setBool("isText", false);
    float bgScale = (number > 9) ? scale * 3.0f : scale * 2.5f;
    std::vector<Vertex> cv = {{{-0.5f, 0.0f, 0.5f}, {0,1,0}, {0,0}}, {{0.5f, 0.0f, 0.5f}, {0,1,0}, {1,0}}, {{0.5f, 0.0f, -0.5f}, {0,1,0}, {1,1}}, {{-0.5f, 0.0f, -0.5f}, {0,1,0}, {0,1}}};
    std::vector<unsigned int> ci = {0,1,2, 0,2,3};
    Mesh bgMesh(cv, ci);
    glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, position); m = glm::scale(m, glm::vec3(bgScale, 1.0f, bgScale));
    shader.setMat4("model", m); glBindTexture(GL_TEXTURE_2D, circleTextureID); bgMesh.Draw(shader.ID); bgMesh.Delete();

    shader.setBool("isCircle", false); shader.setBool("isText", true); glBindTexture(GL_TEXTURE_2D, scoreTextureID);
    float startX = -((s.length() - 1) * spacing) / 2.0f; position.y += 0.02f;
    for (int i = 0; i < s.length(); i++) {
        int d = s[i] - '0'; float us = d / 10.0f; float uw = 1.0f / 10.0f;
        std::vector<Vertex> v = {{{-0.5f, 0.0f, 0.5f}, {0,1,0}, {us,0}}, {{0.5f, 0.0f, 0.5f}, {0,1,0}, {us+uw,0}}, {{0.5f, 0.0f, -0.5f}, {0,1,0}, {us+uw,1}}, {{-0.5f, 0.0f, -0.5f}, {0,1,0}, {us,1}}};
        Mesh dm(v, ci);
        m = glm::mat4(1.0f); m = glm::translate(m, position + glm::vec3(startX + i * spacing, 0.0f, 0.0f)); m = glm::scale(m, glm::vec3(scale * 0.6f, 1.0f, scale));
        shader.setMat4("model", m); dm.Draw(shader.ID); dm.Delete();
    }
}

void RegenerateSeedVisuals() {
    pitSeedsVisuals.clear(); srand(time(0));
    for(int i=0; i<14; i++) {
        std::vector<SeedVisual> visuals;
        for(int s=0; s<60; s++) {
            SeedVisual sv; float spacing = 0.22f; float radius = spacing * sqrt(s + 0.5f); float angle = s * 2.39996f;
            if (i == 6 || i == 13) { sv.offset.x = (radius * 0.9f) * cos(angle); sv.offset.z = (radius * 2.0f) * sin(angle); float d = sqrt(sv.offset.x*sv.offset.x+sv.offset.z*sv.offset.z); sv.offset.y = -0.15f + (d * 0.12f); }
            else { sv.offset.x = (radius * 1.15f) * cos(angle); sv.offset.z = radius * sin(angle); sv.offset.y = -0.30f + (radius * radius * 0.9f); }
            float j = 0.02f; sv.offset.x += ((rand()%100)/100.0f*j)-(j/2); sv.offset.z += ((rand()%100)/100.0f*j)-(j/2);
            sv.colorType = rand() % 3; visuals.push_back(sv);
        }
        pitSeedsVisuals.push_back(visuals);
    }
}

void UpdateWindowTitle(GLFWwindow* window) {
    std::string title = "Mancala 3D [" + themes[currentThemeIdx].name + "] [Eclairage: " + lightingNames[lightingMode] + "] | " + game.statusMessage + " | (T) Theme | (L) Eclairage";
    glfwSetWindowTitle(window, title.c_str());
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); glfwWindowHint(GLFW_SAMPLES, 8);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mancala 3D", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); glfwSetCursorPosCallback(window, mouse_callback); glfwSetScrollCallback(window, scroll_callback); glfwSetMouseButtonCallback(window, mouse_button_callback); glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST); glEnable(GL_STENCIL_TEST); glEnable(GL_MULTISAMPLE); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    Mesh boardMesh = Geometry::CreateCube(); Mesh pitInteriorMesh = Geometry::CreateBowl(0.75f, 48, 24); Mesh seedMesh = Geometry::CreateSphere(0.22f); Mesh tableMesh = Geometry::CreatePlane();

    scoreTextureID = CreateScoreTexture();
    circleTextureID = CreateCircleTexture();

    // Initialisation des thèmes
    InitThemes();
    RegenerateSeedVisuals();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime(); deltaTime = currentFrame - lastFrame; lastFrame = currentFrame;
        processInput(window); game.Update(deltaTime);

        float camX = camRadius * cos(glm::radians(camYaw)) * cos(glm::radians(camPitch)); float camY = camRadius * sin(glm::radians(camPitch)); float camZ = camRadius * sin(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        camera.Position = glm::vec3(camX, camY, camZ); camera.Front = glm::normalize(glm::vec3(0,0,0) - camera.Position); camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f))); camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));

        UpdateWindowTitle(window);
        Theme& currentTheme = themes[currentThemeIdx];

        glStencilMask(0xFF);
        glClearColor(currentTheme.bgColor.r, currentTheme.bgColor.g, currentTheme.bgColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        if (cursorEnabled) { glm::vec3 rayDir = GetMouseRay(window, projection, view); game.UpdateHover(camera.Position, rayDir, false); }

        // --- CONFIGURATION DE L'ECLAIRAGE SELON LE MODE ---
        glm::vec3 lightPos, lightColor;
        float ambientStrength;

        if (lightingMode == 0) { // Normal
            lightPos = glm::vec3(5.0f, 25.0f, 10.0f);
            lightColor = glm::vec3(0.85f, 0.83f, 0.80f); // Réduit l'intensité
            ambientStrength = 0.20f; // Éclairage ambiant réduit
        } else if (lightingMode == 1) { // Tamisé
            lightPos = glm::vec3(0.0f, 15.0f, 8.0f);
            lightColor = glm::vec3(0.65f, 0.60f, 0.50f); // Lumière chaude faible
            ambientStrength = 0.08f; // Très sombre
        } else { // Brillant
            lightPos = glm::vec3(0.0f, 30.0f, 5.0f);
            lightColor = glm::vec3(1.0f, 0.98f, 0.95f); // Blanc normal
            ambientStrength = 0.40f; // Très lumineux
        }

        shader.use(); shader.setMat4("projection", projection); shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setFloat("ambientStrength", ambientStrength); // Passe l'éclairage ambiant au shader
        shader.setBool("isText", false); shader.setBool("isCircle", false);

        // --- Rendu ---

        // 1. Pochoir
        glStencilFunc(GL_ALWAYS, 1, 0xFF); glStencilMask(0xFF); glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); glDepthMask(GL_FALSE); shader.setBool("useTexture", false);
        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;
            glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, pit.position); float sx = (pit.id==6||pit.id==13)?1.4f:1.15f; float sz = (pit.id==6||pit.id==13)?2.8f:1.35f; m = glm::scale(m, glm::vec3(sx, 1.0f, sz)); shader.setMat4("model", m); pitInteriorMesh.Draw(shader.ID);
        }

        // 2. Plateau (Theme Actif)
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF); glStencilMask(0x00); glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); glDepthMask(GL_TRUE);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, currentTheme.boardTexID);
        shader.setInt("texture1", 0); shader.setBool("useTexture", true);
        shader.setVec3("objectColor", currentTheme.boardTint);

        glm::mat4 m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(0.0f, -0.8f, 0.0f)); m = glm::scale(m, glm::vec3(19.0f, 0.8f, 7.8f)); shader.setMat4("model", m); boardMesh.Draw(shader.ID);
        // Bordures
        shader.setVec3("objectColor", currentTheme.boardTint * 0.85f); // Un peu plus sombre
        m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(-9.8f, -0.5f, 0.0f)); m = glm::scale(m, glm::vec3(0.6f, 1.1f, 8.0f)); shader.setMat4("model", m); boardMesh.Draw(shader.ID);
        m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(9.8f, -0.5f, 0.0f)); m = glm::scale(m, glm::vec3(0.6f, 1.1f, 8.0f)); shader.setMat4("model", m); boardMesh.Draw(shader.ID);
        m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(0.0f, -0.5f, -4.1f)); m = glm::scale(m, glm::vec3(19.0f, 1.1f, 0.6f)); shader.setMat4("model", m); boardMesh.Draw(shader.ID);
        m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(0.0f, -0.5f, 4.1f)); m = glm::scale(m, glm::vec3(19.0f, 1.1f, 0.6f)); shader.setMat4("model", m); boardMesh.Draw(shader.ID);

        // 3. Table (Theme Actif)
        glStencilFunc(GL_ALWAYS, 1, 0xFF); glBindTexture(GL_TEXTURE_2D, currentTheme.tableTexID); shader.setBool("useTexture", true); shader.setVec3("objectColor", glm::vec3(1.0f));
        m = glm::mat4(1.0f); m = glm::translate(m, glm::vec3(0.0f, -2.0f, 0.0f)); shader.setMat4("model", m); tableMesh.Draw(shader.ID);

        // 4. Interieur Trous + Graines
        shader.setBool("useTexture", false);
        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;
            float sx = (pit.id==6||pit.id==13)?1.4f:1.15f; float sz = (pit.id==6||pit.id==13)?2.8f:1.35f;
            glm::mat4 hm = glm::mat4(1.0f); hm = glm::translate(hm, pit.position); hm = glm::scale(hm, glm::vec3(sx, 1.6f, sz)); shader.setMat4("model", hm);
            glm::vec3 pitColor = currentTheme.boardTint * 0.65f; // Plus sombre
            if (pit.isHovered && pit.isActive) pitColor = currentTheme.boardTint * 0.85f;
            shader.setVec3("objectColor", pitColor); pitInteriorMesh.Draw(shader.ID);

            for(int s = 0; s < pit.seeds; s++) {
                SeedVisual& sv = pitSeedsVisuals[pit.id][s % 60];
                m = glm::mat4(1.0f); m = glm::translate(m, pit.position + sv.offset); m = glm::scale(m, glm::vec3(1.0f)); shader.setMat4("model", m);
                // Couleur graine selon le theme
                glm::vec3 seedColor = currentTheme.seedColors[sv.colorType];
                shader.setVec3("objectColor", seedColor); seedMesh.Draw(shader.ID);
            }
        }
        if (game.state == ANIMATING) {
            m = glm::mat4(1.0f); m = glm::translate(m, game.activeSeed.currentPos); m = glm::scale(m, glm::vec3(1.0f)); shader.setMat4("model", m);
            shader.setVec3("objectColor", glm::vec3(1.0f, 0.85f, 0.3f)); seedMesh.Draw(shader.ID);
        }

        // 5. Scores
        shader.setBool("useTexture", true);
        for(const auto& pit : game.pits) {
            glm::vec3 tp = pit.position; tp.y = -1.95f;
            if (pit.id >= 0 && pit.id <= 5) tp.z = 6.5f; else if (pit.id >= 7 && pit.id <= 12) tp.z = -6.5f;
            else if (pit.id == 6) { tp.x = 12.0f; tp.z = 0.0f; } else if (pit.id == 13) { tp.x = -12.0f; tp.z = 0.0f; }
            DrawScore(shader, pit.seeds, tp, (pit.id==6||pit.id==13));
        }
        shader.setBool("isText", false); shader.setBool("isCircle", false);

        glfwSwapBuffers(window); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}

// Callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos) { if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; } float xoffset = xpos - lastX; float yoffset = lastY - ypos; lastX = xpos; lastY = ypos; if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) { camYaw += xoffset * 0.3f; camPitch += yoffset * 0.3f; if (camPitch > 89.0f) camPitch = 89.0f; if (camPitch < 10.0f) camPitch = 10.0f; } }
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { camYaw = -90.0f; camPitch = 65.0f; camRadius = 24.0f; }

    // --- CHANGEMENT DE THEME (T) ---
    static bool tPressed = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tPressed) {
        currentThemeIdx = (currentThemeIdx + 1) % themes.size();
        tPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) tPressed = false;

    // --- CHANGEMENT D'ECLAIRAGE (L) ---
    static bool lPressed = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lPressed) {
        lightingMode = (lightingMode + 1) % 3;
        lPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) lPressed = false;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camRadius -= (float)yoffset * 2.0f; if (camRadius < 10.0f) camRadius = 10.0f; if (camRadius > 50.0f) camRadius = 50.0f; }
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) { if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && cursorEnabled) { glm::mat4 p = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); glm::mat4 v = camera.GetViewMatrix(); game.ProcessClick(camera.Position, GetMouseRay(window, p, v), false); } }
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); SCR_WIDTH = width; SCR_HEIGHT = height; }
