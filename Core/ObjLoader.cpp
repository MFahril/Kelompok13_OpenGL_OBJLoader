#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

// For texture loading - using simple BMP loader
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ObjLoader::ObjLoader() : scale(1.0f) {
    minBounds = Vec3(1e10, 1e10, 1e10);
    maxBounds = Vec3(-1e10, -1e10, -1e10);
}

ObjLoader::~ObjLoader() {
    // Clean up textures
    for (auto& matPair : materials) {
        if (matPair.second.textureID != 0) {
            glDeleteTextures(1, &matPair.second.textureID);
        }
    }
}

std::string ObjLoader::getDirectory(const std::string& filepath) {
    size_t found = filepath.find_last_of("/\\");
    if (found != std::string::npos) {
        return filepath.substr(0, found + 1);
    }
    return "";
}

bool ObjLoader::loadObj(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    objDirectory = getDirectory(filename);
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        if (line.empty() || line[0] == '#') continue;

        try {
            parseLine(line);
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing line " << lineNum << ": " << e.what() << std::endl;
        }
    }

    file.close();

    calculateBounds();

    std::cout << "OBJ file loaded successfully:" << std::endl;
    std::cout << "  Vertices: " << vertices.size() << std::endl;
    std::cout << "  Normals: " << normals.size() << std::endl;
    std::cout << "  Texture Coords: " << texCoords.size() << std::endl;
    std::cout << "  Faces: " << faces.size() << std::endl;
    std::cout << "  Materials: " << materials.size() << std::endl;
    std::cout << "  Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
    std::cout << "  Scale: " << scale << std::endl;

    // Check for faces without materials
    int facesWithoutMaterial = 0;
    for (const auto& face : faces) {
        if (face.materialName.empty()) {
            facesWithoutMaterial++;
        }
    }
    if (facesWithoutMaterial > 0) {
        std::cout << "  Warning: " << facesWithoutMaterial << " faces without material" << std::endl;
    }

    return true;
}

void ObjLoader::parseLine(const std::string& line) {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    if (prefix == "v") {
        // Vertex position
        Vec3 vertex;
        iss >> vertex.x >> vertex.y >> vertex.z;
        vertices.push_back(vertex);

        // Update bounds
        minBounds.x = std::min(minBounds.x, vertex.x);
        minBounds.y = std::min(minBounds.y, vertex.y);
        minBounds.z = std::min(minBounds.z, vertex.z);
        maxBounds.x = std::max(maxBounds.x, vertex.x);
        maxBounds.y = std::max(maxBounds.y, vertex.y);
        maxBounds.z = std::max(maxBounds.z, vertex.z);
    }
    else if (prefix == "vn") {
        // Vertex normal
        Vec3 normal;
        iss >> normal.x >> normal.y >> normal.z;
        normals.push_back(normal);
    }
    else if (prefix == "vt") {
        // Texture coordinate
        Vec2 texCoord;
        iss >> texCoord.u >> texCoord.v;
        texCoords.push_back(texCoord);
    }
    else if (prefix == "f") {
        // Face
        parseFace(line);
    }
    else if (prefix == "mtllib") {
        // Material library file
        std::string mtlFile;
        // Read the rest of the line to handle filenames with spaces
        std::getline(iss >> std::ws, mtlFile);
        loadMaterialFile(objDirectory + mtlFile);
    }
    else if (prefix == "usemtl") {
        // Use material
        // Read the rest of the line to handle material names with spaces
        std::string materialName;
        std::getline(iss >> std::ws, materialName);
        currentMaterial = materialName;
    }
}

void ObjLoader::parseFace(const std::string& line) {
    Face face;
    face.materialName = currentMaterial;

    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix; // Skip 'f'

    std::string vertexData;
    while (iss >> vertexData) {
        std::istringstream vertexStream(vertexData);
        std::string indexStr;
        int idx = 0;

        // Parse vertex/texture/normal indices
        // Formats: v, v/vt, v/vt/vn, v//vn
        while (std::getline(vertexStream, indexStr, '/')) {
            if (!indexStr.empty()) {
                int index = std::stoi(indexStr);
                // OBJ indices are 1-based, convert to 0-based
                if (index > 0) index--;
                else if (index < 0) index = (idx == 0 ? vertices.size() :
                    idx == 1 ? texCoords.size() :
                    normals.size()) + index;

                if (idx == 0) face.vertexIndices.push_back(index);
                else if (idx == 1) face.texCoordIndices.push_back(index);
                else if (idx == 2) face.normalIndices.push_back(index);
            }
            idx++;
        }
    }

    faces.push_back(face);
}

