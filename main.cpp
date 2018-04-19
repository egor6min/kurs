// ImGui - standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the opengl3_example/ folder**
// See imgui_impl_glfw.cpp for details.

#include "imgui.h"
#include "imgui_impl_glfw_gl2.h"
#include "SOIL.h"
#include "ImageMagick-7/Magick++.h"
#include "ImageMagick-7/Magick++/Geometry.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>


size_t rows = 3;
size_t cols = 3;
size_t img_width = 128;
size_t img_height = 128;

static void copy(const char* src, const char* dst) {
    fprintf(stderr, "Copy %s to %s\n", src, dst);
    int in = open(src, O_RDONLY | O_CREAT, 0666);
    int out = open(dst, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    unsigned char c;
    while (read(in, &c, 1) > 0)
        write(out, &c, 1);
    close(in);
    close(out);
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Error %d: %s\n", error, description);
}

static std::vector<ImTextureID> prepare_grid_images(const char* orig_name) {
    Magick::Image image(orig_name);
    img_width = image.columns() / cols;
    img_height = image.rows() / rows;

    system("rm -r res");
    system("mkdir res");
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            Magick::Geometry geom1((j + 1) * img_width, (i + 1) * img_height);
            Magick::Geometry geom2(j * img_width, i * img_height);
            Magick::Image cur(image);
            cur.crop(geom1);
            cur.chop(geom2);
            char img_name[100];
            sprintf(img_name, "/tmp/timg%lu_%lu.png", i, j);
            cur.write(img_name);
        }
    }

    std::vector<ImTextureID> img_tex_id(rows * cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            char img_name[100];
            sprintf(img_name, "/tmp/timg%d_%d.png", i, j);
            img_tex_id[i * cols + j] = (void*)SOIL_load_OGL_texture(img_name,
                                        SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
        }
    }
    return img_tex_id;
}

int main(int argc, char** argv) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Captcha", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL2_Init(window, true);
    ImGui::StyleColorsClassic();

    Magick::InitializeMagick(*argv);
    std::string orig_img_name = "test.jpg";
    std::vector<ImTextureID> img_tex_id = prepare_grid_images(orig_img_name.c_str());


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplGlfwGL2_NewFrame();

        {
            static char new_img_name[100] = "test.jpg";
            static int new_cols = 3;
            static int new_rows = 3;
            ImGui::InputText("image", new_img_name, IM_ARRAYSIZE(new_img_name));
            ImGui::InputInt("cols", &new_cols);
            ImGui::InputInt("rows", &new_rows);
            if (ImGui::Button("Update grid")) {
                cols = new_cols;
                rows = new_rows;
                orig_img_name = std::string(new_img_name);
                img_tex_id = prepare_grid_images(orig_img_name.c_str());
            }
            ImGui::Columns(cols, NULL, false);
            for (size_t i = 0; i < rows; ++i) {
                for (size_t j = 0; j < cols; ++j) {

                    if (ImGui::ImageButton(img_tex_id[i * cols + j], ImVec2(img_width, img_height))) {
                        char img_name[100], buf[100];
                        sprintf(img_name, "/tmp/timg%lu_%lu.png", i, j);
                        sprintf(buf, "res/img%lu_%lu.png", i, j);
                        copy(img_name, buf);
                    }
                    ImGui::NextColumn();
                }
            }
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL2_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}
