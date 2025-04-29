#include "Player.h"
#include "utils.h" // função loadTexture()
#include <GL/gl.h>

Player::Player()
    : x(0), y(0), normalTexture(0), attackTexture(0),
    texWidth(0), texHeight(0), attacking(false), attackTimer(0.0f) {
}

// Inicializa as texturas do player
void Player::setup(const std::string& normalPath, const std::string& attackPath) {
    normalTexture = loadTexture(normalPath.c_str(), &texWidth, &texHeight);
    attackTexture = loadTexture(attackPath.c_str(), &texWidth, &texHeight);
}

// Atualiza o player (controle de ataque temporizado)
void Player::update(float deltaTime) {
    if (attacking) {
        attackTimer -= deltaTime;
        if (attackTimer <= 0.0f) {
            attacking = false; // volta para sprite normal
        }
    }
}

// Desenha o player na posição atual
void Player::draw(float screenHeightPercent) {
    GLuint currentTexture = attacking ? attackTexture : normalTexture;

    float desiredHeight = 600.0f * screenHeightPercent; // (considerando altura da tela 600)
    float aspectRatio = (float)texWidth / (float)texHeight;
    float desiredWidth = desiredHeight * aspectRatio;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, currentTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x - desiredWidth / 2, y - desiredHeight / 2);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + desiredWidth / 2, y - desiredHeight / 2);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + desiredWidth / 2, y + desiredHeight / 2);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x - desiredWidth / 2, y + desiredHeight / 2);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// Inicia o ataque (troca sprite por tempo)
void Player::startAttack() {
    attacking = true;
    attackTimer = 0.5f; // meio segundo de animação
}
