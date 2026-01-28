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

// --- VARIABLES GLOBALES (Pas const pour le redimensionnement) ---
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

// --- THEMES ---
struct Theme {
    glm::vec3 bg;
    glm::vec3 board;
    glm::vec3 pit;
    glm::vec3 highlight;
    glm::vec3 p1Color;
    glm::vec3 p2Color;
    bool useTexture;
};

Theme themes[3] = {
    // 0: Bois
    { glm::vec3(0.15f, 0.1f, 0.1f), glm::vec3(0.6f, 0.4f, 0.2f), glm::vec3(0.45f, 0.3f, 0.15f), glm::vec3(1.0f, 0.8f, 0.4f), glm::vec3(0.2f, 0.4f, 1.0f), glm::vec3(1.0f, 0.2f, 0.2f), true },
    // 1: Neon
    { glm::vec3(0.05f, 0.05f, 0.1f), glm::vec3(0.1f, 0.1f, 0.15f), glm::vec3(0.05f, 0.05f, 0.1f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f), false },
    // 2: Marbre
    { glm::vec3(0.8f, 0.85f, 0.9f), glm::vec3(0.95f, 0.95f, 0.95f), glm::vec3(0.85f, 0.85f, 0.85f), glm::vec3(1.0f, 0.8f, 0.5f), glm::vec3(0.4f, 0.4f, 0.6f), glm::vec3(0.6f, 0.4f, 0.4f), false }
};
int currentThemeIdx = 0;

