#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include "MemoryManager.hpp"
#include <cstdio>
#include <functional>
#include <string>

/*
    GLFW erroru callbackas
*/
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

/*
    paskiria procesu blokam spalvas pagal ju vardus
*/
static ImU32 processColor(const std::string& name) {
    static const ImU32 palette[] = {
        IM_COL32(210, 70,  70,  255),
        IM_COL32(70,  180, 90,  255),
        IM_COL32(70,  110, 230, 255),
        IM_COL32(230, 160, 40,  255),
        IM_COL32(170, 70,  210, 255),
        IM_COL32(40,  200, 200, 255),
        IM_COL32(220, 210, 40,  255),
        IM_COL32(220, 110, 160, 255),
    };

    // uzhashuoja pavadinima ir paima liekana nuo 8 (spalvu skaiciaus)
    return palette[std::hash<std::string>{}(name) % 8];
}

/*
    kaires puses panel'is, kuriame valdomas atminties dydis, algoritmas, bloku dydis ir t.t.
*/
static void drawConfigPanel(MemoryManager& mm, int& memSize, int& algoIdx,
                            int& blkSize, int& fitIdx, char* procName,
                            std::string& statusMsg, bool& statusOk) {

    ImGui::TextColored({ 0.8f, 0.8f, 1.f, 1.f }, "CONFIGURATION");
    ImGui::Separator();
    ImGui::Spacing();

    bool locked = mm.initialized;
    if (locked) ImGui::BeginDisabled();

    ImGui::InputInt("Memory size", &memSize);
    if (memSize < 4) memSize = 4;

    const char* algoItems[] = { "Static (Equal Blocks)", "Static (Variable (2^n))", "Dynamic" };
    ImGui::Combo("Algorithm", &algoIdx, algoItems, 3);

    if (algoIdx == 0) {
        ImGui::InputInt("Block size", &blkSize);
        if (blkSize < 1)       blkSize = 1;
        if (blkSize > memSize) blkSize = memSize;
    }

    if (algoIdx == 2) {
        const char* fitItems[] = { "First Fit", "Best Fit", "Worst Fit" };
        ImGui::Combo("Fit algorithm", &fitIdx, fitItems, 3);
    }

    if (locked) ImGui::EndDisabled();

    ImGui::Spacing();

    if (!locked) {
        if (ImGui::Button("Initialize", { 140, 30 })) {
            MemoryManager::Mode mode;
            FitStrategy fit;

            switch (algoIdx) {
                case 0:  mode = MemoryManager::Mode::StaticEqual;    break;
                case 1:  mode = MemoryManager::Mode::StaticVariable; break;
                default: mode = MemoryManager::Mode::Dynamic;        break;
            }

            switch (fitIdx) {
                case 0:  fit = FitStrategy::FirstFit; break;
                case 1:  fit = FitStrategy::BestFit;  break;
                default: fit = FitStrategy::WorstFit; break;
            }

            mm.initialize(memSize, mode, blkSize, fit);
            statusMsg = "Memory initialized.";
            statusOk  = true;
        }

    } else {
        if (ImGui::Button("Reset", { 140, 30 })) {
            mm.reset();
            procName[0] = '\0';
            statusMsg = "Reset. Configure and initialize again.";
            statusOk  = true;
        }
    }
}

