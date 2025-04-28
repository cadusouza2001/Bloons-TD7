// Jogo Gorillas Base - Carlos & Thomaz 
// Computacao Grafica 2025/1

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <ctime>

GLuint buildingTexture;
GLuint player1Texture;
GLuint player1AttackTexture;
float attackAnimationTimer = 0.0f; // Tempo restante da animação
GLuint player2Texture;
GLuint player2AttackTexture;
GLuint projectileTexture;
GLuint backgroundTexture;
int backgroundWidth, backgroundHeight;
GLuint hitTexture;
int hitWidth, hitHeight;
bool hitActive = false;
float hitTimer = 0.0f;
float hitX = 0.0f, hitY = 0.0f;
GLuint player1WinsTexture;
GLuint player2WinsTexture;
int winWidth, winHeight;
bool victoryScreenActive = false;
GLuint currentVictoryTexture;
bool showTrajectory = false;






GLuint loadTexture(const char* filename, int& texWidth, int& texHeight) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false); // Não flipar na carga
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Falha ao carregar " << filename << std::endl;
        exit(1);
    }

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    texWidth = width;
    texHeight = height;

    return textureID;
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
struct Player {
    float x, y;
    GLuint textureID;
    int texWidth, texHeight;
};

struct Building {
    float x, y, width, height;
    GLuint textureID;
    int texWidth, texHeight;
};

struct Projectile {
    float x, y;
    float velocityX, velocityY;
    float angle;
    float force;
    GLuint textureID;
    int texWidth, texHeight;
    float rotation = 0.0f; // << novo: rotação atual do sprite
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

        // Muda para animação de ataque
        player1.textureID = player1AttackTexture;
    }
    else
    {
        projectile.x = player2.x;
        projectile.y = player2.y;
        lastAngle2 = inputAngle;
        lastForce2 = inputForce;

        // Muda para animação de ataque
        player2.textureID = player2AttackTexture;
    }

    projectile.angle = (player1Turn ? angleDeg : (180 - angleDeg)) * (3.14159265f / 180.0f);
    projectile.force = force;
    projectile.velocityX = cos(projectile.angle) * projectile.force;
    projectile.velocityY = sin(projectile.angle) * projectile.force;
    projectile.rotation = 0.0f;
    isProjectileMoving = true;

    // Ativa o timer da animação
    attackAnimationTimer = 0.5f; // meio segundo
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
    float halfWidth = player.texWidth / 2.0f;
    float halfHeight = player.texHeight / 2.0f;

    if (proj.x >= player.x - halfWidth && proj.x <= player.x + halfWidth &&
        proj.y >= player.y - halfHeight && proj.y <= player.y + halfHeight)
    {
        return true;
    }
    return false;
}



bool checkBuildingCollision(Projectile proj)
{
    for (auto& building : buildings)
    {
        if (proj.x >= building.x && proj.x <= building.x + building.width &&
            proj.y >= building.y && proj.y <= building.y + building.height)
        {
            return true;
        }
    }
    return false;
}




void setupBuildings() {
    buildings.clear();
    int buildingWidth = 50;
    int spacing = 10;

    for (int x = 0; x < WIDTH; x += buildingWidth + spacing) {
        Building b;
        b.x = x;
        b.width = buildingWidth;
        b.height = 50 + rand() % (HEIGHT / 2); // altura entre 50 e metade da tela
        b.y = 0;

        b.textureID = loadTexture("images/building.png", b.texWidth, b.texHeight);

        buildings.push_back(b);
    }
}

