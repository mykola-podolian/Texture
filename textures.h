#ifndef TEXTURES_H
#define TEXTURES_H

#include "GL/gl.h"
#include <string>

struct color {
    uint8_t r,g,b,a;
    color() { r = g = b = a = 0; }
    color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {};
};

struct Texture2D {
    GLuint Id;
    int width, height;
    void *data;
    GLuint internalFormat;
    GLuint format;
    GLenum type;
    uint8_t bpp;
    Texture2D() : Id(0) {};
    ~Texture2D() {
        glDeleteTextures(1,&Id);
        if (data) delete (uint8_t*)data;
    }
    void createTexture(bool filters = true, bool free_data = true);
    color getPixel(uint32_t x, uint32_t y);
    color getPixel(float x, float y);
};

bool loadTGA(Texture2D &tex2d, std::string const & aFilename);

#endif //TEXTURES_H