/*
    kaires puses panelis, valdantis procesu kurima, naikinima, defragmentavima
*/
static void drawProcessControls(MemoryManager& mm, char* procName, int& procSize,
                                std::string& statusMsg, bool& statusOk) {
        
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored({ 0.8f, 0.8f, 1.f, 1.f }, "PROCESS CONTROL");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::InputText("Name", procName, 64);
    ImGui::InputInt("Size needed", &procSize);
    if (procSize < 1) procSize = 1;

    ImGui::Spacing();

    if (ImGui::Button("Allocate", { 130, 28 })) {
        if (procName[0] == '\0') {
            statusMsg = "Process name cannot be empty.";
            statusOk  = false;
        } else if (!mm.allocate(procName, procSize)) {
            statusMsg = std::string("Not enough memory for '") + procName + "'.";
            statusOk  = false;
        } else {
            statusMsg = std::string("Allocated ") + std::to_string(procSize)
                        + " for '" + procName + "'.";
            statusOk  = true;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Free", { 80, 28 })) {
        if (procName[0] == '\0') {
            statusMsg = "Process name cannot be empty.";
            statusOk  = false;

        } else if (!mm.free(procName)) {
            statusMsg = std::string("Process '") + procName + "' not found.";
            statusOk  = false;

        } else {
            statusMsg = std::string("Freed memory for '") + procName + "'.";
            statusOk  = true;
        }
    }

    ImGui::Spacing();
    if (ImGui::Button("Defragment", { 214, 28 })) {
        mm.defragment();
        statusMsg = "Memory defragmented.";
        statusOk  = true;
    }

    ImGui::Spacing();
    ImGui::Separator();

    int used = 0;
    for (const auto& b : mm.blocks) {
        if (!b.isFree) used += b.size;
    }

    float percent = mm.totalSize > 0 ? (float)used / mm.totalSize : 0.f;
    
    ImGui::Text("Used: %d / %d  (%.0f%%)", used, mm.totalSize, percent * 100.f);

    ImGui::ProgressBar(percent, { -1, 0 });
}

/*
    statuso zinutes panel'is, rodantis paskutines operacijos rezultata
*/
static void drawStatusMessage(const std::string& statusMsg, bool statusOk) {
    if (statusMsg.empty()) return;
    
    ImGui::Spacing();
    ImGui::Separator();
    ImVec4 col = statusOk
        ? ImVec4{ 0.4f, 1.f, 0.4f, 1.f }
        : ImVec4{ 1.f, 0.4f, 0.4f, 1.f };
    ImGui::TextColored(col, "%s", statusMsg.c_str());
}

/*
    desnes puses panelis, vizualizuoja atminties suskirstymas
*/
static void drawMemoryMap(MemoryManager& mm, std::string& statusMsg, bool& statusOk) {
    ImGui::TextColored({ 0.8f, 0.8f, 1.f, 1.f }, "MEMORY MAP");
    ImGui::Separator();

    ImGui::BeginChild("##map", { 0, 100 }, true, ImGuiWindowFlags_NoScrollbar);

    if (mm.initialized && !mm.blocks.empty()) {
        float total  = (float)mm.totalSize;
        float availW = ImGui::GetContentRegionAvail().x;
        float availH = ImGui::GetContentRegionAvail().y;
        ImVec2 origin = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // iteruoja per blokus ir nupiesia juos, paskiria spalvas, etc..
        for (const auto& b : mm.blocks) {
            float x0 = origin.x + (b.offset / total) * availW;
            float x1 = origin.x + ((b.offset + b.size) / total) * availW;
            float y0 = origin.y;
            float y1 = origin.y + availH - 2.f;

            ImU32 fill = b.isFree
                ? IM_COL32(60, 65, 75, 255)
                : processColor(b.processName);

            dl->AddRectFilled({ x0 + 1, y0 + 1 }, { x1 - 1, y1 - 1 }, fill, 4.f);
            dl->AddRect(      { x0,     y0     }, { x1,     y1     },
                        IM_COL32(150, 150, 165, 200), 4.f);

            float bw = x1 - x0;
            if (bw > 28.f) {
                std::string label = b.isFree ? "Free" : b.processName;
                ImVec2 ts = ImGui::CalcTextSize(label.c_str());
                if (ts.x < bw - 6.f) {
                    dl->AddText(
                        { x0 + (bw - ts.x) * 0.5f, y0 + (availH - 2.f - ts.y) * 0.5f },
                        IM_COL32_WHITE, label.c_str());
                }
            }

            // tooltipas
            if (ImGui::IsMouseHoveringRect({ x0, y0 }, { x1, y1 })) {
                ImGui::BeginTooltip();
                ImGui::Text("Block #%d", b.id);
                ImGui::Text("Offset: %d  |  Size: %d", b.offset, b.size);
                if (b.isFree) ImGui::TextDisabled("Free");
                else {
                    ImGui::Text("Process: %s", b.processName.c_str());
                    ImGui::TextDisabled("Click to free");
                }
                ImGui::EndTooltip();

                if (!b.isFree && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    std::string freed = b.processName;
                    mm.free(freed);
                    statusMsg = "Freed memory for '" + freed + "'.";
                    statusOk  = true;
                    break;
                }
            }
        }
        
        ImGui::Dummy({ availW, availH });

    } else {
        ImGui::TextDisabled("Not initialized");
    }

    ImGui::EndChild(); // ##map
}

/*
    panelis apacioj logams
*/
static void drawAllocationLog(MemoryManager& mm) {
    ImGui::TextColored({ 0.8f, 0.8f, 1.f, 1.f }, "ALLOCATION LOG");
    ImGui::Separator();

    ImGui::BeginChild("##log", { 0, 0 }, true);

    if (ImGui::BeginTable("##logtbl", 5,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit)) {

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Time",    ImGuiTableColumnFlags_WidthFixed,   70.f);
        ImGui::TableSetupColumn("Process", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Block",   ImGuiTableColumnFlags_WidthFixed,   55.f);
        ImGui::TableSetupColumn("Size",    ImGuiTableColumnFlags_WidthFixed,   50.f);
        ImGui::TableSetupColumn("Action",  ImGuiTableColumnFlags_WidthFixed,   95.f);
        ImGui::TableHeadersRow();

        for (const auto& e : mm.log) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(e.timestamp.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(e.processName.c_str());

            ImGui::TableSetColumnIndex(2);
            if (e.blockId >= 0) ImGui::Text("#%d", e.blockId);
            else                ImGui::TextDisabled("-");

            ImGui::TableSetColumnIndex(3);
            if (e.size > 0) ImGui::Text("%d", e.size);
            else            ImGui::TextDisabled("-");

            ImGui::TableSetColumnIndex(4);
            ImVec4 c = { 0.8f, 0.8f, 0.8f, 1.f };

            if      (e.action == "Allocated")    c = { 0.4f, 0.9f, 0.4f, 1.f };
            else if (e.action == "Freed")        c = { 0.9f, 0.5f, 0.3f, 1.f };
            else if (e.action == "Defragmented") c = { 0.4f, 0.7f, 1.0f, 1.f };
            
            ImGui::TextColored(c, "%s", e.action.c_str());
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndTable();
    }

    ImGui::EndChild(); // ##log
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "MemMngr", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    MemoryManager mm;
    int  memSize  = 128;
    int  algoIdx  = 0;
    int  blkSize  = 16;
    int  fitIdx   = 0;
    char procName[64] = "";
    int  procSize = 16;
    std::string statusMsg;
    bool statusOk = true;

    const ImVec4 clearColor = { 0.10f, 0.10f, 0.12f, 1.00f };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##root", nullptr,
            ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoMove      | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Kairys panelis
        ImGui::BeginChild("##left", { 320, 0 }, true);

        drawConfigPanel(mm, memSize, algoIdx, blkSize, fitIdx, procName, statusMsg, statusOk);

        if (mm.initialized)
            drawProcessControls(mm, procName, procSize, statusMsg, statusOk);

        drawStatusMessage(statusMsg, statusOk);

        ImGui::EndChild(); // ##left
        ImGui::SameLine();

        // Desnys panelis
        ImGui::BeginChild("##right", { 0, 0 }, false);

        drawMemoryMap(mm, statusMsg, statusOk);

        ImGui::Spacing();

        drawAllocationLog(mm);

        ImGui::EndChild();  // ##right
        ImGui::End(); // root

        ImGui::Render();

        int fw, fh;
        glfwGetFramebufferSize(window, &fw, &fh);
        glViewport(0, 0, fw, fh);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // cleanupas

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
