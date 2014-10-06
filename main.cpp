#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <vector>
#include "objloader.h"
#include "textures.h"
#include "bitmap_image.hpp"
#include <math.h>

int window_width = 640;
int window_height = 480;

float const PI = 3.14159265;
float const RAD_PER_DEGREE = PI / 180;

static float theta = 180.0f;
static float y_angle = 0.0f;
static float x = 0.0f;
static float y = 0.0f;
static Mesh surface;
static Mesh sky;

int meshWidth;
int meshHeight;

GLuint texId;

using namespace std;

void printError(int place) {
	int err = glGetError();
	if (err) {
		std::cout << err << " at " << place << '\n';
		std::cout.flush();
	}
}

std::ostream & operator <<(std::ostream & is, vec3 v) {
	is << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return is;
}

float length(vec3 &a) {
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

vec3 cross(vec3 &a, vec3 &b) {
	vec3 c;
	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.z - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;
	return c;
}

void normalize(vec3 &a) {
	float d = length(a);
	a.x /= d;
	a.y /= d;
	a.z /= d;
}

vec3 CalcNormal(vec3 &v1, vec3 &v2, vec3 &v3) {
	vec3 a, b;
	a.x = v2.x - v1.x;
	a.y = v2.y - v1.y;
	a.z = v2.z - v1.z;
	b.x = v3.x - v1.x;
	b.y = v3.y - v1.y;
	b.z = v3.z - v1.z;
	normalize(a);
	normalize(b);
	return cross(a, b);
}

vec3 VectorAdd(vec3 &a, vec3 &b) {
	vec3 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;

	return res;
}

void CalcMeshNormals(Mesh &mesh) {
	vec3 v1, v2, v3;
	unsigned int indiciesSize = mesh.indices.size();
	for (size_t i = 0; i < indiciesSize / 3; ++i) {
		v1 = mesh.vertices[mesh.indices[i * 3]];
		v2 = mesh.vertices[mesh.indices[i * 3 + 1]];
		v3 = mesh.vertices[mesh.indices[i * 3 + 2]];
		vec3 n = CalcNormal(v1, v2, v3);
		mesh.normals[mesh.indices[i * 3]] = VectorAdd(mesh.normals[mesh.indices[i * 3]], n);
		mesh.normals[mesh.indices[i * 3 + 1]] = VectorAdd(mesh.normals[mesh.indices[i * 3 + 1]], n);
		mesh.normals[mesh.indices[i * 3 + 2]] = VectorAdd(mesh.normals[mesh.indices[i * 3 + 2]], n);
	}
	for (size_t i = 0; i < mesh.normals.size(); ++i) {
		normalize(mesh.normals[i]);
	}
}

void getMeshFromBitmap(const std::string& filename) {
	//TODO check width
	using namespace std;
	bitmap_image image(filename);

	if (!image) {
		cout << "Error - Failed to open: input.bmp\n";
	}

	unsigned char red;
	unsigned char green;
	unsigned char blue;

	const unsigned int height = image.height();
	const unsigned int width = image.width();
	meshHeight = height;
	meshWidth = width;
	vec3 zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;

	for (unsigned int y = 0; y < height; ++y) {
		for (unsigned int x = 0; x < width; ++x) {
			image.get_pixel(x, y, red, green, blue);
			float avarage = float(red + green + blue) / float(30);
			vec3 v;
			vec2 uv;
			v.x = float(x);
			v.z = float(y);
			v.y = avarage;
			uv.x = float(x) / 20;
			uv.y = float(y) / 20;
			surface.vertices.push_back(v);
			surface.normals.push_back(zero);
			surface.uvs.push_back(uv);
		}
	}
	unsigned long pos = 0;
	for (unsigned int y = 0; y < (height - 1); ++y) {
		for (unsigned int x = 0; x < (width - 1); ++x) {
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + width);
			surface.indices.push_back(pos + width + 1);
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + width + 1);
			surface.indices.push_back(pos + 1);
			++pos;
		}
		pos++;
	}
}

