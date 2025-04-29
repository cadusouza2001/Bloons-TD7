#ifndef GAME_H
#define GAME_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "Player.h"
#include "Projectile.h"
#include "Building.h"

// Gerenciador principal do jogo
class Game
{
public:
    Game();
    ~Game();

    void init();
    void processInput(GLFWwindow *window, float deltaTime);
    void update(float deltaTime);
    void render();
    void run();

private:
    GLFWwindow *window;
    static const int WIDTH = 800;
    static const int HEIGHT = 600;

    Player player1, player2;
    Projectile projectile;
    std::vector<Building> buildings;

    GLuint backgroundTexture;
    GLuint hitTexture;
    GLuint player1WinsTexture;
    GLuint player2WinsTexture;

    bool player1Turn;
    bool isProjectileMoving;
    bool showTrajectory;
    bool hitActive;
    bool victoryScreenActive;
    float hitTimer;
    float inputAngle;
    float inputForce;
    float lastAngle1, lastForce1;
    float lastAngle2, lastForce2;
    float hitX, hitY;

    bool checkCollisionWithPlayers();
    bool checkCollisionWithBuildings();
    bool isOutOfBounds();
    void setupBuildings();
    void setupPlayers();
    void drawUI();
    void drawTrajectory();
    void resetTurn();
    void endGame();
};

#endif // GAME_H