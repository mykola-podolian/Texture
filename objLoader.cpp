#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include "objloader.h"

vec3 min(vec3 &a, vec3 &b) {
  vec3 c;
  c.x = a.x < b.x ? a.x : b.x;
  c.y = a.y < b.y ? a.y : b.y;
  c.z = a.z < b.z ? a.z : b.z;
  return c;
}

vec3 max(vec3 &a, vec3 &b) {
  vec3 c;
  c.x = a.x > b.x ? a.x : b.x;
  c.y = a.y > b.y ? a.y : b.y;
  c.z = a.z > b.z ? a.z : b.z;
  return c;
}

bool Mesh::loadOBJ(const char * path)
{
        vertices.clear();
        uvs     .clear();
        normals .clear();

        printf("Loading OBJ file %s...\n", path);

        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<vec3> temp_vertices;
        std::vector<vec2> temp_uvs;
        std::vector<vec3> temp_normals;


        FILE * file = fopen(path, "r");
        if( file == NULL ){
                printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
                getchar();
                return false;
        }

        while( 1 ){

                char lineHeader[128];
                // read the first word of the line
                int res = fscanf(file, "%s", lineHeader);
                if (res == EOF)
                        break; // EOF = End Of File. Quit the loop.

                // else : parse lineHeader

                if ( strcmp( lineHeader, "v" ) == 0 ){
                        vec3 vertex;
                        fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
                        temp_vertices.push_back(vertex);
                }else if ( strcmp( lineHeader, "vt" ) == 0 ){
                        vec2 uv;
                        fscanf(file, "%f %f\n", &uv.x, &uv.y );
                        uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
                        temp_uvs.push_back(uv);
                }else if ( strcmp( lineHeader, "vn" ) == 0 ){
                        vec3 normal;
                        fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
                        temp_normals.push_back(normal);
                }else if ( strcmp( lineHeader, "f" ) == 0 ){
                        std::string vertex1, vertex2, vertex3;
                        unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                        int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
                        if (matches != 9){
                                printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                                return false;
                        }
                        vertexIndices.push_back(vertexIndex[0]);
                        vertexIndices.push_back(vertexIndex[1]);
                        vertexIndices.push_back(vertexIndex[2]);
                        uvIndices    .push_back(uvIndex[0]);
                        uvIndices    .push_back(uvIndex[1]);
                        uvIndices    .push_back(uvIndex[2]);
                        normalIndices.push_back(normalIndex[0]);
                        normalIndices.push_back(normalIndex[1]);
                        normalIndices.push_back(normalIndex[2]);
                }else{
                        // Probably a comment, eat up the rest of the line
                        char stupidBuffer[1000];
                        fgets(stupidBuffer, 1000, file);
                }

        }

        // For each vertex of each triangle
        for( unsigned int i=0; i<vertexIndices.size(); i++ ){

                // Get the indices of its attributes
                unsigned int vertexIndex = vertexIndices[i];
                unsigned int uvIndex = uvIndices[i];
                unsigned int normalIndex = normalIndices[i];

                // Get the attributes thanks to the index
                vec3 vertex = temp_vertices[ vertexIndex-1 ];
                vec2 uv = temp_uvs[ uvIndex-1 ];
                vec3 normal = temp_normals[ normalIndex-1 ];

                // Put the attributes in buffers
                vertices.push_back(vertex);
                uvs     .push_back(uv);
                normals .push_back(normal);

                if (i==0) {
                    bb.emin = vertex; bb.emax=vertex;
                } else {
                  bb.emin = min(bb.emin,vertex);
                  bb.emax = max(bb.emax,vertex);
                }
        }

        ObjSize.x = bb.emax.x - bb.emin.x;
        ObjSize.y = bb.emax.y - bb.emin.y;
        ObjSize.z = bb.emax.z - bb.emin.z;
        ObjCenter.x = ObjSize.x / 2.0f + bb.emin.x;
        ObjCenter.y = ObjSize.y / 2.0f + bb.emin.y;
        ObjCenter.z = ObjSize.z / 2.0f + bb.emin.z;


        return true;
}
