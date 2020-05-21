#include <cstdio>
#include <iostream>
#include <vector>
#include <exception>
#include <map>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "common/shader.hpp"
#include "common/controls.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "common/vboindexer.hpp"

#include <ft2build.h>
#include <glm/gtc/matrix_transform.hpp>
#include FT_FREETYPE_H

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "FontTextureManager.hpp"

using namespace glm;

GLFWwindow* window;

void APIENTRY glDebugOutput(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char *message,
    const void *userParam
)
{
    // Ignore non-significant error/warning codes
    if (
        id == 131169
        || id == 131185
        || id == 131218
        || id == 131204
    )
    {
        return;
    }
    
    std::string color = "\033[0;37m";
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         color = "\033[0;31m"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       color = "\033[1;33m"; break;
        case GL_DEBUG_SEVERITY_LOW:          color = "\033[0;33m"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: color = "\033[0;37m"; break;
    }
    
    std::cout << color;
    
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "[!]"; break; // Type: Error
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "[x]"; break; // Type: Deprecated Behaviour
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "[?]"; break; // Type: Undefined Behaviour
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "[>]"; break; // Type: Portability
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "[=]"; break; // Type: Performance
        case GL_DEBUG_TYPE_MARKER:              std::cout << "[M]"; break; // Type: Marker
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "[V]"; break; // Type: Push Group
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "[^]"; break; // Type: Pop Group
        case GL_DEBUG_TYPE_OTHER:               std::cout << "[~]"; break; // Type: Other
    }
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "[API       ]"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "[W.System  ]"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "[S.Compiler]"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "[3-Party   ]"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "[App       ]"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "[Other     ]"; break;
    }
    
    std::cout << " " << message << "\033[0m" << std::endl;
}

int init(int width, int height) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    window = glfwCreateWindow(width, height, "GL Playground", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    
    
    return 0;
}

