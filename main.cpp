#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>
#include <algorithm>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "MancalaGame.hpp"

// --- PARAMETRES ---
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

struct Theme {
    glm::vec3 bg;
    glm::vec3 board;
    glm::vec3 pit;
    glm::vec3 highlight;
    glm::vec3 lightColor;
};

Theme currentTheme = {
    glm::vec3(0.10f, 0.08f, 0.10f), // Fond
    glm::vec3(0.50f, 0.30f, 0.20f), // Plateau (Bois un peu plus clair)
    glm::vec3(0.30f, 0.18f, 0.10f), // Trous (Plus sombre pour profondeur)
    glm::vec3(1.00f, 0.80f, 0.40f), // Highlight
    glm::vec3(1.00f, 0.95f, 0.90f)  // Lumière
};

glm::vec3 seedPalette[4] = {
    glm::vec3(0.90f, 0.90f, 0.85f), glm::vec3(0.85f, 0.10f, 0.20f),
    glm::vec3(0.10f, 0.30f, 0.80f), glm::vec3(0.10f, 0.70f, 0.30f)
};

// --- GLOBAL ---
float camRadius = 24.0f;
float camYaw = -90.0f;
float targetYaw = -90.0f;
float camPitch = 65.0f;
glm::vec3 camTarget(0.0f, 0.0f, 0.0f);
Camera camera(glm::vec3(0.0f, 20.0f, 15.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorEnabled = true;
bool editMode = false;

MancalaGame game;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct SeedVisual { glm::vec3 offset; int colorIdx; };
std::vector<std::vector<SeedVisual>> pitSeedsVisuals;
unsigned int woodTextureID;

// --- PROTOTYPES ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
glm::vec3 GetMouseRay(GLFWwindow* window, const glm::mat4& projection, const glm::mat4& view);
float lerp(float a, float b, float f) { return a + f * (b - a); }

// --- TEXTURE ---
unsigned int CreateWoodTexture() {
    const int width = 512; const int height = 512;
    unsigned char data[width * height * 3];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float wave = sin(x * 0.04f + sin(y * 0.01f) * 6.0f) * 0.5f + 0.5f;
            unsigned char r = (unsigned char)(100 + wave * 60);
            unsigned char g = (unsigned char)(60 + wave * 40);
            unsigned char b = (unsigned char)(30 + wave * 20);
            int index = (y * width + x) * 3;
            data[index] = r; data[index + 1] = g; data[index + 2] = b;
        }
    }
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return textureID;
}

// --- GRAINES (CORRECTION) ---
void RegenerateSeedVisuals() {
    pitSeedsVisuals.clear();
    srand(time(0));
    float ballRadius = 0.22f;

    for(int i=0; i<14; i++) {
        std::vector<SeedVisual> visuals;
        std::vector<glm::vec3> placedPositions;

        // CORRECTION: Position de départ BEAUCOUP plus haute pour être bien visible
        float currentLayerY = -0.2f;  // TRES haut, presque à la surface
        float layerRadiusLimit = 0.25f;

        for(int s=0; s<70; s++) {
            SeedVisual sv;
            bool placed = false;
            int attempts = 0;
            while(!placed && attempts < 250) {
                float r = ((rand() % 100) / 100.0f) * layerRadiusLimit;
                float theta = ((rand() % 100) / 100.0f) * 6.28f;
                glm::vec3 candidate = glm::vec3(r * cos(theta), currentLayerY, r * sin(theta));

                // Formule du bol : x^2 + z^2. Si on s'éloigne, on monte légèrement.
                float bowlFloor = (r * r * 0.5f) - 0.35f;  // Formule plus douce
                if (candidate.y < bowlFloor) candidate.y = bowlFloor + ballRadius;

                bool collision = false;
                for(const auto& existing : placedPositions) {
                    if (glm::length(candidate - existing) < ballRadius * 1.5f) { collision = true; break; }
                }
                if (!collision) {
                    sv.offset = candidate; sv.colorIdx = rand() % 4;
                    visuals.push_back(sv); placedPositions.push_back(candidate); placed = true;
                } else attempts++;
            }
            if (!placed) {
                currentLayerY += ballRadius * 0.6f; layerRadiusLimit += 0.05f;
                if (layerRadiusLimit > 0.75f) layerRadiusLimit = 0.75f;
                s--;
            }
        }
        pitSeedsVisuals.push_back(visuals);
    }
}

void UpdateWindowTitle(GLFWwindow* window) {
    std::string title = "Mancala 3D | " + game.statusMessage;
    glfwSetWindowTitle(window, title.c_str());
}

