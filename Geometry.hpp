#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "Mesh.hpp"
#include <cmath>

class Geometry {
public:
    static Mesh CreateCube() {
        std::vector<Vertex> vertices = {
            // Back face
            {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},
            // Front face
            {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {0.0f, 0.0f}},
            // Left face
            {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            // Right face
            {{ 0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
            // Bottom face
            {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
            // Top face
            {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}}
        };
        std::vector<unsigned int> indices;
        for(unsigned int i=0; i<36; i++) indices.push_back(i);
        return Mesh(vertices, indices);
    }

    static Mesh CreatePlane() {
        std::vector<Vertex> vertices = {
            {{ 25.0f, -0.5f,  25.0f}, {0.0f, 1.0f, 0.0f}, {25.0f, 0.0f}},
            {{-25.0f, -0.5f,  25.0f}, {0.0f, 1.0f, 0.0f}, {0.0f,  0.0f}},
            {{-25.0f, -0.5f, -25.0f}, {0.0f, 1.0f, 0.0f}, {0.0f,  25.0f}},
            {{ 25.0f, -0.5f, -25.0f}, {0.0f, 1.0f, 0.0f}, {25.0f, 25.0f}}
        };
        std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };
        return Mesh(vertices, indices);
    }

    static Mesh CreateSphere(float radius = 1.0f) {
        return CreateHemisphere(radius, 36, 18, true);
    }

    static Mesh CreateHemisphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18, bool fullSphere = false) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        float x, y, z, xy;
        float nx, ny, nz, lengthInv = 1.0f / radius;
        float s, t;
        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = M_PI / stackCount;
        float sectorAngle, stackAngle;
        int loopEnd = fullSphere ? stackCount : stackCount / 2;

        for(int i = 0; i <= loopEnd; ++i) {
            stackAngle = M_PI / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);
            for(int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);
                nx = x * lengthInv; ny = y * lengthInv; nz = z * lengthInv;
                s = (float)j / sectorCount; t = (float)i / stackCount;
                vertices.push_back({{x, z, y}, {nx, nz, ny}, {s, t}});
            }
        }
        int k1, k2;
        for(int i = 0; i < loopEnd; ++i) {
            k1 = i * (sectorCount + 1); k2 = k1 + sectorCount + 1;
            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                if(i != 0) { indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1); }
                if(i != (loopEnd - 1)) { indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1); }
            }
        }
        return Mesh(vertices, indices);
    }

    // --- NOUVEAU : Creer une bol/trou (Inverted Hemisphere) ---
    static Mesh CreateBowl(float radius = 1.0f, int sectorCount = 36, int stackCount = 18) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        float x, y, z, xy;
        float nx, ny, nz, lengthInv = 1.0f / radius;
        float s, t;
        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = (M_PI / 2.0f) / stackCount; // Seulement 90 degrés (demi-sphere)
        float sectorAngle, stackAngle;

        for(int i = 0; i <= stackCount; ++i) {
            stackAngle = M_PI / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle); // Hauteur Z

            for(int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                // --- MODIFICATIONS POUR FAIRE UN TROU ---
                // 1. Inversion de la hauteur (z devient -z pour aller vers le bas)
                // 2. Inversion des Normales (pointent vers l'intérieur)

                // Position : x, -z (vers le bas), y
                glm::vec3 pos(x, -z, y);

                // Normale inversée : pointe vers le centre (0,0,0)
                glm::vec3 norm = glm::normalize(-pos);

                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                vertices.push_back({pos, norm, {s, t}});
            }
        }

        int k1, k2;
        for(int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;
            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                // Inversion de l'ordre des indices pour que la face visible soit l'intérieur (CCW)
                indices.push_back(k1);
                indices.push_back(k1 + 1);
                indices.push_back(k2);

                indices.push_back(k1 + 1);
                indices.push_back(k2 + 1);
                indices.push_back(k2);
            }
        }
        return Mesh(vertices, indices);
    }

    static Mesh CreateTorus(float mainRadius, float tubeRadius, int mainSegments = 30, int tubeSegments = 20) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        for (int i = 0; i <= mainSegments; i++) {
            float theta = i * 2.0f * M_PI / mainSegments;
            float cosTheta = cos(theta);
            float sinTheta = sin(theta);
            for (int j = 0; j <= tubeSegments; j++) {
                float phi = j * 2.0f * M_PI / tubeSegments;
                float cosPhi = cos(phi);
                float sinPhi = sin(phi);
                float x = (mainRadius + tubeRadius * cosPhi) * cosTheta;
                float z = (mainRadius + tubeRadius * cosPhi) * sinTheta;
                float y = tubeRadius * sinPhi;

                glm::vec3 pos(x, y, z);
                glm::vec3 center(mainRadius * cosTheta, 0.0f, mainRadius * sinTheta);
                glm::vec3 norm = glm::normalize(pos - center);

                vertices.push_back({pos, norm, {(float)i/mainSegments, (float)j/tubeSegments}});
            }
        }
        for (int i = 0; i < mainSegments; i++) {
            for (int j = 0; j < tubeSegments; j++) {
                int k1 = i * (tubeSegments + 1) + j;
                int k2 = k1 + tubeSegments + 1;
                indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1);
                indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1);
            }
        }
        return Mesh(vertices, indices);
    }
};
#endif
