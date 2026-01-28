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

struct SeedVisual { glm::vec3 offset; int colorType; };
std::vector<std::vector<SeedVisual>> pitSeedsVisuals;
unsigned int woodTextureID;
unsigned int tableTextureID;
unsigned int scoreTextureID; // NOUVEAU : ID pour la texture des chiffres

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

// --- TEXTURES ---

unsigned int CreateWoodTexture() {
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    srand(12345);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float xCoord = (float)x / width * 12.0f;
            float yCoord = (float)y / height * 12.0f;
            float wood = sin(xCoord + 2.0f * sin(yCoord * 0.5f));
            wood += 0.5f * sin(xCoord * 2.0f + yCoord);
            wood += 0.25f * sin(xCoord * 4.0f);
            wood = (wood + 2.0f) / 4.0f;
            float noise = ((float)rand() / RAND_MAX) * 0.15f;
            float baseR = 0.85f; float baseG = 0.68f; float baseB = 0.45f;
            float darkR = 0.60f; float darkG = 0.45f; float darkB = 0.25f;
            float r = (baseR * wood + darkR * (1.0f - wood) + noise) * 255;
            float g = (baseG * wood + darkG * (1.0f - wood) + noise) * 255;
            float b = (baseB * wood + darkB * (1.0f - wood) + noise) * 255;
            int index = (y * width + x) * 3;
            data[index] = (unsigned char)fmin(r, 255); data[index + 1] = (unsigned char)fmin(g, 255); data[index + 2] = (unsigned char)fmin(b, 255);
        }
    }
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return textureID;
}

unsigned int CreateTableTexture() {
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float noise = ((float)rand() / RAND_MAX) * 0.08f;
            float r = (0.20f + noise) * 255; float g = (0.10f + noise) * 255; float b = (0.08f + noise) * 255;
            int index = (y * width + x) * 3;
            data[index] = (unsigned char)r; data[index + 1] = (unsigned char)g; data[index + 2] = (unsigned char)b;
        }
    }
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return textureID;
}

// --- NOUVEAU : GENERATION DE TEXTURE DE CHIFFRES ---
// Crée une texture contenant les chiffres 0123456789 horizontalement
void FillBitmap(unsigned char* data, int startX, int width, int height, const int* pattern) {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 3; x++) {
            if (pattern[y * 3 + x] == 1) {
                // Dessiner un pixel blanc (taille 10x10 pour être visible)
                for(int py = 0; py < 10; py++) {
                    for(int px = 0; px < 10; px++) {
                        int finalX = startX + x * 10 + px;
                        int finalY = (4-y) * 10 + py + 5; // Inverser Y
                        if (finalX < width && finalY < height) {
                            int idx = (finalY * width + finalX) * 3;
                            data[idx] = 255; data[idx+1] = 255; data[idx+2] = 255;
                        }
                    }
                }
            }
        }
    }
}

