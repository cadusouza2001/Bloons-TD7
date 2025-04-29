#ifndef BUILDING_H
#define BUILDING_H

#include <GL/glew.h>
#include <string>

// Representa um pr�dio no jogo
class Building {
public:
    float x, y;
    float width, height;
    GLuint textureID;
    int texWidth, texHeight;

    Building();

    // Configura o pr�dio com textura e posi��o
    void setup(float posX, float baseY, float w, float h, const std::string& texturePath);

    // Desenha o pr�dio na tela
    void draw();
};

#endif // BUILDING_H
