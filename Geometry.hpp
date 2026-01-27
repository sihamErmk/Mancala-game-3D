#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "Mesh.hpp"
#include <cmath>

class Geometry {
public:
    static Mesh CreateCube() {
        std::vector<Vertex> vertices = {
            // Positions          // Normales           // UVs
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

    static Mesh CreateSphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float x, y, z, xy;
        float nx, ny, nz, lengthInv = 1.0f / radius;
        float s, t; // TexCoords

        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = M_PI / stackCount;
        float sectorAngle, stackAngle;

        for(int i = 0; i <= stackCount; ++i) {
            stackAngle = M_PI / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for(int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;

                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;

                s = (float)j / sectorCount;
                t = (float)i / stackCount;

                vertices.push_back({{x, z, y}, {nx, nz, ny}, {s, t}});
            }
        }

        int k1, k2;
        for(int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;

            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                if(i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if(i != (stackCount - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
        return Mesh(vertices, indices);
    }

    static Mesh CreateHemisphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        float x, y, z, xy;
        float nx, ny, nz, lengthInv = 1.0f / radius;
        float s, t;

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

                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;

                s = (float)j / sectorCount;
                t = (float)i / stackCount;

                vertices.push_back({{x, -z, y}, {nx, -nz, ny}, {s, t}});
            }
        }

        int k1, k2;
        for(int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;
            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
        return Mesh(vertices, indices);
    }
};
#endif