void getSkyMesh(float r, int hsectors, int wsectors) {
	vec3 top = { 0, 0, r };
	sky.vertices.push_back(top);
	vec3 zero = { 0.0f, 0.0f, 0.0f };
	for (int zSector = 1; zSector <= hsectors; zSector++) {
		float zAngle = 90 - float(zSector * 180) / hsectors;
		float zR = r * cos(zAngle * RAD_PER_DEGREE);
		float zCord = r * sin(zAngle * RAD_PER_DEGREE);
		for (int xySector = 0; xySector < wsectors; xySector++) {
			float xyArc = float(xySector * 360) / wsectors;
			float xCord = zR * cos(xyArc * RAD_PER_DEGREE);
			float yCord = zR * sin(xyArc * RAD_PER_DEGREE);
			vec3 current = { xCord, yCord, zCord };
			sky.vertices.push_back(current);
			sky.normals.push_back(zero);
			if (zSector == 1) {
				if (xySector != 0) {
					sky.indices.push_back(0);
					sky.indices.push_back(xySector + 1);
					sky.indices.push_back(xySector);
				}
				if (xySector == (wsectors - 1)) {
					sky.indices.push_back(0);
					sky.indices.push_back(1);
					sky.indices.push_back(xySector + 1);
				}
			} else {
				if (xySector != 0) {
					sky.indices.push_back(1 + (zSector - 2) * wsectors + xySector);
					sky.indices.push_back(1 + (zSector - 1) * wsectors + xySector);
					sky.indices.push_back(1 + (zSector - 1) * wsectors + xySector - 1);

					sky.indices.push_back(1 + (zSector - 2) * wsectors + xySector);
					sky.indices.push_back(1 + (zSector - 1) * wsectors + xySector - 1);
					sky.indices.push_back(1 + (zSector - 2) * wsectors + xySector - 1);
				}
				if (xySector == (wsectors - 1)) {
					sky.indices.push_back(1 + (zSector - 2) * wsectors);
					sky.indices.push_back(1 + (zSector - 1) * wsectors);
					sky.indices.push_back(1 + (zSector - 1) * wsectors + xySector);

					sky.indices.push_back(1 + (zSector - 2) * wsectors + xySector);
					sky.indices.push_back(1 + (zSector - 2) * wsectors);
					sky.indices.push_back(1 + (zSector - 1) * wsectors + xySector);
				}
			}
		}
	}
}

void InitScene() {
	getMeshFromBitmap("surface4.bmp");
	getSkyMesh(400, 100, 100);
	CalcMeshNormals(surface);
	CalcMeshNormals(sky);
}

void loadTexture(const std::string& filename) {
	using namespace std;
	bitmap_image image(filename);

	if (!image) {
		cout << "Error - Failed to open: " << filename;
		cout.flush();
	}

	unsigned char red;
	unsigned char green;
	unsigned char blue;

	const unsigned int height = image.height();
	const unsigned int width = image.width();

	GLubyte ***data = new GLubyte**[height];
	for (unsigned int i = 0; i < height; ++i) {
		data[i] = new GLubyte*[width];
		for (unsigned int j = 0; j < width; ++j)
			data[i][j] = new GLubyte[4];
	}

//	GLubyte data[512][512][4];

//	for (unsigned int y = 0; y < height; ++y) {
//		for (unsigned int x = 0; x < width; ++x) {
//			image.get_pixel(x, y, red, green, blue);
//			data[y][x][0] = red;
//			data[y][x][1] = blue;
//			data[y][x][2] = green;
//			data[y][x][3] = 255;
//		}
//	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
	GL_UNSIGNED_BYTE, data);
	delete[] data;
}