void ObjLoader::calculateBounds() {
    // Calculate center
    center.x = (minBounds.x + maxBounds.x) / 2.0f;
    center.y = (minBounds.y + maxBounds.y) / 2.0f;
    center.z = (minBounds.z + maxBounds.z) / 2.0f;

    // Calculate scale (for normalization)
    float dx = maxBounds.x - minBounds.x;
    float dy = maxBounds.y - minBounds.y;
    float dz = maxBounds.z - minBounds.z;
    float maxDim = std::max({ dx, dy, dz });

    if (maxDim > 0) {
        scale = 2.0f / maxDim; // Normalize to fit in [-1, 1]
    }
}

void ObjLoader::draw() {
    glPushMatrix();

    // Center and scale the model
    glScalef(scale, scale, scale);
    glTranslatef(-center.x, -center.y, -center.z);

    // Draw all faces
    for (const auto& face : faces) {
        if (face.vertexIndices.size() == 3) {
            glBegin(GL_TRIANGLES);
        }
        else if (face.vertexIndices.size() == 4) {
            glBegin(GL_QUADS);
        }
        else {
            glBegin(GL_POLYGON);
        }

        for (size_t i = 0; i < face.vertexIndices.size(); i++) {
            // Apply normal if available
            if (i < face.normalIndices.size()) {
                int nIdx = face.normalIndices[i];
                if (nIdx >= 0 && nIdx < normals.size()) {
                    glNormal3f(normals[nIdx].x, normals[nIdx].y, normals[nIdx].z);
                }
            }

            // Apply texture coordinate if available
            if (i < face.texCoordIndices.size()) {
                int tIdx = face.texCoordIndices[i];
                if (tIdx >= 0 && tIdx < texCoords.size()) {
                    glTexCoord2f(texCoords[tIdx].u, texCoords[tIdx].v);
                }
            }

            // Apply vertex
            int vIdx = face.vertexIndices[i];
            if (vIdx >= 0 && vIdx < vertices.size()) {
                glVertex3f(vertices[vIdx].x, vertices[vIdx].y, vertices[vIdx].z);
            }
        }

        glEnd();
    }

    glPopMatrix();
}

void ObjLoader::drawWithNormals() {
    // Same as draw but ensures lighting is enabled
    draw();
}

bool ObjLoader::loadMaterialFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Cannot open material file " << filename << std::endl;
        return false;
    }

    Material currentMat;
    std::string currentMatName;
    bool hasMaterial = false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "newmtl") {
            // Save previous material
            if (hasMaterial) {
                materials[currentMatName] = currentMat;
            }
            // Start new material
            iss >> currentMatName;
            currentMat = Material();
            currentMat.name = currentMatName;
            hasMaterial = true;
        }
        else if (hasMaterial) {
            parseMaterialLine(line, currentMat);
        }
    }

    // Save last material
    if (hasMaterial) {
        materials[currentMatName] = currentMat;
    }

    file.close();

    std::cout << "Loaded " << materials.size() << " materials from " << filename << std::endl;
    return true;
}

