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

    bool isGuiOpen = false;  // Tracks whether the GUI menu is open

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
    int CHUNK_COUNT = 16;
    int GRID_SIZE = 4;
    int octaves = 4;
    float persistence = 0.5f;
    int width = 32;
    int height = 32;
    float frequency = 2.0f;
    int scale = 50;
    float heightScale = 10.0f;
    float lacunarity = 2.0f;

    bool terrainNeedsUpdate = false; //update whenever changes in noise function

    void renderImGuiMenu() {
        if (!isGuiOpen) return;  // Don't render if menu is closed

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings Menu");

        int oldoctaves = octaves;
        int oldwidth = width;
        int oldheight = height;
        float oldfrequency = frequency;
        int oldscale = scale;
        float oldpersistence = persistence;
        float oldheightscale = heightScale;
        float oldLacunarity = lacunarity;

        ImGui::SliderFloat("Persistence", &persistence, 0.0f, 1.0f);
        ImGui::SliderInt("Octaves", &octaves, 0, 10);
        ImGui::SliderInt("Width", &width, 0, 1000);
        ImGui::SliderInt("height", &height, 0, 1000);
        ImGui::SliderFloat("Frequency", &frequency, 0.0f, 8.0f);
        ImGui::SliderInt("Scale", &scale, 0, 100);
        ImGui::SliderFloat("Lacunarity", &lacunarity, 0.0f, 5.0f);
        ImGui::SliderFloat("Height Scale", &heightScale, 0.0f, 50.0f);
        if (oldpersistence != persistence || oldoctaves != octaves ||
            oldwidth != width || oldheight != height || oldfrequency != frequency || oldscale != scale || oldLacunarity != lacunarity || oldheightscale != heightScale) {
            terrainNeedsUpdate = true;
        }

        ImGui::End();

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

    bool isMouseOverImGui = false;  // Flag to check if ImGui has mouse focus
    bool cameraControlEnabled = true;

    void processInput(GLFWwindow* window) {
        static bool eKeyPressed = false;  // Track E key state

        // Toggle GUI with E key
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            if (!eKeyPressed) {  // Only toggle once per press
                isGuiOpen = !isGuiOpen;
                if (isGuiOpen) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                eKeyPressed = true;
            }
        }
        else {
            eKeyPressed = false;
        }

        // Only process camera movement if GUI is closed
        if (!isGuiOpen) {
            float cameraSpeed = 20.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraPos += cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraPos -= cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        }
    }



    // Mouse callback for camera orientation
    void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        if (isGuiOpen) return;  // Don't process mouse movement when GUI is open

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

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

        float seed;


        struct TerrainChunk {
            unsigned int VAO, VBO, EBO;
            TerrainData terrain;
            float xOffset, zOffset;
        };
        
        std::vector<TerrainChunk> chunkList;
        unsigned int zOffset = 0;
        for (int i = 0; i < CHUNK_COUNT; i++) {
            seed = glfwGetTime();
            TerrainChunk chunk;

            // Calculate grid position
            int gridX = i % GRID_SIZE;
            int gridZ = i / GRID_SIZE;

            // Calculate world position
            chunk.xOffset = gridX * (width - 1); // Correct for overlap
            chunk.zOffset = gridZ * (height - 1); // Correct for overlap


            // Generate terrain data first
            chunk.terrain = generateTerrain(width, height, scale, seed, octaves,
                persistence, frequency, lacunarity, heightScale, chunk.xOffset, chunk.zOffset);

            //// set position of terrain data according to grid position
            //for (int i = 0; i < chunk.terrain.vertices.size(); i++)
            //{
            //    if (i % 3 == 0)
            //        chunk.terrain.vertices.at(i) += chunk.xOffset;
            //    if (i % 3 == 2)
            //        chunk.terrain.vertices.at(i) += chunk.zOffset;
            //}
            // Create OpenGL buffers
            glGenVertexArrays(1, &chunk.VAO);
            glGenBuffers(1, &chunk.VBO);
            glGenBuffers(1, &chunk.EBO);

            // Bind and setup VAO/VBO
            glBindVertexArray(chunk.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, chunk.VBO);
            glBufferData(GL_ARRAY_BUFFER,
                chunk.terrain.vertices.size() * sizeof(float),
                chunk.terrain.vertices.data(),
                GL_STATIC_DRAW);

            // Setup indices
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                chunk.terrain.indices.size() * sizeof(unsigned int),
                chunk.terrain.indices.data(),
                GL_STATIC_DRAW);

            // Setup vertex attributes
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            // Add to list
            chunkList.push_back(chunk);
        }

        // Shader setup (place the shaders in the same directory)
        Shader shader("shader.vs", "shader.fs");
        Shader noiseshader("noiseshader.vs", "noiseshader.fs");
        // Background color
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Register mouse callback
        glfwSetCursorPosCallback(window, mouse_callback);

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

            // Model matrix
            glm::mat4 model = glm::mat4(1.0f);
            GLint modelLoc = glGetUniformLocation(shader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Draw the cube
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Update terrain if needed
            if (terrainNeedsUpdate) {
                for (int i = 0; i < chunkList.size(); i++) {
                    float seed = glfwGetTime();
                    TerrainChunk chunk = chunkList[i];
                    // Calculate grid position
                    int gridX = i % GRID_SIZE;
                    int gridZ = i / GRID_SIZE;

                    // Calculate world position
                    chunk.xOffset = gridX * (width - 1); // Correct for overlap
                    chunk.zOffset = gridZ * (height - 1); // Correct for overlap

                    chunk.terrain = generateTerrain(width, height, scale, seed, octaves, persistence, frequency, lacunarity, heightScale, chunk.xOffset, chunk.zOffset);

                    glBindBuffer(GL_ARRAY_BUFFER, chunk.VBO);
                    glBufferData(GL_ARRAY_BUFFER,
                        chunk.terrain.vertices.size() * sizeof(float),
                        chunk.terrain.vertices.data(),
                        GL_DYNAMIC_DRAW);

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.EBO);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                        chunk.terrain.indices.size() * sizeof(unsigned int),
                        chunk.terrain.indices.data(),
                        GL_DYNAMIC_DRAW);
                }
                terrainNeedsUpdate = false; // Reset update flag
            }

            // Render terrain chunks
            noiseshader.use();
            glUniformMatrix4fv(glGetUniformLocation(noiseshader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(noiseshader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(noiseshader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            for (const TerrainChunk& chunk : chunkList) {
                glBindVertexArray(chunk.VAO);
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(chunk.terrain.indices.size()), GL_UNSIGNED_INT, 0);
            }

            // Render ImGui menu
            renderImGuiMenu();

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
        return 0;

        // Cleanup
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
