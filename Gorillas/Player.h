#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <GL/glew.h>

// Player da partida
class Player {
public:
    float x, y;
    GLuint normalTexture;
    GLuint attackTexture;
    int texWidth, texHeight;
    bool attacking;
    float attackTimer;

    Player();

    // Carrega as texturas do player (normal e ataque)
    void setup(const std::string& normalPath, const std::string& attackPath);

    // Atualiza o estado de animação do player
    void update(float deltaTime);

    // Desenha o player na tela
    void draw(float screenHeightPercent = 0.15f);

    // Começa a animação de ataque
    void startAttack();
};

#endif // PLAYER_H
