#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "Mesh.hpp"
#include <cmath>

class Geometry {
public:
    static Mesh CreateCube() {
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
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

    static Mesh CreateBowl(float radius = 1.0f, int sectorCount = 36, int stackCount = 18) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        float x, y, z, xy;
        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = (M_PI / 2.0f) / stackCount;
        float sectorAngle, stackAngle;

        for(int i = 0; i <= stackCount; ++i) {
            stackAngle = M_PI / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for(int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                glm::vec3 pos(x, -z, y);
                glm::vec3 norm = glm::normalize(-pos);
                float s = (float)j / sectorCount;
                float t = (float)i / stackCount;
                vertices.push_back({pos, norm, {s, t}});
            }
        }

        int k1, k2;
        for(int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;
            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
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
};
#endif
