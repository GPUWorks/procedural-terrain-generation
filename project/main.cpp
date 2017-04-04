// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <unistd.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "heightmap/heightmap.h"
#include "terrain/terrain.h"

#define CAMERA_SPEED 2.0

using namespace glm;

Heightmap heightmap;
Terrain terrain;

int window_width = 800;
int window_height = 600;

mat4 projection_matrix;
mat4 view_matrix;

// Camera
vec3 cam_pos, cam_dir, cam_vel;
vec3 light_pos;

void Init(GLFWwindow* window) {
    glClearColor(1.0, 1.0, 1.0 /*white*/, 1.0 /*solid*/);
    glEnable(GL_DEPTH_TEST);

    // setup view and projection matrices
    light_pos = vec3(-1.0f, 0.0f, 2.0f);

    cam_pos = vec3(-2.0f, -2.0f, 2.0f);
    cam_dir = -cam_pos;

    vec3 cam_look = cam_pos + cam_dir;
    vec3 cam_up(0.0f, 0.0f, 1.0f);
    view_matrix = lookAt(cam_pos, cam_look, cam_up);

    projection_matrix = perspective(45.0f, (float)window_width / (float)window_height, 0.1f, 10.0f);

    GLuint heightmap_tex_id = heightmap.Init(512, 512);
    terrain.Init(heightmap_tex_id);
    terrain.SetLighting(light_pos);
}

void Update(float dt) {
    if (cam_vel[0] != 0.0 || cam_vel[1] != 0.0 || cam_vel[2] != 0.0) {
        cam_pos += dt * cam_vel;
        vec3 cam_look = cam_pos + cam_dir;
        vec3 cam_up(0.0f, 0.0f, 1.0f);
        view_matrix = lookAt(cam_pos, cam_look, cam_up);
    }

    static bool first_run = true;
    static float speed = 0.0;
    static float hoffset[2] = { 0.0, 0.0 };

    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f));
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (first_run)
        ImGui::SetNextTreeNodeOpen(true);

    if (ImGui::CollapsingHeader("Terrain Options")) {
        ImGui::DragFloat2("hoffset", hoffset, 0.005);
        ImGui::DragFloat("voffset", &heightmap.voffset_, 0.005);

        ImGui::DragFloat("hcomp", &heightmap.hcomp_, 0.005);
        ImGui::DragFloat("vcomp", &heightmap.vcomp_, 0.005);

        ImGui::DragInt("seed", &heightmap.seed_, 0.05);
        ImGui::DragFloat("speed", &speed, 0.01);
    }

    if (first_run)
        ImGui::SetNextTreeNodeOpen(true);

    if (ImGui::CollapsingHeader("Harmonic Options")) {
        ImGui::RadioButton("fBm", &heightmap.type_, 0); ImGui::SameLine();
        ImGui::RadioButton("ridged fBm", &heightmap.type_, 1); ImGui::SameLine();
        ImGui::RadioButton("billowy fBm", &heightmap.type_, 2);

        ImGui::SliderFloat("H", &heightmap.H_, 0.01, 2.0);
        ImGui::SliderFloat("lacunarity", &heightmap.lacunarity_, 0.8, 3.0);
        ImGui::SliderInt("octaves", &heightmap.octaves_, 1, 24);
        ImGui::SliderFloat("offset", &heightmap.offset_, -10.0, 10.0);
    }

    heightmap.dx_ = (hoffset[0] += speed * dt);
    heightmap.dy_ = (hoffset[1] += speed * dt);
    heightmap.Draw();

    first_run = false;
}

void Display() {
    glViewport(0, 0, window_width, window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    terrain.Draw(IDENTITY_MATRIX, view_matrix, projection_matrix);
}

// gets called when the windows/framebuffer is resized.
void ResizeCallback(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    projection_matrix = perspective(45.0f, (float)width / (float)height, 0.1f, 10.0f);

    glViewport(0, 0, width, height);
}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (ImGui::GetIO().WantCaptureKeyboard) {
        ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
        return;
    }

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;

            case GLFW_KEY_W:
                cam_vel[0] += CAMERA_SPEED;
                break;
            case GLFW_KEY_S:
                cam_vel[0] -= CAMERA_SPEED;
                break;
            case GLFW_KEY_A:
                cam_vel[1] += CAMERA_SPEED;
                break;
            case GLFW_KEY_D:
                cam_vel[1] -= CAMERA_SPEED;
                break;
            case GLFW_KEY_F:
                cam_vel[2] -= CAMERA_SPEED;
                break;
            case GLFW_KEY_R:
                cam_vel[2] += CAMERA_SPEED;
                break;

            default:
                ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
        }
    }

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_W:
                cam_vel[0] -= CAMERA_SPEED;
                break;
            case GLFW_KEY_S:
                cam_vel[0] += CAMERA_SPEED;
                break;
            case GLFW_KEY_A:
                cam_vel[1] -= CAMERA_SPEED;
                break;
            case GLFW_KEY_D:
                cam_vel[1] += CAMERA_SPEED;
                break;
            case GLFW_KEY_F:
                cam_vel[2] += CAMERA_SPEED;
                break;
            case GLFW_KEY_R:
                cam_vel[2] -= CAMERA_SPEED;
                break;

            default:
                ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
        }
    }
}

void CharCallback(GLFWwindow *window, unsigned int codepoint) {
    ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
}

void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);
}

int main(int argc, char *argv[]) {
    // GLFW Initialization
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(ErrorCallback);

    // hint GLFW that we would like an OpenGL 3 context (at least)
    // http://www.glfw.org/faq.html#how-do-i-create-an-opengl-30-context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // attempt to open the window: fails if required version unavailable
    // note some Intel GPUs do not support OpenGL 3.2
    // note update the driver of your graphic card
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          "Terrain Generation", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // set the callback for inputs
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCharCallback(window, CharCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);

    // set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, ResizeCallback);

    // GLEW Initialization (must have a context)
    // https://www.opengl.org/wiki/OpenGL_Loading_Library
    glewExperimental = GL_TRUE; // fixes glew error (see above link)
    if(glewInit() != GLEW_NO_ERROR) {
        fprintf( stderr, "Failed to initialize GLEW\n");
        return EXIT_FAILURE;
    }

    cout << "OpenGL" << glGetString(GL_VERSION) << endl;

    // init ImGui
    ImGui_ImplGlfwGL3_Init(window, false);

    // initialize our OpenGL program
    Init(window);

    float lastTime = glfwGetTime();
    // render loop
    while(!glfwWindowShouldClose(window)){
        float dt = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();

        ImGui_ImplGlfwGL3_NewFrame();

        Update(dt);

        Display();
        ImGui::Render();
        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // cleanup
    terrain.Cleanup();
    heightmap.Cleanup();

    // close OpenGL window and terminate GLFW
    ImGui_ImplGlfwGL3_Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
