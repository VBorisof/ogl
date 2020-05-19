#ifndef MESH_HPP
#define MESH_HPP

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "common/objloader.hpp"
#include "common/vboindexer.hpp"
#include "LoadException.hpp"

class Mesh {
private:
    GLuint _vertexArrayId;
    GLuint _vertexBuffer;
    GLuint _uvBuffer;
    GLuint _normalBuffer;
    GLuint _indexBuffer;
    
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec2> _uvs;
    std::vector<glm::vec3> _normals;
    
    std::vector<unsigned short> _indices;

public:
    
    void draw() {
        glBindVertexArray(_vertexArrayId);
        
        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*) nullptr
        );
        
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, _uvBuffer);
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*) nullptr
        );
        
        // 3rd attribute buffer : Normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
        glVertexAttribPointer(
            2,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*) nullptr
        );
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
        
        glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_SHORT, (void*) 0);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    
    Mesh(const char* meshPath) {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        
        bool isModelLoaded = loadOBJ(meshPath, vertices, uvs, normals);
        if (!isModelLoaded) {
            throw LoadException("Failed to load OBJ model.");
        }
        
        indexVBO(vertices, uvs, normals, _indices, _vertices, _uvs, _normals);
        
        glGenVertexArrays(1, &_vertexArrayId);
        glBindVertexArray(_vertexArrayId);
        
        glGenBuffers(1, &_vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(glm::vec3), &_vertices[0], GL_STATIC_DRAW);
        
        glGenBuffers(1, &_uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, _uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, _uvs.size() * sizeof(glm::vec2), &_uvs[0], GL_STATIC_DRAW);
        
        glGenBuffers(1, &_normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(glm::vec3), &_normals[0], GL_STATIC_DRAW);
        
        glGenBuffers(1, &_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned short), &_indices[0], GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    ~Mesh() {
        glDeleteBuffers(1, &_vertexBuffer);
        glDeleteBuffers(1, &_uvBuffer);
        glDeleteBuffers(1, &_normalBuffer);
        glDeleteBuffers(1, &_indexBuffer);
        glDeleteVertexArrays(1, &_vertexArrayId);
    }
};

#endif//MESH_HPP
