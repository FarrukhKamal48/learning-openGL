#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "src/scenes/Verlet.scene.h"
#include "src/scenes/Batching.scene.h"
#include "src/scenes/Instancing.h"

int main (int argc, char *argv[])
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewInit();

    Scene::Verlet activeScene;
    activeScene.Start();

    double lastTime = glfwGetTime();
    double deltaTime = 0.016;
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        lastTime = glfwGetTime();
        if (lastTime < 0) continue;
        
        activeScene.Update(deltaTime);
        activeScene.Render();

        deltaTime = glfwGetTime() - lastTime;
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // delete activeScene;

    glfwTerminate();
    return 0;
}
