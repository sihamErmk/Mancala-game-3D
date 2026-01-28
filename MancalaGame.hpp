#ifndef MANCALAGAME_HPP
#define MANCALAGAME_HPP

#include <vector>
#include <queue>
#include <glm/glm.hpp>
#include <iostream>
#include <string>

// Structure représentant un trou (fosse) ou un magasin
struct Pit {
    int id;             // Index (0-13)
    int seeds;          // Nombre de graines
    glm::vec3 position; // Position 3D
    float radius;       // Rayon pour le clic
    bool isSelected;    // État sélectionné
    bool isHovered;     // État survolé
    bool isHidden;      // (Non utilisé, mais présent)
    bool isActive;      // C'est le tour de ce trou ?
    float depth;        // (Non utilisé ici)
};

enum GameState {
    IDLE,       // En attente d'un clic
    ANIMATING   // Une graine est en train de bouger
};

struct MovingSeed {
    glm::vec3 startPos;
    glm::vec3 endPos;
    glm::vec3 currentPos;
    float progress;
    int targetPitIndex;
};

class MancalaGame {
public:
    std::vector<Pit> pits;
    GameState state;
    int currentPlayer;      // 0 = Bas, 1 = Haut
    bool gameOver;
    std::string statusMessage;
    std::vector<int> pathQueue; // Liste des prochains trous à visiter
    MovingSeed activeSeed;
    int seedsInHand;        // Graines en main pour la distribution
    float moveSpeed;

    MancalaGame() {
        InitBoard();
    }

    // Initialisation du plateau (4 graines partout, magasins vides)
    void InitBoard() {
        pits.clear();
        state = IDLE;
        currentPlayer = 0;
        gameOver = false;
        moveSpeed = 3.5f; // Vitesse de l'animation
        statusMessage = "Jeu pret. Tour du Joueur 1 (Bas)";

        // --- JOUEUR 1 (Bas) : Trous 0 à 5 ---
        for(int i = 0; i < 6; i++) {
            Pit p; p.id = i; p.seeds = 4;
            p.position = glm::vec3((i - 2.5f) * 2.2f, 0.0f, 1.8f);
            p.radius = 0.8f; p.isSelected = false; p.isHovered = false; p.isHidden = false; p.isActive = true;
            pits.push_back(p);
        }

        // --- MAGASIN J1 (Droite) : Trou 6 ---
        Pit store1; store1.id = 6; store1.seeds = 0;
        store1.position = glm::vec3(7.5f, 0.0f, 0.0f);
        store1.radius = 1.3f; store1.isSelected = false; store1.isHovered = false; store1.isHidden = false; store1.isActive = false;
        pits.push_back(store1);

        // --- JOUEUR 2 (Haut) : Trous 7 à 12 ---
        for(int i = 0; i < 6; i++) {
            Pit p; p.id = 7 + i; p.seeds = 4;
            // Position inversée pour être en face
            p.position = glm::vec3((2.5f - i) * 2.2f, 0.0f, -1.8f);
            p.radius = 0.8f; p.isSelected = false; p.isHovered = false; p.isHidden = false; p.isActive = false;
            pits.push_back(p);
        }

        // --- MAGASIN J2 (Gauche) : Trou 13 ---
        Pit store2; store2.id = 13; store2.seeds = 0;
        store2.position = glm::vec3(-7.5f, 0.0f, 0.0f);
        store2.radius = 1.3f; store2.isSelected = false; store2.isHovered = false; store2.isHidden = false; store2.isActive = false;
        pits.push_back(store2);

        UpdateActivePits();
        PrintGameState();
    }

    void PrintGameState() {
        std::cout << "J1: " << pits[6].seeds << " | J2: " << pits[13].seeds << std::endl;
    }

