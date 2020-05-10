#include <cstdio>
#include <iostream>
#include <vector>
#include <exception>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

#include "common/shader.hpp"
#include "common/controls.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"

using namespace glm;

GLFWwindow* window;

class LoadException : public std::exception {
private:
    const char* _message;

public:
    LoadException(const char* message) {
        _message = message;
    }

    virtual const char* what() {
        return _message;
    }
};

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

class Mesh {
private:
    GLuint _vertexArrayId;
    GLuint _vertexBuffer;
    GLuint _uvBuffer;
    GLuint _normalBuffer;

    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec2> _uvs;
    std::vector<glm::vec3> _normals;


public:

    void draw() {
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

        glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }


    Mesh(const char* meshPath) {
        bool isModelLoaded = loadOBJ(meshPath, _vertices, _uvs, _normals);
        if (!isModelLoaded) {
            throw LoadException("Failed to load OBJ model.");
        }

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
    }

    ~Mesh() {
        glDeleteBuffers(1, &_vertexBuffer);
        glDeleteBuffers(1, &_uvBuffer);
        glDeleteBuffers(1, &_normalBuffer);
        glDeleteVertexArrays(1, &_vertexArrayId);
    }
};

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

int init() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 1024;
    int height = 768;

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

    return 0;
}

int main() {

    int initResult = init();
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
    

    GLuint shaderProgramID = LoadShaders(
        "/home/oma/Code/CPP-Workspace/ogl/playground/vertex-shader.glsl",
        "/home/oma/Code/CPP-Workspace/ogl/playground/fragment-shader.glsl"
    );
    GLuint textureSampler  = glGetUniformLocation(shaderProgramID, "myTextureSampler");
    GLuint mvpMatrixID = glGetUniformLocation(shaderProgramID, "MVP");
    GLuint viewMatrixID = glGetUniformLocation(shaderProgramID, "V");
    GLuint modelMatrixID = glGetUniformLocation(shaderProgramID, "M");

    GLuint lightId = glGetUniformLocation(shaderProgramID, "LightPosition_worldSpace");
    GLuint lightColorId = glGetUniformLocation(shaderProgramID, "LightColor");
    vec3 lightPosition(0, 5, 0);
    double lightTimeCounter = 0;

    /* ================================================ */

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


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

        glUseProgram(shaderProgramID);
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
    
    glDeleteProgram(shaderProgramID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

