#include "Projectile.h"
#include "utils.h"
#include <GL/gl.h>
#include <cmath>

#define GRAVITY 150.0f // Gravidade do jogo (pode ser global também)

Projectile::Projectile()
    : x(0.0f), y(0.0f),
    velocityX(0.0f), velocityY(0.0f),
    angle(0.0f), force(0.0f),
    rotation(0.0f),
    textureID(0),
    texWidth(0), texHeight(0),
    active(false) {
}

void Projectile::setup(const std::string& texturePath) {
    textureID = loadTexture(texturePath.c_str(), &texWidth, &texHeight);
}

// Inicializa o disparo
void Projectile::shoot(float startX, float startY, float angleDeg, float forceVal) {
    x = startX;
    y = startY;
    angle = angleDeg;
    force = forceVal;
    velocityX = cos(angle * 3.14159265f / 180.0f) * force;
    velocityY = sin(angle * 3.14159265f / 180.0f) * force;
    rotation = 0.0f;
    active = true;
}

// Atualiza posição e rotação
void Projectile::update(float deltaTime) {
    if (!active)
        return;

    x += velocityX * deltaTime;
    y += velocityY * deltaTime;
    velocityY -= GRAVITY * deltaTime;

    rotation += 360.0f * deltaTime;
    if (rotation >= 360.0f)
        rotation -= 360.0f;
}

// Desenha o projétil rotacionado
void Projectile::draw(float screenHeightPercent) {
    if (!active)
        return;

    float desiredHeight = 600.0f * screenHeightPercent; // base altura tela 600
    float aspectRatio = (float)texWidth / (float)texHeight;
    float desiredWidth = desiredHeight * aspectRatio;

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);

    drawTexturedRectangle(
        -desiredWidth / 2.0f,
        -desiredHeight / 2.0f,
        desiredWidth,
        desiredHeight,
        textureID
    );

    glPopMatrix();
}
