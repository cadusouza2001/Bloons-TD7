#include "Game.h"
#include <glm/glm.hpp> // para radians()
#include "utils.h"
#include <iostream>
#include <ctime>
#include <cmath>

#define GRAVITY 150.0f

Game::Game()
    : window(nullptr),
      player1Turn(true), isProjectileMoving(false),
      showTrajectory(false), hitActive(false), victoryScreenActive(false),
      hitTimer(0.0f), inputAngle(45.0f), inputForce(100.0f),
      lastAngle1(45.0f), lastForce1(100.0f),
      lastAngle2(45.0f), lastForce2(100.0f),
      hitX(0.0f), hitY(0.0f)
{
}

Game::~Game()
{
    glfwTerminate();
}

void Game::init()
{
    srand(static_cast<unsigned int>(time(0)));

    if (!glfwInit())
    {
        std::cerr << "Erro ao iniciar GLFW" << std::endl;
        exit(1);
    }

    window = glfwCreateWindow(WIDTH, HEIGHT, "Bloons TD7", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "Erro ao criar janela" << std::endl;
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glewInit();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    backgroundTexture = loadTexture("images/background.png");
    hitTexture = loadTexture("images/hit.png");
    player1WinsTexture = loadTexture("images/player1wins.png");
    player2WinsTexture = loadTexture("images/player2wins.png");

    player1.setup("images/player1.png", "images/player12.png");
    player2.setup("images/player2.png", "images/player22.png");
    projectile.setup("images/projectile.png");

    setupBuildings();
    setupPlayers();
}

void Game::setupBuildings()
{
    buildings.clear();
    int spacing = 10;
    int safeZone = 60;

    for (int x = 0; x < WIDTH; x += 50 + spacing)
    {
        Building b;
        float height = 50 + rand() % (HEIGHT / 2);
        b.setup(x, 0, 50, height, "images/building.png");
        buildings.push_back(b);
    }
}

void Game::setupPlayers()
{
    std::vector<Building *> leftBuildings;
    std::vector<Building *> rightBuildings;

    for (auto &building : buildings)
    {
        if (building.x < WIDTH / 2)
            leftBuildings.push_back(&building);
        else
            rightBuildings.push_back(&building);
    }

    float playerHeight = HEIGHT * 0.15f;

    if (!leftBuildings.empty())
    {
        Building *selected = leftBuildings[rand() % leftBuildings.size()];
        player1.x = selected->x + selected->width / 2.0f;
        player1.y = selected->height + (playerHeight * 0.4f);
    }

    if (!rightBuildings.empty())
    {
        Building *selected = rightBuildings[rand() % rightBuildings.size()];
        player2.x = selected->x + selected->width / 2.0f;
        player2.y = selected->height + (playerHeight * 0.4f);
    }
}

void Game::processInput(GLFWwindow *window, float deltaTime)
{
    static bool fKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        inputAngle += 30.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        inputAngle -= 30.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        inputForce += 30.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        inputForce -= 30.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isProjectileMoving && !hitActive && !victoryScreenActive)
    {
        float angle = player1Turn ? inputAngle : (180.0f - inputAngle);
        Player &shooter = player1Turn ? player1 : player2;
        shooter.startAttack();
        projectile.shoot(shooter.x, shooter.y, angle, inputForce * 2.5f);
        if (player1Turn)
        {
            lastAngle1 = inputAngle;
            lastForce1 = inputForce;
        }
        else
        {
            lastAngle2 = inputAngle;
            lastForce2 = inputForce;
        }
        isProjectileMoving = true;
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!fKeyPressed)
        {
            showTrajectory = !showTrajectory;
            fKeyPressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    {
        fKeyPressed = false;
    }
}

void Game::update(float deltaTime)
{
    player1.update(deltaTime);
    player2.update(deltaTime);

    if (isProjectileMoving)
    {
        projectile.update(deltaTime);

        if (projectile.x < 0 || projectile.x > WIDTH || projectile.y < 0)
        {
            isProjectileMoving = false;
            resetTurn();
        }

        float distP1 = sqrt(pow(projectile.x - player1.x, 2) + pow(projectile.y - player1.y, 2));
        float distP2 = sqrt(pow(projectile.x - player2.x, 2) + pow(projectile.y - player2.y, 2));

        if ((player1Turn && distP2 < 30) || (!player1Turn && distP1 < 30))
        {
            isProjectileMoving = false;
            hitActive = true;
            hitTimer = 1.0f;
            hitX = projectile.x;
            hitY = projectile.y;
            victoryScreenActive = true;
        }

        for (auto &building : buildings)
        {
            if (projectile.x >= building.x && projectile.x <= building.x + building.width &&
                projectile.y >= building.y && projectile.y <= building.y + building.height)
            {
                isProjectileMoving = false;
                hitActive = true;
                hitTimer = 1.0f;
                hitX = projectile.x;
                hitY = projectile.y;
            }
        }
    }
    else if (hitActive)
    {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f)
        {
            hitActive = false;
            if (!victoryScreenActive)
                resetTurn();
        }
    }
}

