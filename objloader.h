#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <GL/gl.h>

typedef float (*funcXY)(float x, float y);

struct vec2 {
  float x,y;
  vec2() {x = 0.0f; y = 0.0f; }
  vec2(float X, float Y) : x(X), y(Y) {};
};

struct vec3 {
  float x,y,z;
  vec3() {x = 0.0f; y = 0.0f; z = 0.0f; }
  vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {};
};

struct vec4 {
  float x,y,z,w;
  vec4() {x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f;}
  vec4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {};
};

struct BoundingBox {
  vec3 emin, emax;
};

struct Mesh {
  std::vector<vec3> vertices;
  std::vector<vec2> uvs;
  std::vector<vec3> normals;
  std::vector<uint32_t> indices;

  GLenum faceType;

  BoundingBox bb;
  vec3 ObjCenter;
  vec3 ObjSize;
  bool loadOBJ(const char * path);
  void clearMesh();
  void drawMesh();
  void calcNormals();
};

//some vector math functions
vec3 VectorAdd(vec3 &a, vec3 &b);
float length(vec3 &a);
vec3 cross(vec3 &a, vec3 &b);
void normalize(vec3 &a);
vec3 CalcNormal(vec3 &v1, vec3 &v2, vec3 &v3);

//Some primitives
void createSurface(Mesh &surface, float w, float h, float tilesX, float tilesY, funcXY fxy = NULL);
void createSphere(Mesh & sphere, float R, int VSeg, int HSeg, bool invertNormals = false, bool hemisphere = false);

#endif
