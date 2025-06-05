#include <cstdio>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "raylib/raylib.h"

// Forward declarations for manual input forwarding:
void UpdateImGuiInput();

int main() {
    InitWindow(800, 600, "Raylib + ImGui custom integration");
    SetTargetFPS(60);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    // Initialize OpenGL3 backend
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!WindowShouldClose()) {
        ImGuiIO &io = ImGui::GetIO();

        // Set display size each frame (fixes your crash)
        io.DisplaySize =
            ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());

        // Update ImGui inputs from raylib
        UpdateImGuiInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello from ImGui!");
        ImGui::Text("This is a Raylib + ImGui example.");
        ImGui::End();

        ImGui::Render();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Render ImGui draw data
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        EndDrawing();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    CloseWindow();

    return 0;
}

void UpdateImGuiInput() {
    ImGuiIO &io = ImGui::GetIO();

    auto RaylibKeyToImGuiKey = [](int key) -> ImGuiKey {
        switch (key) {
            case KEY_SPACE:
                return ImGuiKey_Space;
            case KEY_ENTER:
                return ImGuiKey_Enter;
            case KEY_TAB:
                return ImGuiKey_Tab;
            case KEY_BACKSPACE:
                return ImGuiKey_Backspace;
            case KEY_ESCAPE:
                return ImGuiKey_Escape;
            case KEY_LEFT:
                return ImGuiKey_LeftArrow;
            case KEY_RIGHT:
                return ImGuiKey_RightArrow;
            case KEY_UP:
                return ImGuiKey_UpArrow;
            case KEY_DOWN:
                return ImGuiKey_DownArrow;

            // Modifier keys example:
            case KEY_LEFT_CONTROL:
            case KEY_RIGHT_CONTROL:
                return ImGuiKey_ModCtrl;
            case KEY_LEFT_SHIFT:
            case KEY_RIGHT_SHIFT:
                return ImGuiKey_ModShift;
            case KEY_LEFT_ALT:
            case KEY_RIGHT_ALT:
                return ImGuiKey_ModAlt;
            case KEY_LEFT_SUPER:
            case KEY_RIGHT_SUPER:
                return ImGuiKey_ModSuper;

            default:
                return ImGuiKey_None;  // Ignore unmapped keys
        }
    };

    // Mouse position
    io.MousePos = {(float)GetMouseX(), (float)GetMouseY()};

    // Mouse buttons
    io.MouseDown[0] = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    io.MouseDown[1] = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
    io.MouseDown[2] = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

    // Mouse wheel
    io.MouseWheel += GetMouseWheelMove();

    // Keyboard input (basic)
    io.KeyCtrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    io.KeyShift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    io.KeyAlt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
    io.KeySuper = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);

    // ImGui keys - update as needed, here is minimal example:

    for (int key = 0; key < 512; key++) {
        ImGuiKey imgui_key = RaylibKeyToImGuiKey(key);
        if (imgui_key != ImGuiKey_None) {
            bool pressed = IsKeyDown(key);
            io.AddKeyEvent(imgui_key, pressed);
        }
    }
}
