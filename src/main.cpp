#include "common.h"
#include "VDBLoader.h"

#include "config_io.h"
#include "integrator.h"
#include "utils.h"
#include "scene.h"
#include <vector>

//imGUI
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>


GLFWwindow *window;

namespace VolumeRendering {

    Config config;
    std::ifstream fin;
    std::string file_path;
    VDBLoader loader;
    std::shared_ptr<ImageRGB> rendered_img;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::unique_ptr<Integrator> integrator;

    //render parameters
    float iso_value = 0.03;
    float s_color[3];
    std::vector<Vec3f> colors(1);
    std::vector<float> points(1);

    //
    bool show_demo_window = false;
    bool render = false;
    bool write_img = false;

    void LoadingConfig(int argc, char *argv[]) {
        /// load config from json file
        if (argc == 1) {
            std::cout << "No json specified, use default path." << std::endl;
            file_path = GetFilePath("configs/single-small.json");
            fin.open(file_path);
        } else {
            file_path = argv[1];
            fin.open(GetFilePath(file_path));
        }
        if (!fin.is_open()) {
            std::cerr << "Can not open json file. Exit." << std::endl;
            exit(0);
        } else {
            std::cout << "Json file loaded from " << file_path << std::endl;
        }
        // parse json object to Config
        try {
            nlohmann::json j;
            fin >> j;
            nlohmann::from_json(j, config);
            fin.close();
        } catch (nlohmann::json::exception &ex) {
            fin.close();
            std::cerr << "Error:" << ex.what() << std::endl;
            exit(-1);
        }
        std::cout << "Parsed json to config. Start building scene..." << std::endl;
    }

    void InitSettings() {
        // initialize all settings from config
        // set image resolution.
        rendered_img = std::make_shared<ImageRGB>(config.image_resolution[0], config.image_resolution[1]);
        std::cout << "Image resolution: "
                  << config.image_resolution[0] << " x " << config.image_resolution[1] << std::endl;
        // set camera
        camera = std::make_shared<Camera>(config.cam_config, rendered_img);
        // construct scene.
        scene = std::make_shared<Scene>();
        initSceneFromConfig(config, scene);
        scene->setAmbient(Vec3f(0.1, 0.1, 0.1));
        //load vdb
        loader.load(GetFilePath(config.file_path));
        std::cout << "Initialize Setting Finished" << std::endl;

        //init parameters
        iso_value = config.iso_value;
    }

    void DrawContents(std::shared_ptr<ImageRGB> &img) {
        Vec2i res = img->getResolution();
        glDrawPixels(res[0], res[1], GL_RGB, GL_UNSIGNED_BYTE, img->getdata());
    }

    void RenderImg() {
        integrator = std::make_unique<Integrator>(camera, scene, config.spp, loader.grids, config.iso_value, config.var,
                                                  config.step_scale);
        integrator->render();
        render = false;
    }

    void RenderOpenGL() {
        DrawContents(camera->getImage());
    }

    void WriteImg() {
        rendered_img->writeImgToFile("../result.png");
        std::cout << "Image saved to disk." << std::endl;
        write_img = false;
    }

    void RenderMainImGui() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // 1. Show the big demo window//

        {
            ImGui::Begin("Volume Rendering");
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state

            if (ImGui::TreeNode("Render settings")) {
                ImGui::SliderFloat("iso-value", &iso_value, 0, 1);
                integrator->setiso_value(iso_value);
                ImGui::ColorEdit3("Sphere Color", s_color);
                scene->setObjColor(Vec3f(s_color));
                ImGui::Text("Colors");

                //add color
                if(ImGui::Button("Add")){
                    colors.push_back(Vec3f{1,1,1});
                }
                ImGui::SameLine();
                if(ImGui::Button("Remove")){
                    colors.pop_back();
                }
                static int selected = 0;
                {
                    ImGui::BeginChild("left pane", ImVec2(150, 0), true);

                    for (int i = 0; i < colors.size(); ++i) {
                        char label[128];
                        sprintf(label, "MyObject %d", i);
                        if (ImGui::Selectable(label, selected == i))
                            selected = i;
                    }
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                {
                    ImGui::BeginGroup();
                    ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
                    ImGui::Text("MyObject: %d", selected);
                    ImGui::Separator();

                    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
                    {
                        if (ImGui::BeginTabItem("Color"))
                        {
                            ImGui::ColorEdit3("Pick Color",colors[selected].asPointer());
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("position"))
                        {
                            ImGui::SliderFloat("Pick point",&points[selected],0,1);
                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }



                    ImGui::EndChild();
                    ImGui::EndGroup();

                }

            }


            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            if (ImGui::Button("Start Rendering")) {
                render = true;
            }


            if (render) {
                std::cout << "Start Rendering..." << std::endl;
                start = std::chrono::system_clock::now();
                // render scene
                RenderImg();

                end = std::chrono::system_clock::now();
                auto time = std::chrono::duration<double>(end - start);
                std::cout << "\nRender Finished in " << time << "s." << std::endl;

            }

            if (ImGui::Button("Write Image"))
                write_img = true;
            if (write_img)
                WriteImg();
            ImGui::End();
        }



        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void processInput(GLFWwindow *window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

    }

}


/*
void processInput(GLFWwindow *window, std::shared_ptr<Camera> camera) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
}*/



int main(int argc, char *argv[]) {

    VolumeRendering::LoadingConfig(argc, argv);
    VolumeRendering::InitSettings();
    int WIDTH = VolumeRendering::config.image_resolution[0];
    int HEIGHT = VolumeRendering::config.image_resolution[1];
//GUI
    WindowGuard windowGuard(window, WIDTH, HEIGHT, "CG");
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); // Setup Dear ImGui context
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui::StyleColorsDark(); // Setup Dear ImGui style
    const char *glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Setup Platform/Renderer bindings
    ImGui_ImplOpenGL3_Init(glsl_version);

    VolumeRendering::RenderImg();
    VolumeRendering::WriteImg();
    while (!glfwWindowShouldClose(window)) {
        VolumeRendering::processInput(window);
        VolumeRendering::RenderOpenGL();
        VolumeRendering::RenderMainImGui();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


#define TEST
#ifndef TEST
    VolumeRendering::RenderImg();
    VolumeRendering::WriteImg();


#else


#endif
    return 0;
}