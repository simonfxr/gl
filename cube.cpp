#include <iostream>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include <GLBatch.h>

#include "defs.h"
#include "GameLoop.hpp"
#include "gltools.hpp"
#include "Batch.hpp"
#include "ShaderProgram.hpp"

#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/EulerAngles.hpp"

namespace {

void makeUnitCube(gltools::Batch& cube);

struct Game : public GameLoop::Game {

    sf::RenderWindow window;
    sf::Clock clock;
    GameLoop loop;

    gltools::Batch cubeBatch;
    ShaderProgram cubeShader;
    EulerAngles cubeOrientation;
    GLuint uniform_mvp_location;

    Game(const sf::VideoMode& vm, const std::string& title, const sf::ContextSettings& cs);
    ~Game();

    void handleEvents();
    void tick();
    void render(float interpolation);
    float now();
    
};

Game::Game(const sf::VideoMode& vm, const std::string& title, const sf::ContextSettings& cs) :
    window(vm, title, sf::Style::Default, cs),
    loop(50, 0, 60),
    cubeBatch(GL_QUADS)
{
    window.SetActive();

    makeUnitCube(cubeBatch);

    gltools::printErrors(std::cerr);

    cubeShader.compileVertexShaderFromFile("cube.vert");
    cubeShader.compileFragmentShaderFromFile("cube.frag");
    cubeShader.bindAttribute("vertex", gltools::Batch::VertexPos);
//    cubeShader.bindAttribute("vertexColor", gltools::Batch::ColorPos);
    if (!cubeShader.link()) {
        std::cerr << "couldnt link shader" << std::endl;
        loop.exit(1);
    }

    glValidateProgram(cubeShader.program);
    ShaderProgram::printProgramLog(cubeShader.program, std::cerr);

    uniform_mvp_location = glGetUniformLocation(cubeShader.program, "mvp");

    gltools::printErrors(std::cerr);
}

Game::~Game() {
    std::cerr << "exiting game after " << loop.realTime() << " seconds" << std::endl;
}

float Game::now() {
    return clock.GetElapsedTime();
}

void Game::handleEvents() {

    sf::Event e;

    while (window.GetEvent(e)) {
        switch (e.Type) {
        case sf::Event::Closed:
            loop.exit();
            break;
        case sf::Event::Resized:
            glViewport(0, 0, e.Size.Width, e.Size.Height);
            break;
        }
    }
}

void Game::tick() {
    cubeOrientation.heading += 0.012f;
    cubeOrientation.pitch += 0.006f;
    cubeOrientation.bank += 0.009f;
    cubeOrientation.canonize();
}

void Game::render(float interpolation) {

    UNUSED(interpolation);

    window.SetActive();

    glClearColor(0.f, 0.f, 0.f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GL_CHECK(glUseProgram(cubeShader.program));

    mat4 rot = cubeOrientation.getRotationMatrix().transpose();
    glUniformMatrix4fv(uniform_mvp_location, 1, GL_FALSE, rot.flat);

    cubeBatch.draw();

    window.Display();

    gltools::printErrors(std::cerr);
}

void makeUnitCube(gltools::Batch& cube) {

    cube.vertex(vec3(-1.0f, -1.0f,  1.0f));
    cube.vertex(vec3( 1.0f, -1.0f,  1.0f));
    cube.vertex(vec3( 1.0f,  1.0f,  1.0f));
    cube.vertex(vec3(-1.0f,  1.0f,  1.0f));

    cube.vertex(vec3(-1.0f, -1.0f, -1.0f));
    cube.vertex(vec3(-1.0f,  1.0f, -1.0f));
    cube.vertex(vec3( 1.0f,  1.0f, -1.0f));
    cube.vertex(vec3( 1.0f, -1.0f, -1.0f));

    cube.vertex(vec3(-1.0f,  1.0f, -1.0f));
    cube.vertex(vec3(-1.0f,  1.0f,  1.0f));
    cube.vertex(vec3( 1.0f,  1.0f,  1.0f));
    cube.vertex(vec3( 1.0f,  1.0f, -1.0f));

    cube.vertex(vec3(-1.0f, -1.0f, -1.0f));
    cube.vertex(vec3( 1.0f, -1.0f, -1.0f));
    cube.vertex(vec3( 1.0f, -1.0f,  1.0f));
    cube.vertex(vec3(-1.0f, -1.0f,  1.0f));

    cube.vertex(vec3( 1.0f, -1.0f, -1.0f));
    cube.vertex(vec3( 1.0f,  1.0f, -1.0f));
    cube.vertex(vec3( 1.0f,  1.0f,  1.0f));
    cube.vertex(vec3( 1.0f, -1.0f,  1.0f));

    cube.vertex(vec3(-1.0f, -1.0f, -1.0f));
    cube.vertex(vec3(-1.0f, -1.0f,  1.0f));
    cube.vertex(vec3(-1.0f,  1.0f,  1.0f));
    cube.vertex(vec3(-1.0f,  1.0f, -1.0f));

    cube.freeze();
}

} // namespace anon

int main(int argc, char *argv[]) {

    UNUSED(argc);
    UNUSED(argv);

    sf::Clock startupClock;
    float t0 = startupClock.GetElapsedTime();

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    sf::ContextSettings cs;
    cs.MajorVersion = 3;
    cs.MinorVersion = 3;
    Game game(sf::VideoMode(800, 600), "cube", cs);

    float t1 = startupClock.GetElapsedTime() - t0;

    std::cerr << "startup time: " << (t1 * 1000.f) << " ms" << std::endl;
        
    return game.loop.run(game);
}