int main() {
    int screen_width = 1920;
    int screen_height = 1080;
    
    int initResult = init(screen_width, screen_height);
    if (initResult != 0) {
        return initResult;
    }

    /* ================================================ */

    // Load meshes
    Mesh* cubeMesh = nullptr;
    try {
        cubeMesh = new Mesh("/home/oma/Code/CPP-Workspace/ogl/playground/res/cube.obj");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[MESH LOAD ERROR]: %s", e.what());
        return -1;
    }

    Mesh* floorMesh = nullptr;
    try {
        floorMesh = new Mesh("/home/oma/Code/CPP-Workspace/ogl/playground/res/floor.obj");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[MESH LOAD ERROR]: %s", e.what());
        return -1;
    }


    // Load textures
    Texture* cubeTexture = nullptr;
    try {
        cubeTexture = new Texture("/home/oma/Code/CPP-Workspace/ogl/playground/res/tile.bmp");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[TEXTURE LOAD ERROR]: %s", e.what());
        return -1;
    }

    Texture* floorTexture = nullptr;
    try {
        floorTexture = new Texture("/home/oma/Code/CPP-Workspace/ogl/playground/res/tile_bw.bmp");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[TEXTURE LOAD ERROR]: %s", e.what());
        return -1;
    }

    // Create models
    
    auto floorModel = new Model(floorMesh, floorTexture);
    
    auto models = std::vector<Model*>();
    
    models.push_back(floorModel);
    
    for (int i = 0; i < 20; ++i) {
        auto cubeModel = new Model(cubeMesh, cubeTexture);
        cubeModel->setPosition(glm::vec3(-30.0f + i * 3, 1.0f, -1.0f));
        models.push_back(cubeModel);
    }
    
    
    Shader* sceneShader = new Shader(
        "/home/oma/Code/CPP-Workspace/ogl/playground/vertex-shader.glsl",
        "/home/oma/Code/CPP-Workspace/ogl/playground/fragment-shader.glsl"
    );
    GLuint textureSampler = glGetUniformLocation(sceneShader->getId(), "myTextureSampler");
    GLuint mvpMatrixID = glGetUniformLocation(sceneShader->getId(), "MVP");
    GLuint viewMatrixID = glGetUniformLocation(sceneShader->getId(), "V");
    GLuint modelMatrixID = glGetUniformLocation(sceneShader->getId(), "M");

    GLuint lightId = glGetUniformLocation(sceneShader->getId(), "LightPosition_worldSpace");
    GLuint lightColorId = glGetUniformLocation(sceneShader->getId(), "LightColor");
    vec3 lightPosition(0, 5, 0);

    /* ================================================ */
    
    FontTextureManager* fontTextureManager = new FontTextureManager(
        "/home/oma/Code/CPP-Workspace/ogl/playground/res/fonts/arial.ttf"
    );
    
    Shader* textShader = new Shader(
        "/home/oma/Code/CPP-Workspace/ogl/playground/text-vertex-shader.glsl",
        "/home/oma/Code/CPP-Workspace/ogl/playground/text-fragment-shader.glsl"
    );
    glUseProgram(textShader->getId());
    glm::mat4 textProjectionMat = glm::ortho(0.0f, static_cast<float> (screen_width), 0.0f, static_cast<float> (screen_height));
    glUniformMatrix4fv(glGetUniformLocation(textShader->getId(), "projection"), 1, GL_FALSE, &textProjectionMat[0][0]);
    
    /* ================================================ */
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    
    int fps = -1;
    double lightTimeCounter = 0.0;
    double fpsTimeCounter = 0.0;
    double lastTime = glfwGetTime();
    do {
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);

        /* ==== UPDATE =================================== */

        computeMatricesFromInputs(deltaTime);
        mat4 projection = getProjectionMatrix();
        mat4 view = getViewMatrix();
        
        /* =============================================== */


        /* ==== DRAW ===================================== */

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        sceneShader->use();
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &view[0][0]);
    
        lightTimeCounter += deltaTime;
        lightPosition.x = 3 * cos(lightTimeCounter * 2);
        lightPosition.z = 3 * sin(lightTimeCounter * 2);
        //lightPosition.x = 30 * sin(lightTimeCounter * 0.8);
        glUniform3f(lightId, lightPosition.x, lightPosition.y, lightPosition.z);

        vec3 lightColor(1, 1, 1);
        glUniform3f(lightColorId, lightColor.r, lightColor.g, lightColor.b);


        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(textureSampler, 0);

        for (Model* m : models) {
            m->update();
            mat4 model = m->getModelMatrix();
            mat4 mvp = projection * view * model;
            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &model[0][0]);
            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
            m->draw();
        }
    
        
        /* ============================================== */
        
        fpsTimeCounter += deltaTime;
        if (fpsTimeCounter > 1) {
            fps = static_cast<int>(1.0 / deltaTime);
            fpsTimeCounter = 0.0;
        }
        
        fontTextureManager->renderText(
            textShader,
            "FPS : " + std::to_string(fps),
            glm::vec2(10.0f, static_cast<float> (screen_height) - 50.0f),
            0.5f,
            glm::vec3(0.0f, 1.0f, 1.0f)
        );
        
        /* ============================================== */
        
        
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << "[GL][!] " << gluErrorString(error) << " (" << error << ")" << std::endl;
        }

        glfwSwapBuffers(window);

        /* ============================================== */
        
        glfwPollEvents();

        // For the next frame, the "last time" will be "now"
        lastTime = currentTime;
    }
    while (
        glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
        && glfwWindowShouldClose(window) == 0
    );


    // == CLEANUP ===================================== //

    for (Model* m : models) {
        delete m;
    }
    delete cubeMesh;
    delete cubeTexture;

    delete floorMesh;
    delete floorTexture;
    
    delete sceneShader;
    delete textShader;
    
    delete fontTextureManager;
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