void Game::render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    drawTexturedRectangle(0, 0, WIDTH, HEIGHT, backgroundTexture);

    for (auto &building : buildings)
        building.draw();

    player1.draw();
    player2.draw();

    if (isProjectileMoving)
        projectile.draw();

    if (hitActive)
    {
        float size = HEIGHT * 0.1f;
        drawTexturedRectangle(hitX - size / 2, hitY - size / 2, size, size, hitTexture);
    }

    if (victoryScreenActive)
    {
        GLuint winnerTexture = player1Turn ? player1WinsTexture : player2WinsTexture;
        drawTexturedRectangle(0, 0, WIDTH, HEIGHT, winnerTexture);
    }
    else
    {
        drawUI();
    }

    if (showTrajectory && !isProjectileMoving && !hitActive && !victoryScreenActive)
        drawTrajectory();
}

void Game::drawUI()
{
    float anglePercent = inputAngle / 90.0f;
    if (anglePercent > 1.0f)
        anglePercent = 1.0f;
    if (anglePercent < 0.0f)
        anglePercent = 0.0f;

    float lastAnglePercent = (player1Turn ? lastAngle1 : lastAngle2) / 90.0f;
    if (lastAnglePercent > 1.0f)
        lastAnglePercent = 1.0f;
    if (lastAnglePercent < 0.0f)
        lastAnglePercent = 0.0f;

    drawBar(10, HEIGHT - 30, 200, 10, anglePercent, 0.0f, 1.0f, 0.0f, lastAnglePercent);

    float forcePercent = inputForce / 200.0f;
    if (forcePercent > 1.0f)
        forcePercent = 1.0f;
    if (forcePercent < 0.0f)
        forcePercent = 0.0f;

    float lastForcePercent = (player1Turn ? lastForce1 : lastForce2) / 200.0f;
    if (lastForcePercent > 1.0f)
        lastForcePercent = 1.0f;
    if (lastForcePercent < 0.0f)
        lastForcePercent = 0.0f;

    drawBar(10, HEIGHT - 50, 200, 10, forcePercent, 1.0f, 0.5f, 0.0f, lastForcePercent);

    renderTextSimple(220, HEIGHT - 27, "Angulo");
    renderTextSimple(220, HEIGHT - 47, "Forca");
}

void Game::drawTrajectory()
{
    if (!showTrajectory)
        return;

    float posX, posY;
    float velX, velY;

    Player &shooter = player1Turn ? player1 : player2;
    posX = shooter.x;
    posY = shooter.y;

    float angleRad = glm::radians(player1Turn ? inputAngle : (180.0f - inputAngle));
    float forceVal = inputForce * 2.5f;
    velX = cos(angleRad) * forceVal;
    velY = sin(angleRad) * forceVal;

    glColor3f(1.0f, 1.0f, 0.0f); // cor amarela da trajet�ria
    glBegin(GL_LINE_STRIP);

    const float deltaT = 0.1f; // quanto tempo avan�a a cada passo
    float t = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        float x = posX + velX * t;
        float y = posY + velY * t - 0.5f * GRAVITY * t * t;

        if (y < 0)
            break; // se cair no ch�o, para

        glVertex2f(x, y);

        t += deltaT;
    }

    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f); // resetar cor para branco
}

void Game::resetTurn()
{
    player1Turn = !player1Turn;
    inputAngle = 45.0f;
    inputForce = 100.0f;
}

void Game::run()
{
    init();

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        processInput(window, deltaTime);
        update(deltaTime);
        render();
        glfwSwapBuffers(window);
    }
}
