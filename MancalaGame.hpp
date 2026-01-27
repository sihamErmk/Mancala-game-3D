#ifndef MANCALAGAME_HPP
#define MANCALAGAME_HPP

#include <vector>
#include <queue>
#include <glm/glm.hpp>
#include <iostream>
#include <string>

// --- STRUCTURES ---
struct Pit {
    int id;
    int seeds;
    glm::vec3 position;
    float radius;
    bool isSelected;
    bool isHovered;
    bool isHidden;
    bool isActive;
};

enum GameState {
    IDLE,
    ANIMATING
};

struct MovingSeed {
    glm::vec3 startPos;
    glm::vec3 endPos;
    glm::vec3 currentPos;
    float progress;
    int targetPitIndex;
    int colorIdx; // Pour garder la couleur de la graine qui vole
};

class MancalaGame {
public:
    std::vector<Pit> pits;
    GameState state;

    // --- ETAT DU JEU ---
    int currentPlayer; // 0 = Bas, 1 = Haut
    bool gameOver;
    std::string statusMessage;

    // NOUVEAU : Signal pour dire au main.cpp de tourner la caméra
    bool turnChanged;

    // --- ANIMATION ---
    std::vector<int> pathQueue;
    MovingSeed activeSeed;
    int seedsInHand;
    float moveSpeed;

    MancalaGame() {
        InitBoard();
    }

    void InitBoard() {
        pits.clear();
        state = IDLE;
        currentPlayer = 0;
        gameOver = false;
        turnChanged = true; // On force la caméra à se placer au début
        moveSpeed = 5.0f;
        statusMessage = "Tour du Joueur 1";

        // J1 (Bas)
        for(int i = 0; i < 6; i++) {
            Pit p; p.id = i; p.seeds = 4;
            p.position = glm::vec3((i - 2.5f) * 2.2f, 0.0f, 1.8f);
            p.radius = 0.8f; p.isSelected = false; p.isHovered = false; p.isHidden = false; p.isActive = true;
            pits.push_back(p);
        }

        // Magasin J1
        Pit store1; store1.id = 6; store1.seeds = 0;
        store1.position = glm::vec3(7.5f, 0.0f, 0.0f);
        store1.radius = 1.3f; store1.isSelected = false; store1.isHovered = false; store1.isHidden = false; store1.isActive = false;
        pits.push_back(store1);

        // J2 (Haut)
        for(int i = 0; i < 6; i++) {
            Pit p; p.id = 7 + i; p.seeds = 4;
            p.position = glm::vec3((2.5f - i) * 2.2f, 0.0f, -1.8f);
            p.radius = 0.8f; p.isSelected = false; p.isHovered = false; p.isHidden = false; p.isActive = false;
            pits.push_back(p);
        }

        // Magasin J2
        Pit store2; store2.id = 13; store2.seeds = 0;
        store2.position = glm::vec3(-7.5f, 0.0f, 0.0f);
        store2.radius = 1.3f; store2.isSelected = false; store2.isHovered = false; store2.isHidden = false; store2.isActive = false;
        pits.push_back(store2);

        UpdateActivePits();
    }

    void UpdateActivePits() {
        for(auto& p : pits) {
            p.isActive = false;
            if (currentPlayer == 0 && p.id >= 0 && p.id <= 5 && p.seeds > 0) p.isActive = true;
            if (currentPlayer == 1 && p.id >= 7 && p.id <= 12 && p.seeds > 0) p.isActive = true;
        }
    }

    void Update(float deltaTime) {
        if (state == ANIMATING) {
            activeSeed.progress += deltaTime * moveSpeed;
            // Arc parabolique (saute haut)
            float height = sin(activeSeed.progress * 3.14159f) * 3.5f;
            activeSeed.currentPos = glm::mix(activeSeed.startPos, activeSeed.endPos, activeSeed.progress);
            activeSeed.currentPos.y += height;

            if (activeSeed.progress >= 1.0f) {
                int targetIdx = activeSeed.targetPitIndex;
                pits[targetIdx].seeds++;

                if (!pathQueue.empty()) {
                    StartNextSeedAnimation();
                } else {
                    OnMoveFinished(targetIdx);
                }
            }
        }
    }

    void StartNextSeedAnimation() {
        if (pathQueue.empty()) return;
        int nextPitId = pathQueue.front();
        pathQueue.erase(pathQueue.begin());
        activeSeed.startPos = activeSeed.endPos;
        activeSeed.targetPitIndex = nextPitId;
        activeSeed.endPos = pits[nextPitId].position;
        activeSeed.progress = 0.0f;
        activeSeed.colorIdx = rand() % 4; // Change couleur pour effet visuel
    }

    int ProcessClick(glm::vec3 rayOrigin, glm::vec3 rayDir, bool isEditMode) {
        if (!isEditMode && state != IDLE) return -1;
        if (gameOver && !isEditMode) return -1;

        int clickedID = -1;
        float minDistance = 1000.0f;

        for(auto& pit : pits) {
            if (pit.isHidden) continue;
            if (!isEditMode && !pit.isActive) continue;

            glm::vec3 oc = pit.position - rayOrigin;
            float t = glm::dot(oc, rayDir);
            if (t < 0) continue;
            glm::vec3 pOnRay = rayOrigin + rayDir * t;
            if (glm::length(pit.position - pOnRay) < pit.radius) {
                float distCam = glm::length(pit.position - rayOrigin);
                if (distCam < minDistance) { minDistance = distCam; clickedID = pit.id; }
            }
        }

        if (clickedID != -1) {
            if (isEditMode) {
                for(auto& p : pits) p.isSelected = false;
                pits[clickedID].isSelected = true;
            } else {
                TryPlayMove(clickedID);
            }
        }
        return clickedID;
    }

