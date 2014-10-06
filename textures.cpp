#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <GL/gl.h>
#include <GL/glut.h>

#include "textures.h"


bool loadTGA(Texture2D &tex2d, std::string const & aFilename) {
  std::ifstream FileIn(aFilename.c_str(), std::ios::in | std::ios::binary);
  tex2d.data = 0;
  tex2d.width = -1;
  tex2d.height = -1;
  if(!FileIn) {
    printf("TGA loading error\n");
    return false;
  }

  unsigned char IdentificationFieldSize;
  unsigned char ColorMapType;
  unsigned char ImageType;
  unsigned short ColorMapOrigin;
  unsigned short ColorMapLength;
  unsigned char ColorMapEntrySize;
  unsigned short OriginX;
  unsigned short OriginY;
  unsigned short Width;
  unsigned short Height;
  unsigned char TexelSize;
  unsigned char Descriptor;

  FileIn.read((char*)&IdentificationFieldSize, sizeof(IdentificationFieldSize));
  FileIn.read((char*)&ColorMapType, sizeof(ColorMapType));
  FileIn.read((char*)&ImageType, sizeof(ImageType));
  FileIn.read((char*)&ColorMapOrigin, sizeof(ColorMapOrigin));
  FileIn.read((char*)&ColorMapLength, sizeof(ColorMapLength));
  FileIn.read((char*)&ColorMapEntrySize, sizeof(ColorMapEntrySize));
  FileIn.read((char*)&OriginX, sizeof(OriginX));
  FileIn.read((char*)&OriginY, sizeof(OriginY));
  FileIn.read((char*)&Width, sizeof(Width));
  FileIn.read((char*)&Height, sizeof(Height));
  FileIn.read((char*)&TexelSize, sizeof(TexelSize));
  FileIn.read((char*)&Descriptor, sizeof(Descriptor));

  tex2d.type = GL_UNSIGNED_BYTE;
  tex2d.width = Width; tex2d.height = Height;
  if(TexelSize == 24) {tex2d.internalFormat = GL_RGB8; tex2d.format = GL_BGR; }
  else if(TexelSize == 32) {tex2d.internalFormat = GL_RGBA8; tex2d.format = GL_BGRA; }
  else return false;
  tex2d.bpp = TexelSize / 8;

  if (FileIn.fail() || FileIn.bad()) return false;

  switch(ImageType) {
    default:
      return false;
    case 2:
      FileIn.seekg(18 + ColorMapLength, std::ios::beg);

      char* IdentificationField = new char[IdentificationFieldSize + 1];
      FileIn.read(IdentificationField, IdentificationFieldSize);
      IdentificationField[IdentificationFieldSize] = '\0';
      delete[] IdentificationField;

      std::size_t DataSize = Width * Height * (TexelSize >> 3);
      tex2d.data = new char [DataSize];
      FileIn.read((char*)tex2d.data, std::streamsize(DataSize));

      if(FileIn.fail() || FileIn.bad()) return false;
      break;
  }

  FileIn.close();
  return true;
}

void Texture2D::createTexture(bool filters, bool free_data) {
  glGenTextures(1, &Id);

  glBindTexture(GL_TEXTURE_2D, Id);
  if (filters) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, 1);
  } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, 0);
  };

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
  glBindTexture(GL_TEXTURE_2D, 0);
  if (free_data) {
    delete (uint8_t*)data;
    data = NULL;
  }

}

color Texture2D::getPixel(uint32_t x, uint32_t y) {
  if (!data) return color(0,0,0,0);
  color c;
  uint32_t offset = (y*height+x)*bpp;
  uint8_t *ptr = (uint8_t *)data + offset;
  c.b = *ptr++;
  c.g = *ptr++;
  c.r = *ptr++;
  if (bpp == 4) c.a = *ptr;
  else c.a = 255;
  return c;
}


color Texture2D::getPixel(float s, float t) {
  if (!data) return color(0,0,0,0);
  color c;
  float x = int(s*width);
  float y = int(t*height);
  if (s < 0.0f || t < 0.0f || s >= 1.0f || t >= 1.0f)
    return color(0,0,0,0);
  uint32_t offset = (y*height+x)*bpp;
  uint8_t *ptr = (uint8_t *)data + offset;
  c.b = *ptr++;
  c.g = *ptr++;
  c.r = *ptr++;
  if (bpp == 4) c.a = *ptr;
  else c.a = 255;
  return c;
}