// --- CAMERA ---
float camRadius = 22.0f;
float camYaw = -90.0f;
float camPitch = 60.0f;
Camera camera(glm::vec3(0.0f, 15.0f, 20.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorEnabled = true;
bool editMode = false;
bool wireframeMode = false;

MancalaGame game;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct SeedVisual { glm::vec3 offset; int colorType; };
std::vector<std::vector<SeedVisual>> pitSeedsVisuals;
unsigned int woodTextureID;
unsigned int tableTextureID;

// --- PROTOTYPES (Déclarations avant utilisation) ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
void UpdateWindowTitle(GLFWwindow* window); // AJOUTÉ ICI POUR EVITER L'ERREUR

// --- FONCTIONS ---

glm::vec3 GetMouseRay(GLFWwindow* window, const glm::mat4& projection, const glm::mat4& view) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Taille réelle fenêtre (important si redimensionné)
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

unsigned int CreateProceduralTexture(int type) {
    const int width = 512;
    const int height = 512;
    unsigned char data[width * height * 3];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float r, g, b;
            if (type == 0) { // BOIS
                float xCoord = (float)x / width * 10.0f;
                float yCoord = (float)y / height * 10.0f;
                float value = sin(xCoord + sin(yCoord * 0.5f) + 5.0f * ((float)rand()/RAND_MAX * 0.1f));
                value = (value + 1.0f) * 0.5f;
                float r1 = 0.6f, g1 = 0.4f, b1 = 0.2f;
                float r2 = 0.35f, g2 = 0.2f, b2 = 0.1f;
                r = (r1 * value + r2 * (1.0f - value)) * 255;
                g = (g1 * value + g2 * (1.0f - value)) * 255;
                b = (b1 * value + b2 * (1.0f - value)) * 255;
            } else { // TABLE
                float noise = ((float)rand() / RAND_MAX) * 0.1f;
                r = (0.2f + noise) * 255;
                g = (0.2f + noise) * 255;
                b = (0.25f + noise) * 255;
            }
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

void RegenerateSeedVisuals() {
    pitSeedsVisuals.clear();
    srand(time(0));
    for(int i=0; i<14; i++) {
        std::vector<SeedVisual> visuals;
        for(int s=0; s<60; s++) {
            SeedVisual sv;
            float r = ((rand() % 100) / 100.0f) * 0.5f;
            float theta = ((rand() % 100) / 100.0f) * 6.28f;
            float y = -0.1f - ((rand() % 100) / 400.0f);
            sv.offset = glm::vec3(r * cos(theta), y, r * sin(theta));
            sv.colorType = rand() % 3;
            visuals.push_back(sv);
        }
        pitSeedsVisuals.push_back(visuals);
    }
}

// Définition de la fonction UpdateWindowTitle
void UpdateWindowTitle(GLFWwindow* window) {
    std::string title = "Mancala 3D | " + game.statusMessage;
    if (editMode) title += " [MODE EDIT]";
    glfwSetWindowTitle(window, title.c_str());
}

// --- MAIN ---
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mancala 3D Deluxe", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (glewInit() != GLEW_OK) return -1;

    // Stencil Buffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    Mesh boardMesh = Geometry::CreateCube();
    Mesh pitInteriorMesh = Geometry::CreateBowl(0.8f, 36, 18);
    Mesh seedMesh = Geometry::CreateSphere(0.2f);
    Mesh tableMesh = Geometry::CreatePlane();

    woodTextureID = CreateProceduralTexture(0);
    tableTextureID = CreateProceduralTexture(1);
    RegenerateSeedVisuals();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        game.Update(deltaTime);

        // Camera
        float camX = camRadius * cos(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        float camY = camRadius * sin(glm::radians(camPitch));
        float camZ = camRadius * sin(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        camera.Position = glm::vec3(camX, camY, camZ);
        camera.Front = glm::normalize(glm::vec3(0,0,0) - camera.Position);
        camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera.Up    = glm::normalize(glm::cross(camera.Right, camera.Front));

        UpdateWindowTitle(window);
        Theme& theme = themes[currentThemeIdx];

        // --- MASK STENCIL (IMPORTANT POUR QUE LE PLATEAU RESTE) ---
        glStencilMask(0xFF);
        glClearColor(theme.bg.r, theme.bg.g, theme.bg.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if (wireframeMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        if (cursorEnabled) {
            glm::vec3 rayDir = GetMouseRay(window, projection, view);
            game.UpdateHover(camera.Position, rayDir, editMode);
        }

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);

        glm::vec3 activeColor = (game.currentPlayer == 0) ? theme.p1Color : theme.p2Color;
        if(game.gameOver) activeColor = glm::vec3(0.0f, 1.0f, 0.0f);
        shader.setVec3("lightPos", 0.0f, 15.0f, 5.0f);
        shader.setVec3("lightColor", glm::mix(glm::vec3(1.0f), activeColor, 0.3f));

        // ------------------------------------------------------------------
        // ETAPE 1 : REMPLIR LE POCHOIR (LES TROUS)
        // ------------------------------------------------------------------
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        shader.setBool("useTexture", false);
        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pit.position);
            float scaleX = (pit.id == 6 || pit.id == 13) ? 1.3f : 1.0f;
            float scaleZ = (pit.id == 6 || pit.id == 13) ? 2.5f : 1.0f;

            model = glm::scale(model, glm::vec3(scaleX * 0.95f, 1.0f, scaleZ * 0.95f));
            shader.setMat4("model", model);
            pitInteriorMesh.Draw(shader.ID);
        }

        // ------------------------------------------------------------------
        // ETAPE 2 : DESSINER LE PLATEAU (Sauf là où il y a des trous)
        // ------------------------------------------------------------------
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTextureID);
        shader.setInt("texture1", 0);
        shader.setBool("useTexture", theme.useTexture);
        shader.setVec3("objectColor", theme.board);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(18.5f, 2.0f, 7.5f));
        shader.setMat4("model", model);
        boardMesh.Draw(shader.ID);

        // ------------------------------------------------------------------
        // ETAPE 3 : DESSINER LE RESTE
        // ------------------------------------------------------------------
        glStencilFunc(GL_ALWAYS, 1, 0xFF);

        // A. Table
        glBindTexture(GL_TEXTURE_2D, tableTextureID);
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(0.8f));
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));
        shader.setMat4("model", model);
        tableMesh.Draw(shader.ID);

        // B. Intérieur des Trous et Graines
        shader.setBool("useTexture", false);
        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;

            glm::mat4 posMatrix = glm::mat4(1.0f);
            posMatrix = glm::translate(posMatrix, pit.position);

            float scaleX = (pit.id == 6 || pit.id == 13) ? 1.3f : 1.0f;
            float scaleZ = (pit.id == 6 || pit.id == 13) ? 2.5f : 1.0f;

            // 1. Bol Intérieur (Creusé)
            glm::mat4 holeModel = posMatrix;
            holeModel = glm::scale(holeModel, glm::vec3(scaleX, 1.8f, scaleZ));
            holeModel = glm::translate(holeModel, glm::vec3(0.0f, 0.02f, 0.0f));
            shader.setMat4("model", holeModel);

            glm::vec3 pitColor = theme.pit;
            if (pit.isSelected) pitColor = glm::vec3(1.0f, 0.0f, 0.0f);
            else if (pit.isHovered && pit.isActive) pitColor = theme.highlight;
            else if (!pit.isActive && !(pit.id == 6 || pit.id == 13)) pitColor *= 0.5f;

            shader.setVec3("objectColor", pitColor * 0.7f);
            pitInteriorMesh.Draw(shader.ID);

            // 2. Graines
            int seedCount = pit.seeds;
            for(int s = 0; s < seedCount; s++) {
                SeedVisual& sv = pitSeedsVisuals[pit.id][s % 60];

                glm::mat4 seedModel = glm::mat4(1.0f);
                float finalX = sv.offset.x;
                float finalZ = sv.offset.z;
                // Étaler les graines dans les grands magasins
                if (pit.id == 6 || pit.id == 13) {
                    finalX *= 1.5f; finalZ *= 2.5f;
                }

                glm::vec3 seedPos = pit.position + glm::vec3(finalX, sv.offset.y - 0.1f, finalZ);

                seedModel = glm::translate(seedModel, seedPos);
                seedModel = glm::scale(seedModel, glm::vec3(1.0f));

                shader.setMat4("model", seedModel);

                glm::vec3 seedColor;
                if (sv.colorType == 0) seedColor = glm::vec3(0.2f, 0.6f, 1.0f);
                else if (sv.colorType == 1) seedColor = glm::vec3(1.0f, 0.2f, 0.2f);
                else seedColor = glm::vec3(0.2f, 0.8f, 0.2f);
                if (currentThemeIdx == 2) seedColor = glm::vec3(0.15f);

                shader.setVec3("objectColor", seedColor);
                seedMesh.Draw(shader.ID);
            }
        }

        if (game.state == ANIMATING) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, game.activeSeed.currentPos);
            shader.setMat4("model", model);
            shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 0.0f));
            seedMesh.Draw(shader.ID);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        float sensitivity = 0.3f;
        camYaw += xoffset * sensitivity;
        camPitch += yoffset * sensitivity;
        if (camPitch > 89.0f) camPitch = 89.0f;
        if (camPitch < 10.0f) camPitch = 10.0f;
    }
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camYaw = -90.0f; camPitch = 60.0f; camRadius = 22.0f;
    }
    static bool ePressed = false;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed) { editMode = !editMode; ePressed = true; }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) ePressed = false;
    static bool tPressed = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tPressed) { currentThemeIdx = (currentThemeIdx + 1) % 3; tPressed = true; }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) tPressed = false;
    static bool fPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fPressed) { wireframeMode = !wireframeMode; fPressed = true; }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fPressed = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camRadius -= 10.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camRadius += 10.0f * deltaTime;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camRadius -= (float)yoffset * 2.0f;
    if (camRadius < 5.0f) camRadius = 5.0f;
    if (camRadius > 50.0f) camRadius = 50.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && cursorEnabled) {
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 rayDir = GetMouseRay(window, projection, view);
        game.ProcessClick(camera.Position, rayDir, editMode);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}