    void TryPlayMove(int pitIndex) {
        if (currentPlayer == 0 && (pitIndex < 0 || pitIndex > 5)) return;
        if (currentPlayer == 1 && (pitIndex < 7 || pitIndex > 12)) return;

        seedsInHand = pits[pitIndex].seeds;
        pits[pitIndex].seeds = 0;

        pathQueue.clear();
        int currentIndex = pitIndex;

        for (int i = 0; i < seedsInHand; i++) {
            currentIndex = (currentIndex + 1) % 14;
            if (currentPlayer == 0 && currentIndex == 13) currentIndex = 0;
            else if (currentPlayer == 1 && currentIndex == 6) currentIndex = 7;

            pathQueue.push_back(currentIndex);
        }

        state = ANIMATING;
        statusMessage = "Distribution...";

        activeSeed.startPos = pits[pitIndex].position;
        int firstTarget = pathQueue.front();
        pathQueue.erase(pathQueue.begin());
        activeSeed.targetPitIndex = firstTarget;
        activeSeed.endPos = pits[firstTarget].position;
        activeSeed.progress = 0.0f;
        activeSeed.colorIdx = rand() % 4;

        for(auto& p : pits) p.isActive = false;
    }

    void OnMoveFinished(int lastPitIdx) {
        state = IDLE;
        bool switchTurn = true;
        turnChanged = false; // Reset

        // REJOUER
        if (currentPlayer == 0 && lastPitIdx == 6) {
            statusMessage = "JOUEUR 1 REJOUE !";
            switchTurn = false;
        } else if (currentPlayer == 1 && lastPitIdx == 13) {
            statusMessage = "JOUEUR 2 REJOUE !";
            switchTurn = false;
        }

        // CAPTURE
        else if (pits[lastPitIdx].seeds == 1) {
            bool isMySide = false;
            if (currentPlayer == 0 && lastPitIdx >= 0 && lastPitIdx <= 5) isMySide = true;
            if (currentPlayer == 1 && lastPitIdx >= 7 && lastPitIdx <= 12) isMySide = true;

            if (isMySide) {
                int oppositeIdx = 12 - lastPitIdx;
                if (pits[oppositeIdx].seeds > 0) {
                    int captured = pits[oppositeIdx].seeds + 1;
                    pits[oppositeIdx].seeds = 0;
                    pits[lastPitIdx].seeds = 0;
                    int myStore = (currentPlayer == 0) ? 6 : 13;
                    pits[myStore].seeds += captured;
                    statusMessage = "CAPTURE !";
                }
            }
        }

        CheckGameOver();

        if (!gameOver) {
            if (switchTurn) {
                currentPlayer = 1 - currentPlayer;
                statusMessage = (currentPlayer == 0) ? "Tour du Joueur 1" : "Tour du Joueur 2";
                // IMPORTANT: C'est ici qu'on déclenche l'animation
                turnChanged = true;
            }
            UpdateActivePits();
        }
    }

    void CheckGameOver() {
        bool p1Empty = true;
        bool p2Empty = true;
        for(int i=0; i<6; i++) if(pits[i].seeds > 0) p1Empty = false;
        for(int i=7; i<12; i++) if(pits[i].seeds > 0) p2Empty = false;

        if (p1Empty || p2Empty) {
            gameOver = true;
            for(int i=0; i<6; i++) { pits[6].seeds += pits[i].seeds; pits[i].seeds = 0; }
            for(int i=7; i<12; i++) { pits[13].seeds += pits[i].seeds; pits[i].seeds = 0; }

            if (pits[6].seeds > pits[13].seeds) statusMessage = "VICTOIRE JOUEUR 1 !";
            else if (pits[13].seeds > pits[6].seeds) statusMessage = "VICTOIRE JOUEUR 2 !";
            else statusMessage = "EGALITE !";
        }
    }

    void UpdateHover(glm::vec3 rayOrigin, glm::vec3 rayDir, bool isEditMode) {
        for(auto& pit : pits) pit.isHovered = false;
        if (state == ANIMATING && !isEditMode) return;

        float minDistance = 1000.0f;
        int hoverID = -1;
        for(auto& pit : pits) {
            if (pit.isHidden) continue;
            if (!isEditMode && !pit.isActive) continue;

            glm::vec3 oc = pit.position - rayOrigin;
            float t = glm::dot(oc, rayDir);
            if (t < 0) continue;
            glm::vec3 pOnRay = rayOrigin + rayDir * t;
            if (glm::length(pit.position - pOnRay) < pit.radius) {
                float distCam = glm::length(pit.position - rayOrigin);
                if (distCam < minDistance) { minDistance = distCam; hoverID = pit.id; }
            }
        }
        if(hoverID != -1) pits[hoverID].isHovered = true;
    }
};
#endif
