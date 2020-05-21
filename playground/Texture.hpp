#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h>
#include <common/texture.hpp>
#include "LoadException.hpp"

class Texture {
private:
    GLuint _textureId;

public:
    
    void bind() {
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureId);
    }
    
    
    Texture(const char* filePath) {
        _textureId = loadBMP_custom(filePath);
        
        if (_textureId == 0) {
            throw LoadException("Failed to load texture");
        }
    }
    
    ~Texture() {
        glDeleteTextures(1, &_textureId);
    }
};

#endif//TEXTURE_HPP