void ObjLoader::parseMaterialLine(const std::string& line, Material& mat) {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    if (prefix == "Ka") {
        // Ambient color
        iss >> mat.ambient.x >> mat.ambient.y >> mat.ambient.z;
    }
    else if (prefix == "Kd") {
        // Diffuse color
        iss >> mat.diffuse.x >> mat.diffuse.y >> mat.diffuse.z;
    }
    else if (prefix == "Ks") {
        // Specular color
        iss >> mat.specular.x >> mat.specular.y >> mat.specular.z;
    }
    else if (prefix == "Ns") {
        // Shininess
        iss >> mat.shininess;
    }
    else if (prefix == "d" || prefix == "Tr") {
        // Transparency
        iss >> mat.transparency;
    }
    else if (prefix == "illum") {
        // Illumination model
        iss >> mat.illum;
    }
    else if (prefix == "map_Kd") {
        // Diffuse texture map
        iss >> mat.diffuseTexture;
        mat.textureID = loadTexture(objDirectory + mat.diffuseTexture);
    }
    else if (prefix == "map_Ka") {
        // Ambient texture map
        iss >> mat.ambientTexture;
    }
    else if (prefix == "map_Ks") {
        // Specular texture map
        iss >> mat.specularTexture;
    }
    else if (prefix == "map_Bump" || prefix == "bump") {
        // Bump map
        iss >> mat.bumpTexture;
    }
}

GLuint ObjLoader::loadTexture(const std::string& filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Warning: Failed to load texture " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload texture data with mipmaps (using gluBuild2DMipmaps for legacy OpenGL compatibility)
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    std::cout << "Loaded texture: " << filename << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;

    return textureID;
}

void ObjLoader::drawWithMaterials() {
    glPushMatrix();

    // Center and scale the model
    glScalef(scale, scale, scale);
    glTranslatef(-center.x, -center.y, -center.z);

    glEnable(GL_TEXTURE_2D);

    // Group faces by material for efficiency
    std::string lastMaterial;

    for (const auto& face : faces) {
        // Apply material if changed
        if (face.materialName != lastMaterial) {
            lastMaterial = face.materialName;

            if (!face.materialName.empty() && materials.find(face.materialName) != materials.end()) {
                const Material& mat = materials[face.materialName];

                // Disable color material temporarily to set materials
                glDisable(GL_COLOR_MATERIAL);

                // --- KODE YANG DIPERBAIKI (Mulai dari sini) ---
                // Menggunakan properti material yang terpisah (Ka, Kd, Ks, Ns)
                // yang sudah dibaca dari file .mtl
                GLfloat ambient[] = { mat.ambient.x, mat.ambient.y, mat.ambient.z, mat.transparency };
                GLfloat diffuse[] = { mat.diffuse.x, mat.diffuse.y, mat.diffuse.z, mat.transparency };
                GLfloat specular[] = { mat.specular.x, mat.specular.y, mat.specular.z, mat.transparency };

                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);   // <-- Sekarang menggunakan Ka
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);   // <-- Menggunakan Kd
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular); // <-- Menggunakan Ks
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess); // <-- Menggunakan Ns
                // --- KODE YANG DIPERBAIKI (Selesai) ---

                // Bind texture if available
                if (mat.textureID != 0) {
                    glBindTexture(GL_TEXTURE_2D, mat.textureID);
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, 0);
                }

                // Handle transparency
                if (mat.transparency < 1.0f) {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
                else {
                    glDisable(GL_BLEND);
                }
            }
        }

        // Draw the face
        if (face.vertexIndices.size() == 3) {
            glBegin(GL_TRIANGLES);
        }
        else if (face.vertexIndices.size() == 4) {
            glBegin(GL_QUADS);
        }
        else {
            glBegin(GL_POLYGON);
        }

        for (size_t i = 0; i < face.vertexIndices.size(); i++) {
            // Apply normal if available
            if (i < face.normalIndices.size()) {
                int nIdx = face.normalIndices[i];
                if (nIdx >= 0 && nIdx < normals.size()) {
                    glNormal3f(normals[nIdx].x, normals[nIdx].y, normals[nIdx].z);
                }
            }

            // Apply texture coordinate if available
            if (i < face.texCoordIndices.size()) {
                int tIdx = face.texCoordIndices[i];
                if (tIdx >= 0 && tIdx < texCoords.size()) {
                    glTexCoord2f(texCoords[tIdx].u, texCoords[tIdx].v);
                }
            }

            // Apply vertex
            int vIdx = face.vertexIndices[i];
            if (vIdx >= 0 && vIdx < vertices.size()) {
                glVertex3f(vertices[vIdx].x, vertices[vIdx].y, vertices[vIdx].z);
            }
        }

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix();
}