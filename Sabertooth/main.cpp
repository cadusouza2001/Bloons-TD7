// Jogo Gorillas Base - Thomaz Ritter
// Computacao Grafica 2025/1

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint buildingTexture;
GLuint player1Texture;
GLuint player2Texture;

void loadTextureAndMask(const char* filename, GLuint& textureID, std::vector<unsigned char>& mask, int& texWidth, int& texHeight) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texWidth = width;
        texHeight = height;

        if (nrChannels == 4) {
            mask.resize(width * height);
            for (int i = 0; i < width * height; ++i) {
                mask[i] = data[i * 4 + 3];  // salva o canal alpha
            }
        }

        stbi_image_free(data);
    }
    else {
        std::cout << "Falha ao carregar " << filename << std::endl;
    }
}




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
struct Player
{
    float x, y;
    GLuint textureID;
    int texWidth, texHeight;
    std::vector<unsigned char> alphaMask; // novo campo
};

struct Building
{
    float x, y, width, height;
    GLuint textureID;
    int texWidth, texHeight;
    std::vector<unsigned char> alphaMask; // novo campo
};

struct Projectile
{
    float x, y;
    float velocityX, velocityY;
    float angle;
    float force;
};

Player player1 = {100.0f, 100.0f};
Player player2 = {700.0f, 100.0f};
Projectile projectile;
std::vector<Building> buildings;


// Função para inicializar projétil
void shootProjectile(float angleDeg, float force)
{
    if (player1Turn)
    {
        projectile.x = player1.x;
        projectile.y = player1.y;
        lastAngle1 = inputAngle;
        lastForce1 = inputForce;
    }
    else
    {
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

void drawCircle(float cx, float cy, float r)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i++)
    {
        float angle = i * 3.14159265f / 180.0f;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawRectangle(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawBar(float x, float y, float width, float height, float percent, float r, float g, float b, float lastPercent)
{
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

bool checkCollision(Player player, Projectile proj)
{
    float dist = sqrt((player.x - proj.x) * (player.x - proj.x) + (player.y - proj.y) * (player.y - proj.y));
    return dist < 15.0f;
}

bool checkPixelCollision(Player& player, Projectile& proj) {
    int localX = (int)((proj.x - (player.x - 15)) / (30.0f) * player.texWidth);
    int localY = (int)((proj.y - (player.y - 15)) / (30.0f) * player.texHeight);

    if (localX >= 0 && localX < player.texWidth && localY >= 0 && localY < player.texHeight) {
        int idx = (localY * player.texWidth) + localX;
        return player.alphaMask[idx] > 128;  // se alpha significativo
    }
    return false;
}


bool checkBuildingCollision(Projectile proj)
{
    for (auto& building : buildings)
    {
        if (proj.x >= building.x && proj.x <= (building.x + building.width) &&
            proj.y >= building.y && proj.y <= (building.y + building.height))
        {
            // Está dentro do retângulo geral — agora checar alpha!
            int localX = (int)(((proj.x - building.x) / building.width) * building.texWidth);
            int localY = (int)(((proj.y - building.y) / building.height) * building.texHeight);

            if (localX >= 0 && localX < building.texWidth && localY >= 0 && localY < building.texHeight)
            {
                int idx = (localY * building.texWidth) + localX;
                return building.alphaMask[idx] > 128;  // se o alpha é significativo
            }
        }
    }
    return false;
}



void setupBuildings() {
    buildings.clear();
    int buildingWidth = 50;
    int spacing = 10;
    int safeZone = 60; // Nova distância mínima de segurança dos jogadores

    // Gera prédios apenas entre player1 + safeZone e player2 - safeZone
    for (int x = player1.x + safeZone; x < player2.x - safeZone - buildingWidth; x += buildingWidth + spacing) {
        Building b;
        b.x = x;
        b.width = buildingWidth;
        b.height = 100 + rand() % 200;  // altura aleatória
        b.y = 0;

        loadTextureAndMask("images/building.png", b.textureID, b.alphaMask, b.texWidth, b.texHeight);

        buildings.push_back(b);
    }
}



void setup()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    setupBuildings();

    loadTextureAndMask("images/player1.png", player1.textureID, player1.alphaMask, player1.texWidth, player1.texHeight);
    loadTextureAndMask("images/player2.png", player2.textureID, player2.alphaMask, player2.texWidth, player2.texHeight);

}

void drawTexturedRectangle(float x, float y, float width, float height, GLuint textureID)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y);             // baixo esquerdo
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y);      // baixo direito
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y + height); // topo direito
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y + height);     // topo esquerdo
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


void drawTexturedCircle(float cx, float cy, float r, GLuint textureID)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f);
    glVertex2f(cx, cy); // centro
    for (int i = 0; i <= 360; i++)
    {
        float angle = i * 3.14159265f / 180.0f;
        float x = cx + cos(angle) * r;
        float y = cy + sin(angle) * r;
        glTexCoord2f(0.5f + cos(angle) * 0.5f, 0.5f + sin(angle) * 0.5f);
        glVertex2f(x, y);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void update(float deltaTime)
{
    if (isProjectileMoving && !gameOver)
    {
        projectile.x += projectile.velocityX * deltaTime;
        projectile.y += projectile.velocityY * deltaTime;
        projectile.velocityY -= GRAVITY * deltaTime;

        if (checkCollision(player1Turn ? player2 : player1, projectile))
        {
            gameOver = true;
            isProjectileMoving = false;
        }

        if (checkBuildingCollision(projectile))
        {
            isProjectileMoving = false;
            player1Turn = !player1Turn;
            inputAngle = 45.0f;
            inputForce = 100.0f;
        }

        if (projectile.y <= 0 || projectile.x < 0 || projectile.x > WIDTH)
        {
            isProjectileMoving = false;
            player1Turn = !player1Turn;
            inputAngle = 45.0f;
            inputForce = 100.0f;
        }
    }
}

void renderTextSimple(float x, float y, const std::string &text)
{
    glRasterPos2f(x, y);
    for (char c : text)
    {
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto& building : buildings) {
        drawTexturedRectangle(building.x, building.y, building.width, building.height, building.textureID);
    }


    drawTexturedCircle(player1.x, player1.y, 15, player1Texture);
    drawTexturedCircle(player2.x, player2.y, 15, player2Texture);

    if (isProjectileMoving)
    {
        glPushMatrix();
        glTranslatef(projectile.x, projectile.y, 0.0f);
        glRotatef(projectile.angle * (180.0f / 3.14159265f), 0.0f, 0.0f, 1.0f);
        glColor3f(1.0f, 1.0f, 0.0f);
        drawCircle(0.0f, 0.0f, 5);
        glPopMatrix();
    }

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

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Erro ao iniciar GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Gorillas Game Base", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    setup();

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!isProjectileMoving && !gameOver)
        {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                inputAngle += 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                inputAngle -= 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                inputForce += 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                inputForce -= 30.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            {
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