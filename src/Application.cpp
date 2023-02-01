#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <array>

#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Renderer.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <chrono>
#include <QMediaPlayer>
#include <QAudioOutput>

GLFWwindow* InitWindow(int width, int height)
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return nullptr;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(width, height, "space", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return nullptr;

    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return nullptr;
    }

    std::cout << "Using GL Version: " << glGetString(GL_VERSION) << std::endl;

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    return window;
}

struct Point
{
    Point(double x = 0.0, double y = 0.0, double z = 0.0) : mData({x, y, z}){}

    const double& operator[](int i) const
    {
        return mData[i];
    }

    double& operator[](int i)
    {
        return mData[i];
    }

private:
    std::vector<double> mData;
};

std::vector<double> createSphereVertices(int horiz, int vert, const Point &center, double radius)
{
    // vertex buffer
    const int verticesSize = 8 * vert * (horiz + 2);
    std::vector<double> vertices(verticesSize);
    for (int yy = 0; yy <= horiz + 1; ++yy) {
        double y = cos(yy * M_PI / (horiz + 1.0));
        double r = sqrt(1 - y * y);
        for (int rr = 0; rr < vert; ++rr) {
            double x = r * cos(2 * M_PI * rr / vert);
            double z = r * sin(2 * M_PI * rr / vert);
            const std::array<double, 3> pos{x, y, z};
            const int idx = (rr + yy * vert) * 8;
            // positions
            for (int i = 0; i < 3; i++)
            {
                vertices[idx + i] = (pos[i] * radius) + center[i];
            }

            // texels
            constexpr double Su = 1.0f;
            constexpr double Sv = 1.0f;

            constexpr double textureCenterU = 0.0f;
            constexpr double textureCenterV = 0.5f;

            const double dx = vertices[idx];
            const double dy = vertices[idx + 1];
            const double dz = vertices[idx + 2];
            const double len = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));

            const double u = (Su/(2*M_PI) * atan2(dx, dz) + textureCenterU);
            const double v = (Sv/M_PI * tan(dy / len)) + textureCenterV;

            vertices[idx + 3] = u;
            vertices[idx + 4] = v;

            // normals
            auto nx = center[0] - dx;
            auto ny = center[1] - dy;
            auto nz = center[2] - dz;
            const auto normalLen = sqrt(nx*nx + ny*ny + nz*nz);
            nx /= normalLen;
            ny /= normalLen;
            nz /= normalLen;
            assert(sqrt(nx*nx + ny*ny + nz*nz) <= 1.01);
            vertices[idx + 5] = nx;
            vertices[idx + 6] = ny;
            vertices[idx + 7] = nz;
        }
    }
    
    return std::move(vertices);
}

std::vector<unsigned int> createIndices(int horiz, int vert)
{
    // index buffer
    const int indicesSize = 3 * 2 * vert * horiz;
    std::vector<unsigned int> indices(indicesSize);
    for (int yy = 0; yy < horiz; ++yy)
    {
        for (int rr = 0; rr < vert; ++rr) {
            const int idx = (rr + 2 * yy * vert) * 3;
            indices[idx] = (rr + 1) % vert
                           + yy * vert;
            indices[idx + 1] = rr + vert
                               + yy * vert;
            indices[idx + 2] = (rr + 1) % vert + vert + yy * vert;

            const int idx2 = (rr + vert + 2 * yy * vert) * 3;
            indices[idx2] = rr + vert
                            + yy * vert;
            indices[idx2 + 1] = rr + 2 * vert
                                + yy * vert;
            indices[idx2 + 2] = (rr + 1) % vert + vert + yy * vert;
        }
    }
    
    return std::move(indices);
}

