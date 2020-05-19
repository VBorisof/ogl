#ifndef MODEL_HPP
#define MODEL_HPP

#include "glm/glm.hpp"

#include "Mesh.hpp"
#include "Texture.hpp"

class Model {
private:
    Mesh* _mesh;
    Texture* _texture;
    
    glm::mat4 _modelMatrix = glm::mat4(1.0f);
    glm::vec3 _position = glm::vec3(0.0f, 0.0f, 0.0f);

public:
    
    glm::mat4 getModelMatrix() const {
        return _modelMatrix;
    }
    
    void setPosition(glm::vec3 position) {
        _position = position;
    }
    
    void update() {
        _modelMatrix[3] = glm::vec4(_position, 1.0f);
    }
    
    void draw() {
        _texture->bind();
        _mesh->draw();
    }
    
    
    Model(Mesh* mesh, Texture* texture) {
        _mesh = mesh;
        _texture = texture;
    }
};

#endif//MODEL_HPP
