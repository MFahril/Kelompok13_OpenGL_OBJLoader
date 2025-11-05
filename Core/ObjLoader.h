#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <vector>
#include <string>
#include <map>
#include <GL/glut.h>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Vec2 {
    float u, v;
    Vec2() : u(0), v(0) {}
    Vec2(float u, float v) : u(u), v(v) {}
};

struct Material {
    std::string name;
    Vec3 ambient;      // Ka
    Vec3 diffuse;      // Kd
    Vec3 specular;     // Ks
    float shininess;   // Ns
    float transparency; // d or Tr
    int illum;         // illum model
    
    std::string ambientTexture;  // map_Ka
    std::string diffuseTexture;  // map_Kd
    std::string specularTexture; // map_Ks
    std::string bumpTexture;     // map_Bump or bump
    
    GLuint textureID;  // OpenGL texture ID
    
    Material() : ambient(0.2f, 0.2f, 0.2f), 
                 diffuse(0.8f, 0.8f, 0.8f),
                 specular(1.0f, 1.0f, 1.0f),
                 shininess(32.0f),
                 transparency(1.0f),
                 illum(2),
                 textureID(0) {}
};

struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> texCoordIndices;
    std::vector<int> normalIndices;
    std::string materialName;
};

class ObjLoader {
private:
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> texCoords;
    std::vector<Face> faces;
    std::map<std::string, Material> materials;
    
    Vec3 minBounds;
    Vec3 maxBounds;
    Vec3 center;
    float scale;
    
    std::string currentMaterial;
    std::string objDirectory;

    void calculateBounds();
    void parseLine(const std::string& line);
    void parseFace(const std::string& line);
    bool loadMaterialFile(const std::string& filename);
    void parseMaterialLine(const std::string& line, Material& mat);
    GLuint loadTexture(const std::string& filename);
    std::string getDirectory(const std::string& filepath);

public:
    ObjLoader();
    ~ObjLoader();
    bool loadObj(const std::string& filename);
    void draw();
    void drawWithNormals();
    void drawWithMaterials();
    
    // Getters
    Vec3 getCenter() const { return center; }
    float getScale() const { return scale; }
    int getVertexCount() const { return vertices.size(); }
    int getFaceCount() const { return faces.size(); }
    int getMaterialCount() const { return materials.size(); }
    bool hasMaterials() const { return !materials.empty(); }
    
    // Data access for animation (returns const references)
    const std::vector<Vec3>& getVertices() const { return vertices; }
    const std::vector<Vec3>& getNormals() const { return normals; }
    const std::vector<Vec2>& getTexCoords() const { return texCoords; }
    const std::vector<Face>& getFaces() const { return faces; }
    const std::map<std::string, Material>& getMaterials() const { return materials; }
    
    // Type aliases for AnimationLoader to use
    typedef Vec3 Vec3;
    typedef Vec2 Vec2;
    typedef Face Face;
    typedef Material Material;
};

#endif
