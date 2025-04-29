#ifndef UTILS_H
#define UTILS_H

#include <GL/glew.h>
#include <string>
#include <iostream>
#include "stb_image.h"

// Fun��o para carregar uma textura 2D do disco
inline GLuint loadTexture(const std::string &filename, int *width = nullptr, int *height = nullptr)
{
    int texWidth, texHeight, nrChannels;

    stbi_set_flip_vertically_on_load(false); // N�o inverter verticalmente
    unsigned char *data = stbi_load(filename.c_str(), &texWidth, &texHeight, &nrChannels, 0);

    if (!data)
    {
        std::cerr << "Erro ao carregar textura: " << filename << std::endl;
        exit(1); // Encerra se n�o encontrar a textura
    }

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    if (width)
        *width = texWidth;
    if (height)
        *height = texHeight;

    return textureID;
}

// Fun��o para desenhar um ret�ngulo texturizado
inline void drawTexturedRectangle(float x, float y, float width, float height, GLuint textureID)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x, y); // inferior esquerdo
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + width, y); // inferior direito
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + width, y + height); // superior direito
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y + height); // superior esquerdo
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// Fun��o para desenhar um c�rculo texturizado
inline void drawTexturedCircle(float cx, float cy, float r, GLuint textureID)
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

inline void drawBar(float x, float y, float width, float height, float percent, float r, float g, float b, float lastPercent)
{
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width * percent, y);
    glVertex2f(x + width * percent, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Linha branca no �ltimo valor
    glColor3f(1.0f, 1.0f, 1.0f);
    float markerX = x + width * lastPercent;
    glBegin(GL_LINES);
    glVertex2f(markerX, y);
    glVertex2f(markerX, y + height);
    glEnd();
}

#endif // UTILS_H