void setupPlayers() {
    std::vector<Building*> leftBuildings;
    std::vector<Building*> rightBuildings;

    for (auto& building : buildings) {
        if (building.x < WIDTH / 2) {
            leftBuildings.push_back(&building);
        }
        else {
            rightBuildings.push_back(&building);
        }
    }

    if (!leftBuildings.empty()) {
        int idx = rand() % leftBuildings.size();
        Building* selected = leftBuildings[idx];
        player1.x = selected->x + selected->width / 2.0f;
        player1.y = selected->height + (HEIGHT * 0.05f);
    }

    if (!rightBuildings.empty()) {
        int idx = rand() % rightBuildings.size();
        Building* selected = rightBuildings[idx];
        player2.x = selected->x + selected->width / 2.0f;
        player2.y = selected->height + (HEIGHT * 0.05f);
    }
}

void setup()
{
    glMatrixMode(GL_PROJECTION);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    setupBuildings();
    setupPlayers();

    player1.textureID = loadTexture("images/player1.png", player1.texWidth, player1.texHeight);
    player2.textureID = loadTexture("images/player2.png", player2.texWidth, player2.texHeight);
    projectile.textureID = loadTexture("images/projectile.png", projectile.texWidth, projectile.texHeight);
    backgroundTexture = loadTexture("images/background.png", backgroundWidth, backgroundHeight);
    hitTexture = loadTexture("images/hit.png", hitWidth, hitHeight);
    player1WinsTexture = loadTexture("images/player1wins.png", winWidth, winHeight);
    player2WinsTexture = loadTexture("images/player2wins.png", winWidth, winHeight);
    player1AttackTexture = loadTexture("images/player12.png", player1.texWidth, player1.texHeight);
    player2AttackTexture = loadTexture("images/player22.png", player2.texWidth, player2.texHeight);

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


void drawProjectile(Projectile& proj, float screenHeightPercent = 0.05f) {
    float desiredHeight = HEIGHT * screenHeightPercent;
    float aspectRatio = (float)proj.texWidth / (float)proj.texHeight;
    float desiredWidth = desiredHeight * aspectRatio;

    drawTexturedRectangle(
        -desiredWidth / 2.0f,
        -desiredHeight / 2.0f,
        desiredWidth,
        desiredHeight,
        proj.textureID
    );
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

        projectile.rotation += 360.0f * deltaTime;
        if (projectile.rotation >= 360.0f)
            projectile.rotation -= 360.0f;

        if (checkCollision(player1Turn ? player2 : player1, projectile))
        {
            isProjectileMoving = false;
            hitActive = true;
            hitTimer = 1.0f;
            hitX = projectile.x;
            hitY = projectile.y;
            gameOver = true;

            // Ativa a tela de vitória após o hit
            victoryScreenActive = true;
            currentVictoryTexture = player1Turn ? player1WinsTexture : player2WinsTexture;
        }


        if (checkBuildingCollision(projectile))
        {
            isProjectileMoving = false;
            hitActive = true;
            hitTimer = 1.0f;
            hitX = projectile.x;
            hitY = projectile.y;
        }

        if (projectile.y <= 0 || projectile.x < 0 || projectile.x > WIDTH)
        {
            isProjectileMoving = false;
            player1Turn = !player1Turn;
            inputAngle = 45.0f;
            inputForce = 100.0f;
        }
    }
    else if (hitActive)
    {
        hitTimer -= deltaTime;
        if (hitTimer <= 0.0f)
        {
            hitActive = false;
            if (!gameOver)
            {
                player1Turn = !player1Turn;
                inputAngle = 45.0f;
                inputForce = 100.0f;
            }
        }
    }

    // Atualiza timer de animação de ataque
    if (attackAnimationTimer > 0.0f)
    {
        attackAnimationTimer -= deltaTime;
        if (attackAnimationTimer <= 0.0f)
        {
            // Voltar para textura normal
            player1.textureID = loadTexture("images/player1.png", player1.texWidth, player1.texHeight);
            player2.textureID = loadTexture("images/player2.png", player2.texWidth, player2.texHeight);
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

void drawPlayer(Player& player, float screenHeightPercent = 0.15f) {
    // Queremos que o jogador tenha screenHeightPercent% da altura da tela
    float desiredHeight = HEIGHT * screenHeightPercent;

    // Mantém a proporção da textura original
    float aspectRatio = (float)player.texWidth / (float)player.texHeight;
    float desiredWidth = desiredHeight * aspectRatio;

    drawTexturedRectangle(
        player.x - desiredWidth / 2.0f,
        player.y - desiredHeight / 2.0f,
        desiredWidth,
        desiredHeight,
        player.textureID
    );
}


void drawTrajectory()
{
    float angleRad = (player1Turn ? inputAngle : (180 - inputAngle)) * 3.14159265f / 180.0f;
    float vX = cos(angleRad) * inputForce * 2.5f;
    float vY = sin(angleRad) * inputForce * 2.5f;

    float projX = player1Turn ? player1.x : player2.x;
    float projY = player1Turn ? player1.y : player2.y;

    glColor3f(1.0f, 0.0f, 0.0f); // vermelho

    glBegin(GL_LINE_STRIP);
    for (float t = 0.0f; t < 5.0f; t += 0.1f)
    {
        float x = projX + vX * t;
        float y = projY + vY * t - 0.5f * GRAVITY * t * t;
        glVertex2f(x, y);

        if (x < 0 || x > WIDTH || y < 0 || y > HEIGHT)
            break;
    }
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f); // <<< volta para branco depois
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (victoryScreenActive)
    {
        drawTexturedRectangle(0, 0, WIDTH, HEIGHT, backgroundTexture);

        // Cálculo da escala para ajustar a imagem
        float scaleX = (float)WIDTH / (float)winWidth;
        float scaleY = (float)HEIGHT / (float)winHeight;
        float scale = (scaleX < scaleY) ? scaleX : scaleY; // pega o menor para caber inteiro

        float newWidth = winWidth * scale;
        float newHeight = winHeight * scale;

        drawTexturedRectangle(
            (WIDTH - newWidth) / 2.0f,
            (HEIGHT - newHeight) / 2.0f,
            newWidth,
            newHeight,
            currentVictoryTexture
        );

        return;
    }


    // 1. Desenha o fundo primeiro
    drawTexturedRectangle(0, 0, WIDTH, HEIGHT, backgroundTexture);

    // 2. Depois desenha os prédios
    for (auto& building : buildings) {
        drawTexturedRectangle(building.x, building.y, building.width, building.height, building.textureID);
    }

    // 3. Depois desenha os jogadores
    drawPlayer(player1);
    drawPlayer(player2);

    // 4. Depois desenha o projétil se estiver ativo
    if (isProjectileMoving)
    {
        glPushMatrix();
        glTranslatef(projectile.x, projectile.y, 0.0f);
        glRotatef(projectile.rotation, 0.0f, 0.0f, 1.0f);
        drawProjectile(projectile);
        glPopMatrix();
    }

    if (hitActive)
    {
        glPushMatrix();
        glTranslatef(hitX, hitY, 0.0f);
        float hitSize = HEIGHT * 0.1f; // 10% da altura da tela
        drawTexturedRectangle(
            -hitSize / 2.0f, -hitSize / 2.0f,
            hitSize, hitSize,
            hitTexture
        );
        glPopMatrix();
    }

    // 5. Agora desenha a UI (sempre por cima de tudo)

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

    // Desenhar os textos de labels
    renderTextSimple(220, HEIGHT - 27, "Angulo");
    renderTextSimple(220, HEIGHT - 47, "Forca");

    if (showTrajectory && !isProjectileMoving && !hitActive && !victoryScreenActive)
    {
        drawTrajectory();
    }

}



int main()
{
    srand(time(0));
    if (!glfwInit())
    {
        std::cerr << "Erro ao iniciar GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Bloons TD7", NULL, NULL);
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

        static bool fKeyPressed = false;

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            if (!fKeyPressed) {
                showTrajectory = !showTrajectory; // alterna
                fKeyPressed = true;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
            fKeyPressed = false;
        }


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