void main_loop_function() {
	// And depth (used internally to block obstructed objects)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load identity matrix
	glLoadIdentity();
	glPushMatrix();
	glRotatef(-90 + y_angle, 1.0f, 0.0f, 0.0f);
	glRotatef(theta, 0.0f, 1.0f, 0.0f);
	int vertexPositio = -(int(x) + int(y) * meshWidth);
	vec3 vertex = surface.vertices[vertexPositio];
	glTranslatef(x, -5 - vertex.y, y);

	vec4 pos;
	pos.x = 0.0f;
	pos.y = 80.0f;
	pos.z = -50.0f;
	pos.w = 0.0f;
	glLightfv(GL_LIGHT0, GL_POSITION, &pos.x);

//	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, 0, &surface.uvs.front());
	glNormalPointer(GL_FLOAT, 0, &surface.normals.front());
	glVertexPointer(3, GL_FLOAT, 0, &surface.vertices.front());

	glDrawElements(GL_TRIANGLES, surface.indices.size(), GL_UNSIGNED_INT, &surface.indices.front());

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);

	//TODO disable light for sphere
	glNormalPointer(GL_FLOAT, 0, &sky.normals.front());
	glVertexPointer(3, GL_FLOAT, 0, &sky.vertices.front());
	glDrawElements(GL_TRIANGLES, sky.indices.size(), GL_UNSIGNED_INT, &sky.indices.front());

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();
	glutSwapBuffers();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load identity matrix
	glLoadIdentity();
	glPushMatrix();
	glFlush();

	//draw textured object
}

void GL_Setup(int width, int height) {
	InitScene();
	float green_mat[] = { 0.1, 0.1, 0.1, 0.0 };
	float green_mat_d[] = { .0f, 0.8f, 0.8f, 0.0 };
	float shines[] = { 50 };
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(true);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, green_mat);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, green_mat_d);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, green_mat_d);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shines);
	glClearColor(0.0f, 0.7f, 0.7f, 0.0f);
	glShadeModel(GLU_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, width, height);
	gluPerspective(70.0f, (float) width / height, .1, 700);

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

//setup pixel alignment in memory
	glPixelStorei( GL_PACK_ALIGNMENT, 1);
	glPixelStorei( GL_PACK_ROW_LENGTH, 0);
	glPixelStorei( GL_PACK_SKIP_ROWS, 0);
	glPixelStorei( GL_PACK_SKIP_PIXELS, 0);

//	glDisable(GL_COLOR_MATERIAL);
	//allocate memory and fill in with color

	loadTexture("grass_texture.bmp");

	//set wrapping and filtering mode

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_MODELVIEW);
}

int step = 1;

void onKeyEvent(unsigned char key, int xMouse, int yMouse) {
	switch (key) {
	case 'w':
		x -= step * sinf(theta * RAD_PER_DEGREE);
		y += step * cosf(theta * RAD_PER_DEGREE);
		break;
	case 's':
		x += step * sinf(theta * RAD_PER_DEGREE);
		y -= step * cosf(theta * RAD_PER_DEGREE);
		break;
	case 'a':
		x -= step * sinf((theta - 90) * RAD_PER_DEGREE);
		y += step * cosf((theta - 90) * RAD_PER_DEGREE);
		break;
	case 'd':
		x -= step * sinf((theta + 90) * RAD_PER_DEGREE);
		y += step * cosf((theta + 90) * RAD_PER_DEGREE);
		break;
	}
	if (y > 0) {
		y = 0;
	} else if (y < -meshHeight) {
		y = -meshHeight;
	}
	if (x > 0) {
		x = 0;
	} else if (x < -meshWidth) {
		x = -meshWidth;
	}
}

void mouseFunction(int x, int y) {
	theta = 360 * float(x) / float(window_width);
	y_angle = 180 * float(y) / float(window_height);
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(window_width, window_height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Hello GL");
	glutIdleFunc(main_loop_function);
	GL_Setup(window_width, window_height);
	glutKeyboardFunc(onKeyEvent);
	glutPassiveMotionFunc(mouseFunction);
	glutReshapeFunc(reshape);
	glutMainLoop();
}
