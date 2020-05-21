#ifndef FONTTEXTUREMANAGER_HPP
#define FONTTEXTUREMANAGER_HPP

#include <map>

#include "GL/glew.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Character.hpp"
#include "LoadException.hpp"
#include "Shader.hpp"

class FontTextureManager {
private:
    std::map<char, Character> _characters;
    GLuint _textVao;
    GLuint _textVbo;
    
    std::map<char, Character> loadChars(FT_Face face) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        std::map<char, Character> characters;
        for (unsigned char c = 0; c < 128; ++c){
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)){
                fprintf(stderr, "Failed to load Glyph.");
                continue;
            }
            
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
            };
            
            characters.insert(std::pair<char, Character>(c, character));
        }
        
        return characters;
    }
    
public:
    
    void renderText(
        Shader const* shader,
        std::string text,
        glm::vec2 position,
        float scale,
        glm::vec3 color
    )
    {
        shader->use();
        
        glActiveTexture(GL_TEXTURE0);
        
        glBindVertexArray(_textVao);
        glBindBuffer(GL_ARRAY_BUFFER, _textVbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            4,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(float),
            (void*) nullptr
        );
        
        glUniform1i(glGetUniformLocation(shader->getId(), "text"), 0);
        glUniform3f(glGetUniformLocation(shader->getId(), "textColor"), color.x, color.y, color.z);
        
        std::string::const_iterator it;
        float x = position.x;
        float y = position.y;
        for (it = text.begin(); it != text.end(); ++it){
            Character c = _characters[*it];
            
            float xpos = x + c.Bearing.x * scale;
            float ypos = y - (c.Size.y - c.Bearing.y) * scale;
            
            float w = c.Size.x * scale;
            float h = c.Size.y * scale;
            
            float vertices[6][4] = {
                { xpos,     ypos + h, 0.0f, 0.0f },
                { xpos,     ypos,     0.0f, 1.0f },
                { xpos + w, ypos,     1.0f, 1.0f },
                
                { xpos    , ypos + h, 0.0f, 0.0f },
                { xpos + w, ypos    , 1.0f, 1.0f },
                { xpos + w, ypos + h, 1.0f, 0.0f }
            };
            
            glBindTexture(GL_TEXTURE_2D, c.TextureId);
            
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (c.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    
    FontTextureManager(const char* filePath) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            throw LoadException("Could not init FreeType library.");
        }
    
        FT_Face face;
        if (FT_New_Face(ft, filePath, 0, &face)) {
            throw LoadException("Could not load the font.");
        }
    
        FT_Set_Pixel_Sizes(face, 0, 48);
    
        _characters = loadChars(face);
    
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    
    
        glGenVertexArrays(1, &_textVao);
        glBindVertexArray(_textVao);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
        glGenBuffers(1, &_textVbo);
        glBindBuffer(GL_ARRAY_BUFFER, _textVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};

#endif//FONTTEXTUREMANAGER_HPP
