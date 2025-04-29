#include "Building.h"
#include "utils.h"
#include <GL/gl.h>

Building::Building()
    : x(0), y(0), width(50), height(100), textureID(0), texWidth(0), texHeight(0) {
}

void Building::setup(float posX, float baseY, float w, float h, const std::string& texturePath) {
    x = posX;
    y = baseY;
    width = w;
    height = h;
    textureID = loadTexture(texturePath.c_str(), &texWidth, &texHeight);
}

void Building::draw() {
    drawTexturedRectangle(x, y, width, height, textureID);
}