unsigned int CreateScoreTexture() {
    // Texture large de 10 chiffres. Chaque chiffre ~30x50 pixels + marges
    const int width = 320;
    const int height = 64;
    unsigned char data[width * height * 3] = {0}; // Tout noir par défaut

    // Patrons des chiffres 3x5 pixels
    int p0[] = {1,1,1, 1,0,1, 1,0,1, 1,0,1, 1,1,1};
    int p1[] = {0,1,0, 1,1,0, 0,1,0, 0,1,0, 1,1,1};
    int p2[] = {1,1,1, 0,0,1, 1,1,1, 1,0,0, 1,1,1};
    int p3[] = {1,1,1, 0,0,1, 1,1,1, 0,0,1, 1,1,1};
    int p4[] = {1,0,1, 1,0,1, 1,1,1, 0,0,1, 0,0,1};
    int p5[] = {1,1,1, 1,0,0, 1,1,1, 0,0,1, 1,1,1};
    int p6[] = {1,1,1, 1,0,0, 1,1,1, 1,0,1, 1,1,1};
    int p7[] = {1,1,1, 0,0,1, 0,0,1, 0,1,0, 0,1,0};
    int p8[] = {1,1,1, 1,0,1, 1,1,1, 1,0,1, 1,1,1};
    int p9[] = {1,1,1, 1,0,1, 1,1,1, 0,0,1, 1,1,1};

    int* patterns[] = {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9};

    for(int i=0; i<10; i++) {
        FillBitmap(data, i * 32, width, height, patterns[i]);
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return textureID;
}

// --- FONCTION POUR DESSINER UN NOMBRE ---
void DrawScore(Shader& shader, Mesh& quadMesh, int number, glm::vec3 position) {
    std::string s = std::to_string(number);
    float scale = 0.5f;
    float spacing = 0.35f; // Espace entre les chiffres

    // Centrer le texte : décaler vers la gauche selon le nombre de chiffres
    float startX = -((s.length() - 1) * spacing) / 2.0f;

    for (int i = 0; i < s.length(); i++) {
        int digit = s[i] - '0';

        // Calcul des coordonnées UV pour ce chiffre (Atlas de 10 chiffres)
        float u_start = digit / 10.0f;
        float u_width = 1.0f / 10.0f; // Largeur d'un chiffre (0.1)

        // Modifier les UV du mesh à la volée est lent, mais pour ce projet c'est acceptable
        // Ou mieux : Passer l'offset UV au shader. Ici on va utiliser un hack d'échelle/offset texture
        // Hack: On déplace la texture dans le shader via la matrice Texture (non dispo en core moderne simple)
        // Alternative simple : Dessiner un quad avec les bons UVs.
        // Pour simplifier ici avec Mesh existant : on va supposer que Mesh Quad a UV 0..1

        // Pour faire simple sans changer Mesh class : on triche dans le shader avec l'offset ?
        // Non, on va utiliser un mesh dynamique ou des uniforms.
        // Option simple : Uniform "texOffset" dans le shader ? Non, modifions le shader plus tard.
        // Option Rapide : Recréer un Quad à la volée. C'est pas opti mais ça marche.

        std::vector<Vertex> verts = {
            {{-0.5f, 0.0f,  0.5f}, {0,1,0}, {u_start, 0.0f}},
            {{ 0.5f, 0.0f,  0.5f}, {0,1,0}, {u_start + u_width, 0.0f}},
            {{ 0.5f, 0.0f, -0.5f}, {0,1,0}, {u_start + u_width, 1.0f}},
            {{-0.5f, 0.0f, -0.5f}, {0,1,0}, {u_start, 1.0f}}
        };
        std::vector<unsigned int> inds = {0, 1, 2, 0, 2, 3};
        Mesh digitMesh(verts, inds); // Création dynamique (lourd mais ok pour 14 nombres)

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position + glm::vec3(startX + i * spacing, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale, 1.0f, scale)); // Taille du chiffre

        shader.setMat4("model", model);
        digitMesh.Draw(shader.ID);
        digitMesh.Delete(); // Nettoyage immédiat
    }
}

void RegenerateSeedVisuals() {
    pitSeedsVisuals.clear();
    srand(time(0));
    for(int i=0; i<14; i++) {
        std::vector<SeedVisual> visuals;
        for(int s=0; s<60; s++) {
            SeedVisual sv;
            float spacing = 0.22f;
            float radius = spacing * sqrt(s + 0.5f);
            float angle = s * 2.39996f;
            if (i == 6 || i == 13) {
                sv.offset.x = (radius * 0.9f) * cos(angle);
                sv.offset.z = (radius * 2.0f) * sin(angle);
                float dist = sqrt(sv.offset.x*sv.offset.x + sv.offset.z*sv.offset.z);
                sv.offset.y = -0.15f + (dist * 0.12f);
            } else {
                sv.offset.x = (radius * 1.15f) * cos(angle);
                sv.offset.z = radius * sin(angle);
                sv.offset.y = -0.30f + (radius * radius * 0.9f);
            }
            float jitter = 0.02f;
            sv.offset.x += ((rand() % 100) / 100.0f * jitter) - (jitter/2);
            sv.offset.z += ((rand() % 100) / 100.0f * jitter) - (jitter/2);
            sv.colorType = rand() % 3;
            visuals.push_back(sv);
        }
        pitSeedsVisuals.push_back(visuals);
    }
}

