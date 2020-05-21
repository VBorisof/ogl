#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureId;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    FT_Pos       Advance;
};

#endif//CHARACTER_HPP
