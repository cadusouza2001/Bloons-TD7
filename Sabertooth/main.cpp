// Jogo Gorillas Base - Thomaz Ritter
// Computacao Grafica 2025/1

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

// Dimensões da tela
const int WIDTH = 800;
const int HEIGHT = 600;

// Constantes físicas
const float GRAVITY = 150.0f;

// Estados do jogo
bool isProjectileMoving = false;
bool player1Turn = true;
bool gameOver = false;

float inputAngle = 45.0f;
float inputForce = 100.0f;
float lastAngle1 = 45.0f;
float lastForce1 = 100.0f;
float lastAngle2 = 45.0f;
float lastForce2 = 100.0f;

// Estruturas para jogadores e projétil
struct Player {
    float x, y;
};

struct Projectile {
    float x, y;
    float velocityX, velocityY;
    float angle;
    float force;
};

struct Building {
    float x, y, width, height;
};

Player player1 = { 100.0f, 100.0f };
Player player2 = { 700.0f, 100.0f };
Projectile projectile;
std::vector<Building> buildings;

// Função para inicializar projétil
void shootProjectile(float angleDeg, float force) {
    if (player1Turn) {
        projectile.x = player1.x;
        projectile.y = player1.y;
        lastAngle1 = inputAngle;
        lastForce1 = inputForce;
    }
    else {
        projectile.x = player2.x;
        projectile.y = player2.y;
        lastAngle2 = inputAngle;
        lastForce2 = inputForce;
    }
    projectile.angle = (player1Turn ? angleDeg : (180 - angleDeg)) * (3.14159265f / 180.0f);
    projectile.force = force;
    projectile.velocityX = cos(projectile.angle) * projectile.force;
    projectile.velocityY = sin(projectile.angle) * projectile.force;
    isProjectileMoving = true;
}

void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i++) {
        float angle = i * 3.14159265f / 180.0f;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawRectangle(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawBar(float x, float y, float width, float height, float percent, float r, float g, float b, float lastPercent) {
    glColor3f(0.5f, 0.5f, 0.5f);
    drawRectangle(x, y, width, height);
    glColor3f(r, g, b);
    drawRectangle(x, y, width * percent, height);

    // Desenhar risco da última seleção
    glColor3f(1.0f, 1.0f, 1.0f);
    float markerX = x + width * lastPercent;
    glBegin(GL_LINES);
    glVertex2f(markerX, y);
    glVertex2f(markerX, y + height);
    glEnd();
}

bool checkCollision(Player player, Projectile proj) {
    float dist = sqrt((player.x - proj.x) * (player.x - proj.x) + (player.y - proj.y) * (player.y - proj.y));
    return dist < 15.0f;
}

bool checkBuildingCollision(Projectile proj) {
    for (auto& building : buildings) {
        if (proj.x >= building.x && proj.x <= (building.x + building.width) &&
            proj.y >= building.y && proj.y <= (building.y + building.height)) {
            return true;
        }
    }
    return false;
}

void setupBuildings() {
    buildings.push_back({ 300, 0, 50, 300 });
    buildings.push_back({ 500, 0, 50, 250 });
}

void setup() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    setupBuildings();
}

void update(float deltaTime) {
    if (isProjectileMoving && !gameOver) {
        projectile.x += projectile.velocityX * deltaTime;
        projectile.y += projectile.velocityY * deltaTime;
        projectile.velocityY -= GRAVITY * deltaTime;

        if (checkCollision(player1Turn ? player2 : player1, projectile)) {
            gameOver = true;
            isProjectileMoving = false;
        }

        if (checkBuildingCollision(projectile)) {
            isProjectileMoving = false;
            player1Turn = !player1Turn;
            inputAngle = 45.0f;
            inputForce = 100.0f;
        }

        if (projectile.y <= 0 || projectile.x < 0 || projectile.x > WIDTH) {
            isProjectileMoving = false;
            player1Turn = !player1Turn;
            inputAngle = 45.0f;
            inputForce = 100.0f;
        }
    }
}

void renderTextSimple(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.6f, 0.6f, 0.6f);
    for (auto& building : buildings) {
        drawRectangle(building.x, building.y, building.width, building.height);
    }

    glColor3f(0.0f, 0.0f, 1.0f);
    drawCircle(player1.x, player1.y, 15);

    glColor3f(1.0f, 0.0f, 0.0f);
    drawCircle(player2.x, player2.y, 15);

    if (isProjectileMoving) {
        glPushMatrix();
        glTranslatef(projectile.x, projectile.y, 0.0f);
        glRotatef(projectile.angle * (180.0f / 3.14159265f), 0.0f, 0.0f, 1.0f);
        glColor3f(1.0f, 1.0f, 0.0f);
        drawCircle(0.0f, 0.0f, 5);
        glPopMatrix();
    }

    float anglePercent = inputAngle / 90.0f;
    if (anglePercent > 1.0f) anglePercent = 1.0f;
    if (anglePercent < 0.0f) anglePercent = 0.0f;

    float lastAnglePercent = (player1Turn ? lastAngle1 : lastAngle2) / 90.0f;
    if (lastAnglePercent > 1.0f) lastAnglePercent = 1.0f;
    if (lastAnglePercent < 0.0f) lastAnglePercent = 0.0f;

    drawBar(10, HEIGHT - 30, 200, 10, anglePercent, 0.0f, 1.0f, 0.0f, lastAnglePercent);

    float forcePercent = inputForce / 200.0f;
    if (forcePercent > 1.0f) forcePercent = 1.0f;
    if (forcePercent < 0.0f) forcePercent = 0.0f;

    float lastForcePercent = (player1Turn ? lastForce1 : lastForce2) / 200.0f;
    if (lastForcePercent > 1.0f) lastForcePercent = 1.0f;
    if (lastForcePercent < 0.0f) lastForcePercent = 0.0f;

    drawBar(10, HEIGHT - 50, 200, 10, forcePercent, 1.0f, 0.5f, 0.0f, lastForcePercent);

    renderTextSimple(220, HEIGHT - 27, "Angulo");
    renderTextSimple(220, HEIGHT - 47, "Forca");
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Erro ao iniciar GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Gorillas Game Base", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    setup();

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!isProjectileMoving && !gameOver) {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) inputAngle += 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) inputAngle -= 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) inputForce += 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) inputForce -= 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                shootProjectile(inputAngle, inputForce * 2.5f);
            }
        }

        update(deltaTime);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}