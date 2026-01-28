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
float camRadius = 25.0f;
float camYaw = -90.0f;
float camPitch = 70.0f;
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

// Texture bois réaliste avec veines
unsigned int CreateWoodTexture() {
    const int width = 512;
    const int height = 512;
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

            float baseR = 0.75f;
            float baseG = 0.58f;
            float baseB = 0.38f;

            float darkR = 0.55f;
            float darkG = 0.42f;
            float darkB = 0.25f;

            float r = (baseR * wood + darkR * (1.0f - wood) + noise) * 255;
            float g = (baseG * wood + darkG * (1.0f - wood) + noise) * 255;
            float b = (baseB * wood + darkB * (1.0f - wood) + noise) * 255;

            int index = (y * width + x) * 3;
            data[index] = (unsigned char)fmin(r, 255);
            data[index + 1] = (unsigned char)fmin(g, 255);
            data[index + 2] = (unsigned char)fmin(b, 255);
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
    const int width = 512;
    const int height = 512;
    unsigned char data[width * height * 3];

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float noise = ((float)rand() / RAND_MAX) * 0.08f;
            float r = (0.28f + noise) * 255;
            float g = (0.15f + noise) * 255;
            float b = (0.12f + noise) * 255;

            int index = (y * width + x) * 3;
            data[index] = (unsigned char)r;
            data[index + 1] = (unsigned char)g;
            data[index + 2] = (unsigned char)b;
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
            // Dispersion un peu plus serrée par défaut
            float r = ((rand() % 100) / 100.0f) * 0.45f;
            float theta = ((rand() % 100) / 100.0f) * 6.28f;
            float y = -0.15f - ((rand() % 100) / 300.0f);
            sv.offset = glm::vec3(r * cos(theta), y, r * sin(theta));
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
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    Mesh boardMesh = Geometry::CreateCube();
    Mesh pitInteriorMesh = Geometry::CreateBowl(0.75f, 48, 24);
    Mesh seedMesh = Geometry::CreateSphere(0.18f);
    Mesh tableMesh = Geometry::CreatePlane();

    woodTextureID = CreateWoodTexture();
    tableTextureID = CreateTableTexture();
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
        glClearColor(0.28f, 0.15f, 0.12f, 1.0f);
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

            float scaleX = (pit.id == 6 || pit.id == 13) ? 1.2f : 0.9f;
            float scaleZ = (pit.id == 6 || pit.id == 13) ? 2.8f : 1.35f;

            model = glm::scale(model, glm::vec3(scaleX, 1.0f, scaleZ));
            shader.setMat4("model", model);
            pitInteriorMesh.Draw(shader.ID);
        }

        // ETAPE 2 : PLATEAU EN BOIS
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTextureID);
        shader.setInt("texture1", 0);
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(0.75f, 0.58f, 0.38f));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.8f, 0.0f));
        model = glm::scale(model, glm::vec3(19.0f, 0.8f, 7.8f));
        shader.setMat4("model", model);
        boardMesh.Draw(shader.ID);

        // Bordures
        shader.setVec3("objectColor", glm::vec3(0.68f, 0.52f, 0.32f));

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-9.8f, -0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 1.1f, 8.0f));
        shader.setMat4("model", model);
        boardMesh.Draw(shader.ID);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(9.8f, -0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.6f, 1.1f, 8.0f));
        shader.setMat4("model", model);
        boardMesh.Draw(shader.ID);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, -4.1f));
        model = glm::scale(model, glm::vec3(19.0f, 1.1f, 0.6f));
        shader.setMat4("model", model);
        boardMesh.Draw(shader.ID);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, 4.1f));
        model = glm::scale(model, glm::vec3(19.0f, 1.1f, 0.6f));
        shader.setMat4("model", model);
        boardMesh.Draw(shader.ID);

        // ETAPE 3 : TABLE ET INTERIEUR DES TROUS + GRAINES
        glStencilFunc(GL_ALWAYS, 1, 0xFF);

        // Table
        glBindTexture(GL_TEXTURE_2D, tableTextureID);
        shader.setBool("useTexture", true);
        shader.setVec3("objectColor", glm::vec3(0.28f, 0.15f, 0.12f));
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        shader.setMat4("model", model);
        tableMesh.Draw(shader.ID);

        shader.setBool("useTexture", false);
        for(const auto& pit : game.pits) {
            if (pit.isHidden) continue;

            float scaleX = (pit.id == 6 || pit.id == 13) ? 1.2f : 0.9f;
            float scaleZ = (pit.id == 6 || pit.id == 13) ? 2.8f : 1.35f;

            // Intérieur des bols
            glm::mat4 holeModel = glm::mat4(1.0f);
            holeModel = glm::translate(holeModel, pit.position);
            holeModel = glm::scale(holeModel, glm::vec3(scaleX, 1.6f, scaleZ));
            shader.setMat4("model", holeModel);

            glm::vec3 pitColor = glm::vec3(0.48f, 0.35f, 0.22f);
            if (pit.isHovered && pit.isActive) pitColor = glm::vec3(0.65f, 0.48f, 0.32f);

            shader.setVec3("objectColor", pitColor);
            pitInteriorMesh.Draw(shader.ID);

            // --- CORRECTION DU BUG D'AFFICHAGE DES GRAINES ---
            int seedCount = pit.seeds;
            for(int s = 0; s < seedCount; s++) {
                SeedVisual& sv = pitSeedsVisuals[pit.id][s % 60];

                glm::vec3 currentOffset = sv.offset;

                // On applique l'échelle sur l'offset SEULEMENT, pas sur la position du trou
                if (pit.id == 6 || pit.id == 13) {
                    currentOffset.x *= 2.5f; // Étaler en largeur
                    currentOffset.z *= 4.5f; // Étaler en longueur
                }

                glm::vec3 seedPos = pit.position + currentOffset;

                glm::mat4 seedModel = glm::mat4(1.0f);
                seedModel = glm::translate(seedModel, seedPos);
                seedModel = glm::scale(seedModel, glm::vec3(1.0f));
                shader.setMat4("model", seedModel);

                glm::vec3 seedColor;
                if (sv.colorType == 0) {
                    seedColor = glm::vec3(0.85f, 0.25f, 0.35f);
                } else if (sv.colorType == 1) {
                    seedColor = glm::vec3(0.92f, 0.88f, 0.82f);
                } else {
                    seedColor = glm::vec3(0.18f, 0.12f, 0.10f);
                }

                shader.setVec3("objectColor", seedColor);
                seedMesh.Draw(shader.ID);
            }
        }

        if (game.state == ANIMATING) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, game.activeSeed.currentPos);
            shader.setMat4("model", model);
            shader.setVec3("objectColor", glm::vec3(1.0f, 0.85f, 0.3f));
            seedMesh.Draw(shader.ID);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &woodTextureID);
    glDeleteTextures(1, &tableTextureID);
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
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camYaw = -90.0f;
        camPitch = 70.0f;
        camRadius = 25.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camRadius -= 10.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camRadius += 10.0f * deltaTime;

    if (camRadius < 10.0f) camRadius = 10.0f;
    if (camRadius > 50.0f) camRadius = 50.0f;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camRadius -= (float)yoffset * 2.0f;
    if (camRadius < 10.0f) camRadius = 10.0f;
    if (camRadius > 50.0f) camRadius = 50.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && cursorEnabled) {
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 rayDir = GetMouseRay(window, projection, view);
        game.ProcessClick(camera.Position, rayDir, false);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}