// --- MAIN ---
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mancala 3D Deluxe", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (glewInit() != GLEW_OK) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    Mesh cube = Geometry::CreateCube();
    Mesh pitMesh = Geometry::CreateHemisphere(0.85f);
    Mesh marbleMesh = Geometry::CreateSphere(0.22f);

    woodTextureID = CreateWoodTexture();
    RegenerateSeedVisuals();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        game.Update(deltaTime);

        // Animation Camera
        if (game.turnChanged) {
            if (game.currentPlayer == 0) targetYaw = -90.0f;
            else targetYaw = 90.0f;
            game.turnChanged = false;
        }
        float moveSpeed = 3.0f * deltaTime;
        if (abs(targetYaw - camYaw) > 0.1f) camYaw = lerp(camYaw, targetYaw, moveSpeed);
        else camYaw = targetYaw;

        float camX = camRadius * cos(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        float camY = camRadius * sin(glm::radians(camPitch));
        float camZ = camRadius * sin(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        camera.Position = glm::vec3(camX, camY, camZ);
        camera.Front = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - camera.Position);
        camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera.Up    = glm::normalize(glm::cross(camera.Right, camera.Front));

        if (game.state == ANIMATING && game.activeSeed.progress == 0.0f) {
            game.activeSeed.startPos = game.activeSeed.endPos;
        }

        UpdateWindowTitle(window);

        glClearColor(currentTheme.bg.r, currentTheme.bg.g, currentTheme.bg.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("viewPos", camera.Position);

        // Lumière sur le côté pour créer des ombres dans les trous
        shader.setVec3("lightPos", 10.0f, 20.0f, 10.0f);

        shader.setVec3("lightColor", currentTheme.lightColor);
        shader.setBool("lightEnabled", true);
        shader.setFloat("materialShininess", 64.0f);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        if (cursorEnabled) {
            glm::vec3 rayDir = GetMouseRay(window, projection, view);
            game.UpdateHover(camera.Position, rayDir, editMode);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTextureID);
        shader.setInt("texture1", 0);

        // 1. PLATEAU
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", currentTheme.board);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
        model = glm::scale(model, glm::vec3(19.0f, 0.5f, 8.0f));
        shader.setMat4("model", model);
        cube.Draw(shader.ID);

        // 2. BOLS (Creux) - CORRECTION MAJEURE
        shader.setBool("useTexture", false);
        glDisable(GL_CULL_FACE); // Voir l'intérieur
        glEnable(GL_BLEND);      // Activer la transparence pour mieux voir
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;
            model = glm::mat4(1.0f);
            model = glm::translate(model, pit.position);
            bool isStore = (pit.id == 6 || pit.id == 13);
            // CORRECTION: Scale encore plus négatif pour des bols plus profonds
            if (isStore) model = glm::scale(model, glm::vec3(1.4f, -1.2f, 2.6f));
            else model = glm::scale(model, glm::vec3(1.0f, -1.2f, 1.0f));
            shader.setMat4("model", model);

            if (pit.isSelected) shader.setVec3("objectColor", glm::vec3(0.8f, 0.2f, 0.2f));
            else if (pit.isHovered && !isStore) shader.setVec3("objectColor", currentTheme.highlight);
            else shader.setVec3("objectColor", currentTheme.pit);
            pitMesh.Draw(shader.ID);

            // 3. GRAINES - CORRECTION: modulo 70 au lieu de 60
            int seedCount = pit.seeds;
            if(seedCount > 0 && pit.id == 0) {
                // Debug pour le premier trou
                std::cout << "Pit " << pit.id << " has " << seedCount << " seeds, visual size: " << pitSeedsVisuals[pit.id].size() << std::endl;
            }
            for(int s = 0; s < seedCount; s++) {
                if(pit.id >= pitSeedsVisuals.size() || s >= pitSeedsVisuals[pit.id].size()) {
                    std::cout << "ERROR: Out of bounds! pit.id=" << pit.id << " s=" << s << std::endl;
                    break;
                }
                SeedVisual& sv = pitSeedsVisuals[pit.id][s % 70];
                glm::mat4 seedModel = glm::mat4(1.0f);
                glm::vec3 pos = pit.position + sv.offset;
                if (isStore) { pos.x *= 1.2f; pos.z *= 2.0f; }
                seedModel = glm::translate(seedModel, pos);
                seedModel = glm::scale(seedModel, glm::vec3(1.0f, 1.0f, 1.0f));  // Taille normale
                shader.setMat4("model", seedModel);
                shader.setVec3("objectColor", seedPalette[sv.colorIdx]);
                marbleMesh.Draw(shader.ID);
            }
        }
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);

        // 4. ANIMATION
        if (game.state == ANIMATING) {
            glm::mat4 animModel = glm::mat4(1.0f);
            animModel = glm::translate(animModel, game.activeSeed.currentPos);
            animModel = glm::scale(animModel, glm::vec3(1.0f));
            shader.setMat4("model", animModel);
            shader.setVec3("objectColor", glm::vec3(1.0f));
            marbleMesh.Draw(shader.ID);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        camYaw += xoffset * 0.3f; camPitch += yoffset * 0.3f;
        if (camPitch > 89.0f) camPitch = 89.0f; if (camPitch < 10.0f) camPitch = 10.0f;
    }
}
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    static bool ePressed = false;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed) { editMode = !editMode; ePressed = true; }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) ePressed = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camRadius -= 10.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camRadius += 10.0f * deltaTime;
    if (camRadius < 10.0f) camRadius = 10.0f; if (camRadius > 50.0f) camRadius = 50.0f;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camRadius -= (float)yoffset * 2.0f;
    if (camRadius < 10.0f) camRadius = 10.0f; if (camRadius > 50.0f) camRadius = 50.0f;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && cursorEnabled) {
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 rayDir = GetMouseRay(window, projection, view);
        game.ProcessClick(camera.Position, rayDir, editMode);
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
glm::vec3 GetMouseRay(GLFWwindow* window, const glm::mat4& projection, const glm::mat4& view) {
    double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos);
    float x = (2.0f * xpos) / SCR_WIDTH - 1.0f; float y = 1.0f - (2.0f * ypos) / SCR_HEIGHT;
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    return glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));
}
