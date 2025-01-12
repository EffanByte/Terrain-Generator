#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <shader_m.h>
#include <iostream>
#include "noise.h"
#include <vector>

    // Callback to resize the viewport
    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }


    void initImGui(GLFWwindow* window) {
        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        // Set ImGui style
        ImGui::StyleColorsDark();
    }

    // ImGui variables
    int octaves = 4;
    float persistence = 0.5f;
    int width = 256;
    int height = 256;
    float scale = 50.0f;
    float frequency = 2.0f;

    bool terrainNeedsUpdate = false; //update whenever changes in noise function

    void renderImGuiMenu() {
        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a window
        ImGui::Begin("Settings Menu");

        // Store old values to detect changes
        float oldPersistence = persistence;
        int oldOctaves = octaves;
        int oldwidth = width;
        int oldheight = height;
        float oldfrequency = frequency;

        // Add sliders
        ImGui::SliderFloat("Persistence", &persistence, 0.0f, 1.0f);
        ImGui::SliderInt("Octaves", &octaves, 0, 10);
        ImGui::SliderInt("Width", &width, 0, 1000);
        ImGui::SliderInt("height", &height, 0, 1000);
        ImGui::SliderFloat("Frequency", &frequency, 0.0f, 8.0f);

        // Check if values changed
        if (oldPersistence != persistence || oldOctaves != octaves || oldwidth != width || oldheight != height || oldfrequency != frequency) {
            terrainNeedsUpdate = true;
        }


        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Camera variables
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Camera position
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera facing direction
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Camera up direction
    float yaw = -90.0f; // Initial yaw
    float pitch = 0.0f; // Initial pitch
    float lastX = 400, lastY = 300; // Mouse position
    bool firstMouse = true; // First mouse movement flag

    float deltaTime = 0.0f; // Time between frames
    float lastFrame = 0.0f; // Time of last frame

    // Keyboard input processing
    void processInput(GLFWwindow* window) {
        float cameraSpeed = 2.5f * deltaTime; // Adjust camera speed based on time

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    // Mouse callback for camera orientation
    void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f; // Mouse sensitivity
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Constrain pitch
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }

    int main() {
        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return -1;
        }

        // Set OpenGL version to 3.3 Core Profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a window
        GLFWwindow* window = glfwCreateWindow(800, 600, "Terrain Renderer", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        // Load OpenGL functions using GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        // Set the viewport and callback
        glViewport(0, 0, 800, 600);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        // Cube vertices
        float cubeVertices[] = {
            // positions
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,

            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f
        };


        // Create VAO and VBO
        unsigned int cubeVAO, cubeVBO;
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        unsigned int terrainVAO, terrainVBO, terrainEBO;


        float seed = glfwGetTime(); //for random generation everytime, can be replaced with a fixed value acting as seed, mostly for elevation


        TerrainData terrain = generateTerrain(width, height, scale, seed, octaves, persistence, frequency);


        glGenBuffers(1, &terrainVBO);
        glGenVertexArrays(1, &terrainVAO);
        glGenBuffers(1, &terrainEBO);
        glBindVertexArray(terrainVAO);

        // Load vertex data
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER,
            terrain.vertices.size() * sizeof(float),
            terrain.vertices.data(),
            GL_STATIC_DRAW);

        // Load index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            terrain.indices.size() * sizeof(unsigned int),
            terrain.indices.data(),
            GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // Shader setup (place the shaders in the same directory)
        Shader shader("shader.vs", "shader.fs");
        Shader noiseshader("noiseshader.vs", "noiseshader.fs");
        // Background color
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Register mouse callback
        glfwSetCursorPosCallback(window, mouse_callback);
       // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture the cursor

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        initImGui(window);

        // Render loop
        while (!glfwWindowShouldClose(window)) {



            // Calculate deltaTime
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Process input (camera movement)
            processInput(window);

            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use the shader and draw the cube
            shader.use();

            // View matrix (camera position and orientation)
            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            GLint viewLoc = glGetUniformLocation(shader.ID, "view");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

            // Projection matrix (perspective projection)
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
            GLint projLoc = glGetUniformLocation(shader.ID, "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            // In the render loop, before drawing:
            glm::mat4 model = glm::mat4(1.0f); // Identity matrix
            GLint modelLoc = glGetUniformLocation(shader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Draw the cube
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            if (terrainNeedsUpdate) {

                // Regenerate terrain with new values
                TerrainData newTerrain = generateTerrain(width, height, scale, 10, octaves, persistence, frequency);

                // Update the buffers
                glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
                glBufferData(GL_ARRAY_BUFFER,
                    newTerrain.vertices.size() * sizeof(float),
                    newTerrain.vertices.data(),
                    GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    newTerrain.indices.size() * sizeof(unsigned int),
                    newTerrain.indices.data(),
                    GL_DYNAMIC_DRAW);

                terrain = newTerrain;  // Update the terrain data
                terrainNeedsUpdate = false;  // Reset the flag
            }

            noiseshader.use();


            noiseshader.use();
            glUniformMatrix4fv(glGetUniformLocation(noiseshader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(noiseshader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(noiseshader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


            glBindVertexArray(terrainVAO);
            glDrawElements(GL_TRIANGLES, terrain.indices.size(), GL_UNSIGNED_INT, 0);

            renderImGuiMenu();


            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Cleanup
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
