#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <GL/glew.h>
#include <string>

// Representa o projétil (banana, bola, etc)
class Projectile {
public:
    float x, y;
    float velocityX, velocityY;
    float angle, force;
    float rotation;
    GLuint textureID;
    int texWidth, texHeight;
    bool active;

    Projectile();

    // Inicializa a textura do projétil
    void setup(const std::string& texturePath);

    // Dispara o projétil
    void shoot(float startX, float startY, float angleDeg, float forceVal);

    // Atualiza posição e rotação
    void update(float deltaTime);

    // Desenha o projétil na tela
    void draw(float screenHeightPercent = 0.05f);
};

#endif // PROJECTILE_H
