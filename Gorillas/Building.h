#ifndef BUILDING_H
#define BUILDING_H

#include <GL/glew.h>
#include <string>

// Representa um prédio no jogo
class Building {
public:
    float x, y;
    float width, height;
    GLuint textureID;
    int texWidth, texHeight;

    Building();

    // Configura o prédio com textura e posição
    void setup(float posX, float baseY, float w, float h, const std::string& texturePath);

    // Desenha o prédio na tela
    void draw();
};

#endif // BUILDING_H