    // Active/Désactive la surbrillance des trous selon le tour
    void UpdateActivePits() {
        for(auto& p : pits) {
            p.isActive = false;
            // Joueur 1 joue sur 0-5
            if (currentPlayer == 0 && p.id >= 0 && p.id <= 5 && p.seeds > 0) p.isActive = true;
            // Joueur 2 joue sur 7-12
            if (currentPlayer == 1 && p.id >= 7 && p.id <= 12 && p.seeds > 0) p.isActive = true;
        }
    }

    // Boucle de mise à jour (Animation)
    void Update(float deltaTime) {
        if (state == ANIMATING) {
            activeSeed.progress += deltaTime * moveSpeed;
            // Courbe parabolique pour le saut
            float height = sin(activeSeed.progress * 3.14159f) * 2.5f;
            activeSeed.currentPos = glm::mix(activeSeed.startPos, activeSeed.endPos, activeSeed.progress);
            activeSeed.currentPos.y += height;

            // Fin du mouvement d'une graine
            if (activeSeed.progress >= 1.0f) {
                int targetIdx = activeSeed.targetPitIndex;
                pits[targetIdx].seeds++; // Ajouter la graine au trou cible

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
        activeSeed.startPos = activeSeed.endPos; // Part de la où la précédente s'est arrêtée (visuellement fluide)
        activeSeed.targetPitIndex = nextPitId;
        activeSeed.endPos = pits[nextPitId].position;
        activeSeed.progress = 0.0f;
    }

    // Gestion du clic souris (Raycasting)
    int ProcessClick(glm::vec3 rayOrigin, glm::vec3 rayDir, bool isEditMode) {
        if (!isEditMode && state != IDLE) return -1;
        if (gameOver && !isEditMode) return -1;

        int clickedID = -1;
        float minDistance = 1000.0f;

        for(auto& pit : pits) {
            if (pit.isHidden) continue;
            // En jeu, on ne peut cliquer que sur ses propres trous non vides
            if (!isEditMode && !pit.isActive) continue;

            // Intersection Rayon/Sphère simple
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

    // Initialise le coup
    void TryPlayMove(int pitIndex) {
        // Sécurité : Vérifier que le joueur joue du bon côté
        if (currentPlayer == 0 && (pitIndex < 0 || pitIndex > 5)) return;
        if (currentPlayer == 1 && (pitIndex < 7 || pitIndex > 12)) return;

        seedsInHand = pits[pitIndex].seeds;
        pits[pitIndex].seeds = 0; // On vide le trou cliqué

        // Calculer le chemin
        pathQueue.clear();
        int currentIndex = pitIndex;

        for (int i = 0; i < seedsInHand; i++) {
            currentIndex = (currentIndex + 1) % 14;

            // Sauter le magasin adverse
            if (currentPlayer == 0 && currentIndex == 13) currentIndex = 0;
            else if (currentPlayer == 1 && currentIndex == 6) currentIndex = 7;

            pathQueue.push_back(currentIndex);
        }

        // Démarrer l'animation
        state = ANIMATING;
        statusMessage = "Distribution...";

        activeSeed.startPos = pits[pitIndex].position;
        int firstTarget = pathQueue.front();
        pathQueue.erase(pathQueue.begin());
        activeSeed.targetPitIndex = firstTarget;
        activeSeed.endPos = pits[firstTarget].position;
        activeSeed.progress = 0.0f;

        // Désactiver les clics pendant l'anim
        for(auto& p : pits) p.isActive = false;
    }

    // Appelé quand la dernière graine tombe
    void OnMoveFinished(int lastPitIdx) {
        state = IDLE;
        bool switchTurn = true;

        // REGLE : REJOUER si on finit dans son magasin
        if (currentPlayer == 0 && lastPitIdx == 6) {
            statusMessage = ">>> JOUEUR 1 REJOUE ! (Derniere au magasin)";
            switchTurn = false;
        } else if (currentPlayer == 1 && lastPitIdx == 13) {
            statusMessage = ">>> JOUEUR 2 REJOUE ! (Derniere au magasin)";
            switchTurn = false;
        }
        // REGLE : CAPTURE si on finit dans un trou vide de son côté
        else if (pits[lastPitIdx].seeds == 1) { // == 1 car on vient de la poser
            bool isMySide = false;
            if (currentPlayer == 0 && lastPitIdx >= 0 && lastPitIdx <= 5) isMySide = true;
            if (currentPlayer == 1 && lastPitIdx >= 7 && lastPitIdx <= 12) isMySide = true;

            if (isMySide) {
                int oppositeIdx = 12 - lastPitIdx; // Formule symétrique
                if (pits[oppositeIdx].seeds > 0) {
                    int captured = pits[oppositeIdx].seeds + 1; // +1 pour celle qu'on vient de poser
                    pits[oppositeIdx].seeds = 0;
                    pits[lastPitIdx].seeds = 0;

                    int myStore = (currentPlayer == 0) ? 6 : 13;
                    pits[myStore].seeds += captured;

                    statusMessage = "CAPTURE ! + " + std::to_string(captured);
                }
            }
        }

        // Vérifier si la partie est finie
        CheckGameOver();

        if (!gameOver) {
            if (switchTurn) {
                currentPlayer = 1 - currentPlayer;
                statusMessage = (currentPlayer == 0) ? "Tour du Joueur 1 (Bas)" : "Tour du Joueur 2 (Haut)";
            }
            UpdateActivePits();
            PrintGameState();
        }
    }

    // --- C'EST ICI QUE SE JOUE LA FIN DE PARTIE ---
    void CheckGameOver() {
        bool p1Empty = true;
        bool p2Empty = true;

        // Vérification J1 (Indices 0 à 5)
        for(int i=0; i<6; i++) {
            if(pits[i].seeds > 0) p1Empty = false;
        }

        // Vérification J2 (Indices 7 à 12)
        // ATTENTION : i <= 12 est crucial pour inclure le dernier trou
        for(int i=7; i<=12; i++) {
            if(pits[i].seeds > 0) p2Empty = false;
        }

        if (p1Empty || p2Empty) {
            gameOver = true;

            // --- NETTOYAGE DU PLATEAU (UPDATE VISUEL) ---

            // 1. Tout ce qui reste chez J1 va dans le magasin J1
            for(int i=0; i<6; i++) {
                pits[6].seeds += pits[i].seeds;
                pits[i].seeds = 0; // IMPORTANT : Met à 0 pour que le rendu 3D ne les affiche plus
            }

            // 2. Tout ce qui reste chez J2 va dans le magasin J2
            for(int i=7; i<=12; i++) {
                pits[13].seeds += pits[i].seeds;
                pits[i].seeds = 0; // IMPORTANT : Met à 0 pour le rendu 3D
            }

            // --- VAINQUEUR ---
            std::string winner;
            if (pits[6].seeds > pits[13].seeds) {
                winner = "VICTOIRE JOUEUR 1 !";
            }
            else if (pits[13].seeds > pits[6].seeds) {
                winner = "VICTOIRE JOUEUR 2 !";
            }
            else {
                winner = "EGALITE !";
            }

            // Mise à jour du message final
            statusMessage = "FIN : " + winner + " [J1:" + std::to_string(pits[6].seeds) + " - J2:" + std::to_string(pits[13].seeds) + "]";

            // Désactiver toutes les lumières d'interaction
            for(auto& p : pits) p.isActive = false;
        }
    }

    void UpdateHover(glm::vec3 rayOrigin, glm::vec3 rayDir, bool isEditMode) {
        for (auto& pit : pits) pit.isHovered = false;

        if (state == ANIMATING && !isEditMode) return;

        float minDistance = 1000.0f;
        int hoverID = -1;

        for (auto& pit : pits) {
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

        if (hoverID != -1) {
            pits[hoverID].isHovered = true;
        }
    }
};
#endif