void UpdateWindowTitle(GLFWwindow* window) {
    std::string title = "Mancala 3D - Style Realiste | " + game.statusMessage;
    glfwSetWindowTitle(window, title.c_str());
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mancala 3D - Realiste", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_MULTISAMPLE);

    // Activation de la transparence (Alpha Blending) pour le texte
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    Mesh boardMesh = Geometry::CreateCube();
    Mesh pitInteriorMesh = Geometry::CreateBowl(0.75f, 48, 24);
    Mesh seedMesh = Geometry::CreateSphere(0.22f);
    Mesh tableMesh = Geometry::CreatePlane();

    // Mesh générique pour les chiffres (mis à jour à la volée)
    Mesh quadMesh = Geometry::CreatePlane();

    woodTextureID = CreateWoodTexture();
    tableTextureID = CreateTableTexture();
    scoreTextureID = CreateScoreTexture(); // Générer la texture des chiffres

    RegenerateSeedVisuals();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        game.Update(deltaTime);

        float camX = camRadius * cos(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        float camY = camRadius * sin(glm::radians(camPitch));
        float camZ = camRadius * sin(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        camera.Position = glm::vec3(camX, camY, camZ);
        camera.Front = glm::normalize(glm::vec3(0,0,0) - camera.Position);
        camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera.Up    = glm::normalize(glm::cross(camera.Right, camera.Front));

        UpdateWindowTitle(window);

        glStencilMask(0xFF);
        glClearColor(0.25f, 0.12f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        if (cursorEnabled) {
            glm::vec3 rayDir = GetMouseRay(window, projection, view);
            game.UpdateHover(camera.Position, rayDir, false);
        }

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", 5.0f, 25.0f, 10.0f);
        shader.setVec3("lightColor", glm::vec3(1.0f, 0.98f, 0.95f));
        shader.setBool("isText", false); // Par défaut, ce n'est pas du texte

        // ETAPE 1 : POCHOIR (TROUS)
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        shader.setBool("useTexture", false);
        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pit.position);
            float scaleX = (pit.id == 6 || pit.id == 13) ? 1.4f : 1.15f;
            float scaleZ = (pit.id == 6 || pit.id == 13) ? 2.8f : 1.35f;
            model = glm::scale(model, glm::vec3(scaleX, 1.0f, scaleZ));
            shader.setMat4("model", model);
            pitInteriorMesh.Draw(shader.ID);
        }

        // ETAPE 2 : PLATEAU
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTextureID);
        shader.setInt("texture1", 0);
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(0.85f, 0.68f, 0.45f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.8f, 0.0f));
        model = glm::scale(model, glm::vec3(19.0f, 0.8f, 7.8f));
        shader.setMat4("model", model);
        boardMesh.Draw(shader.ID);
        // Bordures
        shader.setVec3("objectColor", glm::vec3(0.75f, 0.60f, 0.40f));
        model = glm::mat4(1.0f); model = glm::translate(model, glm::vec3(-9.8f, -0.5f, 0.0f)); model = glm::scale(model, glm::vec3(0.6f, 1.1f, 8.0f)); shader.setMat4("model", model); boardMesh.Draw(shader.ID);
        model = glm::mat4(1.0f); model = glm::translate(model, glm::vec3(9.8f, -0.5f, 0.0f)); model = glm::scale(model, glm::vec3(0.6f, 1.1f, 8.0f)); shader.setMat4("model", model); boardMesh.Draw(shader.ID);
        model = glm::mat4(1.0f); model = glm::translate(model, glm::vec3(0.0f, -0.5f, -4.1f)); model = glm::scale(model, glm::vec3(19.0f, 1.1f, 0.6f)); shader.setMat4("model", model); boardMesh.Draw(shader.ID);
        model = glm::mat4(1.0f); model = glm::translate(model, glm::vec3(0.0f, -0.5f, 4.1f)); model = glm::scale(model, glm::vec3(19.0f, 1.1f, 0.6f)); shader.setMat4("model", model); boardMesh.Draw(shader.ID);

        // ETAPE 3 : TABLE ET INTERIEUR + GRAINES
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glBindTexture(GL_TEXTURE_2D, tableTextureID);
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(0.28f, 0.15f, 0.12f));
        model = glm::mat4(1.0f); model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f)); shader.setMat4("model", model); tableMesh.Draw(shader.ID);

        shader.setBool("useTexture", false);
        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;
            float scaleX = (pit.id == 6 || pit.id == 13) ? 1.4f : 1.15f;
            float scaleZ = (pit.id == 6 || pit.id == 13) ? 2.8f : 1.35f;
            glm::mat4 holeModel = glm::mat4(1.0f);
            holeModel = glm::translate(holeModel, pit.position);
            holeModel = glm::scale(holeModel, glm::vec3(scaleX, 1.6f, scaleZ));
            shader.setMat4("model", holeModel);
            glm::vec3 pitColor = glm::vec3(0.65f, 0.50f, 0.35f);
            if (pit.isHovered && pit.isActive) pitColor = glm::vec3(0.75f, 0.60f, 0.45f);
            shader.setVec3("objectColor", pitColor);
            pitInteriorMesh.Draw(shader.ID);

            int seedCount = pit.seeds;
            for(int s = 0; s < seedCount; s++) {
                SeedVisual& sv = pitSeedsVisuals[pit.id][s % 60];
                glm::vec3 seedPos = pit.position + sv.offset;
                glm::mat4 seedModel = glm::mat4(1.0f);
                seedModel = glm::translate(seedModel, seedPos);
                seedModel = glm::scale(seedModel, glm::vec3(1.0f));
                shader.setMat4("model", seedModel);
                glm::vec3 seedColor;
                if (sv.colorType == 0) seedColor = glm::vec3(0.90f, 0.85f, 0.80f);
                else if (sv.colorType == 1) seedColor = glm::vec3(0.20f, 0.12f, 0.08f);
                else seedColor = glm::vec3(0.70f, 0.25f, 0.30f);
                shader.setVec3("objectColor", seedColor);
                seedMesh.Draw(shader.ID);
            }
        }
        if (game.state == ANIMATING) {
            model = glm::mat4(1.0f); model = glm::translate(model, game.activeSeed.currentPos); model = glm::scale(model, glm::vec3(1.0f)); shader.setMat4("model", model);
            shader.setVec3("objectColor", glm::vec3(1.0f, 0.85f, 0.3f)); seedMesh.Draw(shader.ID);
        }

        // --- ETAPE 4 : AFFICHAGE DES SCORES (TEXTE) ---
        // On active le mode texte pour le shader
        shader.setBool("isText", true);
        shader.setBool("useTexture", true);
        glBindTexture(GL_TEXTURE_2D, scoreTextureID);

        for(const auto& pit : game.pits) {
            // Positionnement des chiffres comme sur l'image
            glm::vec3 textPos = pit.position;

            // J1 (Bas, 0-5) : Texte en dessous
            if (pit.id >= 0 && pit.id <= 5) {
                textPos.z += 2.2f;
            }
            // J2 (Haut, 7-12) : Texte au dessus
            else if (pit.id >= 7 && pit.id <= 12) {
                textPos.z -= 2.2f;
            }
            // Magasin J1 (Droite, 6) : Texte en dessous
            else if (pit.id == 6) {
                textPos.z += 3.5f;
            }
             // Magasin J2 (Gauche, 13) : Texte au dessus
            else if (pit.id == 13) {
                textPos.z -= 3.5f;
            }

            // On soulève un peu le texte pour qu'il ne soit pas dans le bois (z-fighting)
            textPos.y = -0.18f;

            DrawScore(shader, quadMesh, pit.seeds, textPos);
        }

        shader.setBool("isText", false); // On désactive pour la frame suivante

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &woodTextureID);
    glDeleteTextures(1, &tableTextureID);
    glDeleteTextures(1, &scoreTextureID);
    glfwTerminate();
    return 0;
}

// Callbacks inchangés...
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos; lastX = xpos; lastY = ypos;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        float sensitivity = 0.3f; camYaw += xoffset * sensitivity; camPitch += yoffset * sensitivity;
        if (camPitch > 89.0f) camPitch = 89.0f; if (camPitch < 10.0f) camPitch = 10.0f;
    }
}
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { camYaw = -90.0f; camPitch = 65.0f; camRadius = 24.0f; }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camRadius -= 10.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camRadius += 10.0f * deltaTime;
    if (camRadius < 10.0f) camRadius = 10.0f; if (camRadius > 50.0f) camRadius = 50.0f;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camRadius -= (float)yoffset * 2.0f; if (camRadius < 10.0f) camRadius = 10.0f; if (camRadius > 50.0f) camRadius = 50.0f;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && cursorEnabled) {
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 rayDir = GetMouseRay(window, projection, view);
        game.ProcessClick(camera.Position, rayDir, false);
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); SCR_WIDTH = width; SCR_HEIGHT = height; }
