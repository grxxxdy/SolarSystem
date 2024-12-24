#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>

#include "Shader.h"
#include "Camera.h"
#include "Figures.h"
#include "Planet.h"
#include "Skybox.h"

#define PI 3.14159265358979323846

void processInput(GLFWwindow* window, glm::mat4* projection, float& deltaTime, float currentFrame);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

bool spaceKeyPressed = false, pKeyPressed = false;
float lastMouseX = 400, lastMouseY = 300;
bool firstMouseMovement = true;
bool visibleOrbits = true;

bool pause = false;
float pausedTime = 0.0f;
float pauseStartTime = 0.0f;

// Creating a camera
Camera camera(
    5.0f,                              // speed
    7.0f,                               // zoom
    glm::vec3(0.0f, 7.0f, 0.0f),        // pos
    glm::vec3(0.0f, 0.0f, 0.0f),        // target
    glm::vec3(0.0f, 0.0f, -1.0f),       // front
    glm::vec3(0.0f, 1.0f, 0.0f),        // up
    glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f));    // projection   

int main(void)
{
    GLFWwindow* window;

    // Initializing the library
    if (!glfwInit())
    {
        std::cout << "Failed to initialise GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Creating a windowed mode window and its OpenGL context
    window = glfwCreateWindow(800, 600, "Lab 3", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialising glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Adjusting screen size and it's resizing
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Skybox
    Skybox skybox(std::vector<std::string>{
        "Textures/Skybox/right.jpg",
            "Textures/Skybox/left.jpg",
            "Textures/Skybox/top.jpg",
            "Textures/Skybox/bottom.jpg",
            "Textures/Skybox/front.jpg",
            "Textures/Skybox/back.jpg"}, 
        "ShaderData/Skybox/skybox_vertex.txt",
        "ShaderData/Skybox/skybox_fragment.txt");

    // Creating planets
    Planet sun(5.0f, 36, 18, 0.0f, 0.0f, glm::radians(10.0f), "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Sun/sun.jpg", nullptr, nullptr, 0, true);
    Planet mercury(0.19f, 36, 18, 7.2f, 1.05f, 0.00071f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Mercury/mercury.jpg");
    Planet venus(0.48f, 36, 18, 9.5f, 0.62f, 0.00017f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Venus/venus_surface.jpg", "Textures/Venus/venus_atmosphere.jpg");
    Planet earth(0.50f, 36, 18, 12.0f, 0.2f, 0.04167f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Earth/earth_surface.jpg", "Textures/Earth/earth_clouds.jpg");
    Planet mars(0.27f, 36, 18, 14.2f, 0.43f, 0.04060f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Mars/mars.jpg");
    Planet jupiter(3.0f, 36, 18, 19.5f, 0.08f, 0.1f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Jupiter/jupiter.jpg");
    Planet saturn(2.5f, 36, 18, 27.5f, 0.03f, 0.209260f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Saturn/saturn.jpg", nullptr, "Textures/Saturn/saturn_ring.png", 20);
    Planet uranus(1.5f, 36, 18, 35.0f, 0.1f, 0.05818f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Uranus/uranus.jpg");
    Planet neptune(1.4f, 36, 18, 39.0f, 0.006f, 0.06192f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Neptune/neptune.jpg");
    // Yeah yeah it's not a planet
    Planet pluto(0.1f, 36, 18, 41.5f, 0.004f, 0.00063f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Pluto/pluto.jpg");
    // Earth moon
    Planet moon(0.19f, 36, 18, 12.0f, 0.2f, 0.00071f, "ShaderData/Planets/vertex_shader.txt", "ShaderData/Planets/fragment_shader.txt", "Textures/Earth/moon.jpg");

    // Lights
    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 0.8f);

    // Some additional stuff before render starts
    float deltaTime = 0.0f, lastFrame = 0.0f;

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Keeping track of deltaTime
        float currentFrame = glfwGetTime();
        if (!pause)
        {
            deltaTime = currentFrame - lastFrame - pausedTime;
            lastFrame = currentFrame - pausedTime;
        }

        // Processing input
        processInput(window, &camera.projection, deltaTime, currentFrame);

        // Update camera
        camera.updateView();

        // Update planets
        if (!pause)
        {
            sun.updatePos(currentFrame - pausedTime);
            mercury.updatePos(currentFrame - pausedTime);
            venus.updatePos(currentFrame - pausedTime);
            earth.updatePos(currentFrame - pausedTime);
            moon.updatePos(currentFrame - pausedTime, earth.getDistanceFromSun());
            mars.updatePos(currentFrame - pausedTime);
            jupiter.updatePos(currentFrame - pausedTime);
            saturn.updatePos(currentFrame - pausedTime);
            uranus.updatePos(currentFrame - pausedTime);
            neptune.updatePos(currentFrame - pausedTime);
            pluto.updatePos(currentFrame - pausedTime);
        }

        // Render here 
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 0.68f, 0.79f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render planets
        sun.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, false);
        mercury.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        venus.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        earth.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        moon.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        mars.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        jupiter.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        saturn.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        uranus.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        neptune.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);
        pluto.render(camera.view, camera.projection, lightPos, lightColor, camera.cameraPos, visibleOrbits);

        // Skybox
        skybox.render(camera.view, camera.projection);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Cleanup
    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow* window, glm::mat4* projection, float& deltaTime, float currentFrame)
{
    // Close
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    // Move
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveUp(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveDown(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight(deltaTime);


    // Speed
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.speedUp();
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.speedDown();
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera.resetSpeed();
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        camera.resetPos();


    // Pause
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed)
    {
        pKeyPressed = true;
        pause = !pause;

        if (pause)
        {
            pauseStartTime = currentFrame;
        }
        else
        {
            pausedTime += (currentFrame - pauseStartTime);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
        pKeyPressed = false;


    // Enable/Disable sun orbits
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceKeyPressed)
    {
        visibleOrbits = !visibleOrbits;
        spaceKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        spaceKeyPressed = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
{
    if (firstMouseMovement)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouseMovement = false;
    }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.rotateCamera(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}