int main()
{
    QMediaPlayer player;
    QAudioOutput audioOutput; // chooses the default audio routing
    player.setAudioOutput(&audioOutput);
    player.setSource(QUrl("res/music/interstellar.mp3"));
    player.play();

    constexpr int kWidth = 1500.0;
    constexpr int kHeight = 960.0;

    GLFWwindow* window = InitWindow(kWidth, kHeight);
    if (!window)
        return -1;

    Point center;
//    const auto [springVertices, springIndices] = createCylinder(1000, 1000, center, 110.0, 500.0);

    constexpr int kHorizSphere = 500;
    constexpr int kVertSphere = 500;
    const auto sphereVertices = createSphereVertices(kHorizSphere, kVertSphere, center, 450);
    const auto sphereIndices = createIndices(kHorizSphere, kVertSphere);
    constexpr int kSpringRingSegmentCount= 12;
    constexpr int kSpringRingCount = 5000;
    const auto springIndices = createIndices(kSpringRingCount - 2, kSpringRingSegmentCount);

    const std::vector<double> starsVertices {-4000.0, 2000.0, 1000.0, 0.0, 0.0,
                                           -4000.0, -2000.0, 1000.0, 0.0, 1.0,
                                           4000.0, 2000.0, 1000.0, 1.0, 0.0,
                                           4000.0, -2000.0, 1000.0, 1.0, 1.0};

    const std::vector<unsigned> starsIndices {0, 1, 2, 3, 2, 1};

    GLCall( glEnable(GL_BLEND) );
    GLCall( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

    {
        VertexArray sphereVa;
        VertexBuffer sphereVb(sphereVertices.data(), sphereVertices.size() * sizeof(double));
        IndexBuffer ib(sphereIndices.data(), sphereIndices.size());

        VertexArray starsVa;
        VertexBuffer starsVb(starsVertices.data(), starsVertices.size() * sizeof(double));
        IndexBuffer starsIb(starsIndices.data(), starsIndices.size());

        // projection matrix
//        glm::mat4 proj = glm::ortho(0.0, kWidth, 0.0, kHeight, -1000.0, 3000.0);
        glm::mat4 proj = glm::perspective(45.0f, (float)kWidth/(float)kHeight, 500.0f, 30000.0f);

        // view matrix
        glm::mat4 ident = glm::mat4(1.0);
        glm::vec3 trvec = glm::vec3(0, 0, -3500);
        glm::mat4 view = glm::translate(ident, trvec);

        VertexBufferLayout layoutWithTexture;
        layoutWithTexture.AddDouble(3);
        layoutWithTexture.AddDouble(2);
        layoutWithTexture.AddDouble(3);
        
        sphereVa.AddBuffer(sphereVb, layoutWithTexture);
        starsVa.AddBuffer(starsVb, layoutWithTexture);

        Shader shader("res/shaders/Planet.shader");
        shader.SetUniform3f("lightColor", 1.0, 1.0, 1.0);
        shader.SetUniform3f("lightPos", 1.0, 1.0, 1000.0);
        shader.Bind();

        Shader sunShader("res/shaders/Sun.shader");
        sunShader.Bind();

        Texture sunTexture("res/textures/sun.jpg");
        Texture mercuryTexture("res/textures/mercury.jpg");
        Texture venusTexture("res/textures/venus.jpg");
        Texture earthTexture("res/textures/earth.jpg");
        Texture marsTexture("res/textures/mars.jpg");
        Texture jupiterTexture("res/textures/jupiter.jpg");
        Texture saturnTexture("res/textures/saturn.jpg");
        Texture uranusTexture("res/textures/uranus.jpg");
        Texture neptuneTexture("res/textures/neptune.jpg");
        Texture plutonTexture("res/textures/pluton.jpg");

        Texture starsTexture("res/textures/stars.jpg");


        double red = 0.0;
        double step = 0.05;

        Renderer renderer;

        glm::vec3 sunTranslation(0, 0, 0);
        glm::vec3 mercuryTranslation(600, 300, 0);
        glm::vec3 venusTranslation(900, 600, 0);
        glm::vec3 earthTranslation(1300, -300, 0);
        glm::vec3 marsTranslation(1700, 900, 0);
        glm::vec3 jupiterTranslation(2300, 450, 0);
        glm::vec3 saturnTranslation(3100, -150, 0);
        glm::vec3 uranusTranslation(3900, -50, 0);
        glm::vec3 neptuneTranslation(4600, -750, 0);

        glm::vec3 sunScale(1.1, 1, 1);
        glm::vec3 mercuryScale(0.12, 0.1, 0.1);
        glm::vec3 venusScale(0.34, 0.3, 0.3);
        glm::vec3 earthScale(0.34, 0.3, 0.3);
        glm::vec3 marsScale(0.23, 0.2, 0.2);
        glm::vec3 jupiterScale(0.78, 0.7, 0.7);
        glm::vec3 saturnScale(0.67, 0.6, 0.6);
        glm::vec3 uranusScale(0.45, 0.4, 0.4);
        glm::vec3 neptuneScale(0.34, 0.3, 0.3);

        const std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

        glm::vec3 rotateVector(0.0, 1.0, 0.0);

        do {
            std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsedSeconds = t - start;
            const double time = elapsedSeconds.count();

            renderer.Clear();

            {
                // STARS
                starsTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                glm::mat4 mvp = proj * view;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                const auto model = glm::mat4(1.0);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(starsVa, starsIb, shader);
            }
            glClear(GL_DEPTH_BUFFER_BIT);

            {
                // SUN
                sunTexture.Bind();
                sunShader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = 0.03f * time;
                glm::mat4 model = glm::translate(glm::mat4(1.0), sunTranslation) *
                        glm::scale(glm::mat4(1.0), sunScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                sunShader.Bind();
                sunShader.SetUniformMat4f("u_MVP", mvp);
                renderer.Draw(sphereVa, ib, sunShader);
            }

            {
                // Mercury
                mercuryTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = 0.1f * time;
                const float sunRotationSpeed = 0.01f * time;
                glm::vec3 sunRotateVector(-mercuryTranslation.y, mercuryTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), mercuryTranslation) *
                        glm::scale(glm::mat4(1.0), mercuryScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            {
                // Venus
                venusTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = -0.09f * time;
                const float sunRotationSpeed = -0.03f * time;
                glm::vec3 sunRotateVector(-venusTranslation.y, venusTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), venusTranslation) *
                        glm::scale(glm::mat4(1.0), venusScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            {
                // the Earth
                earthTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = 0.05f * time;
                const float sunRotationSpeed = 0.04f * time;
                glm::vec3 sunRotateVector(-earthTranslation.y, earthTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), earthTranslation) *
                        glm::scale(glm::mat4(1.0), earthScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            {
                // Mars
                marsTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = -0.06f * time;
                const float sunRotationSpeed = 0.02f * time;
                glm::vec3 sunRotateVector(-marsTranslation.y, marsTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), marsTranslation) *
                        glm::scale(glm::mat4(1.0), marsScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            {
                // Jupiter
                jupiterTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = 0.07f * time;
                const float sunRotationSpeed = -0.03f * time;
                glm::vec3 sunRotateVector(-jupiterTranslation.y, jupiterTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), jupiterTranslation) *
                        glm::scale(glm::mat4(1.0), jupiterScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            {
                // Saturn
                saturnTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = -0.12f * time;
                const float sunRotationSpeed = 0.01f * time;
                glm::vec3 sunRotateVector(-saturnTranslation.y, saturnTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), saturnTranslation) *
                        glm::scale(glm::mat4(1.0), saturnScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            {
                // Uranus
                uranusTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = -0.1f * time;
                const float sunRotationSpeed = 0.02f * time;
                glm::vec3 sunRotateVector(-uranusTranslation.y, uranusTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), uranusTranslation) *
                        glm::scale(glm::mat4(1.0), uranusScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            {
                // Neptune
                neptuneTexture.Bind();
                shader.SetUniform1i("u_Texture", 0);
                const float selfRotationSpeed = 0.08f * time;
                const float sunRotationSpeed = -0.04f * time;
                glm::vec3 sunRotateVector(-neptuneTranslation.y, neptuneTranslation.x, 0);
                glm::mat4 model = glm::rotate(glm::mat4(1.0), sunRotationSpeed, sunRotateVector) *
                        glm::translate(glm::mat4(1.0), neptuneTranslation) *
                        glm::scale(glm::mat4(1.0), neptuneScale) *
                        glm::rotate(glm::mat4(1.0), selfRotationSpeed, rotateVector);
                glm::mat4 mvp = proj * view * model;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniformMat4f("model", model);
                renderer.Draw(sphereVa, ib, shader);
            }

            // Swap buffers
            glfwSwapBuffers(window);
            glfwPollEvents();

            // increment red
            if (red < 0.0 || red > 1.0)
                step *= -1.0;
            red += step;

        } // Check if the ESC key was pressed or the window was closed
        while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
                glfwWindowShouldClose(window) == 0 );
    }

    player.stop();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

