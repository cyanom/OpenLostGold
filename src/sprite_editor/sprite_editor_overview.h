#ifndef SPRITE_EDITOR_OVERVIEW_H
#define SPRITE_EDITOR_OVERVIEW_H

#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "graphics_base.h"
#include "drawable.h"
#include "sprite.h"
#include "sprite_preview.h"
#include "sprite_editor_overview_renderer.h"
#include "file_browser.h"

namespace uengine::sprite_editor {
    enum SelectionMode {
        SelectionMode_Free = 0, SelectionMode_Tileset = 1
    };

    enum SelectionFreeMode {
        SelectionFreeMode_None = 0, SelectionFreeMode_Crop = 1, SelectionFreeMode_Split = 2
    };

    struct Rect {
        float pos[2];
        float size[2];
        float color[4] = {1.0, 0.0, 0.0, 0.5};
    };

    class SpriteEditorOverview {
        public:
            SpriteEditorOverview(uengine::graphics::GraphicsBase * gb);
            ~SpriteEditorOverview();

            void update();

            // Virtual function implementation
            void renderUI();
            void render(VkCommandBuffer cb);
            void resize(int32_t width, int32_t height);

        private:
            // Data

            uengine::graphics::GraphicsBase * gb;

            struct Overview {
                ImVec2 size = ImVec2(0, 0);
 
                uengine::graphics::Sprite * sprite = nullptr;

                struct MenuBar {
                    ImVec2 size = ImVec2(0, 19);

                    struct FileInfo {
                        std::experimental::filesystem::path file = "";
                        std::string filename = "";
                        bool loaded = false;
                    } fi;

                } mb;

                struct GeneralPanel {
                    ImVec2 pos = ImVec2(0, 0);
                    ImVec2 size = ImVec2(0, 0);

                    float ingameUnitSize[2] = {32, 32};

                    struct Selection {
                        SelectionMode mode = SelectionMode_Free;

                        struct Free {
                            SelectionFreeMode mode = SelectionFreeMode_None;
                        } free;
                        
                        struct Tileset {
                            bool tiedToGrid = true;
                            int offset[2] = {0, 0};
                            int size[2] = {32, 32};
                            bool crop = 0;
                        } tileset;

                        struct Edit {
                            std::string skinName;
                            std::string animationName;
                            int frameId = 0;
                        } edit;
                    } sel;
                } gp;

                struct MainView {
                    ImVec2 pos = ImVec2(0, 0);
                    ImVec2 size = ImVec2(0, 0);
                    
                    glm::mat4 view;
                    glm::mat4 projection;
                   
                    SpriteEditorOverviewRenderer * seor;
                    
                    float backgroundColor[4] = {0.0f, 1.0f, 1.0f, 1.0f};

                    struct Tileset {
                        glm::mat4 model;
                    
                        int size[2] = {640, 640};
                        
                        ImTextureID texture = nullptr;
                    } tileset;

                    struct Grid {
                        float color1[4] = {1.0f, 0.0f, 0.0f, 1.0f};
                        float color2[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                        int unitSize[2] = {32, 32};
                        int offset[2] = {0, 0};
                        bool extended = false;
                    } grid;

                    struct Parameters {
                        float scale = 0.0;
                        float position[2] = {0.0, 0.0};
                    } parameters;

                    struct Interaction {
                        ImVec2 dragDelta = ImVec2(0, 0);
                        ImVec2 leftClickPos = ImVec2(0, 0);
                        ImVec2 rightClickPos = ImVec2(0, 0);
                    } interaction;
                
                    float cur1[2] = {0, 0};
                    float cur2[2] = {0, 0};

                    struct Rect rect = {{0, 0}, {0, 0}, {0, 0, 1, 1}}; // Current selection
                    std::vector<Rect> subRects; // Sub-selections attached to current selection
                    bool currentSelectionChanged = false;
                    std::map<std::string, std::map<std::string, std::vector<Rect>>> rects; // Previous selections
                    bool previousSelectionChanged = false;
                } mv;

                struct SpritePanel {
                    ImVec2 pos = ImVec2(0, 0);
                    ImVec2 size = ImVec2(0, 0);

                    uengine::graphics::Skin * selectedSkin = nullptr;

                    uengine::graphics::Animation * selectedAnimation = nullptr;
                    uengine::graphics::SpritePreview * selectedAnimationPreview = nullptr;

                    std::vector<uengine::graphics::Animation *> animations;
                    std::vector<uengine::graphics::SpritePreview *> animationPreviews;

                    char newAnimationName[100] = {0};
                    char newAnimationRename[100] = {0};
                    bool deleteRequest = false;
                } sp;
            } overview;

            // External UI data
            uengine::ui::FileBrowser fileBrowser;

            // Internal methods
            void load(std::experimental::filesystem::path file);
            void clearSpritePanelData();
            void fillSpritePanelData(uengine::graphics::Skin * skin = nullptr);
            void updateParameters();
            void updatePreviews();
            void updateCurrentSelection();
            void resetModel();
            void resetView();
            void resetProjection();
            void translateView(float dx = 0.0, float dy = 0.0, float dz = 0.0);
            void scaleAbsoluteView(float factor = 1.0);
            void scaleRelativeView(float factor = 1.0);
            void scaleFromPointView(float factor = 1.0, float dx = 0.0, float dy = 0.0);
            glm::vec2 screenToWorld(glm::vec2 pos);
            void refineSelection();
            void correctSelection();
            bool emptyColumn(int y1, int y2, int i);
            bool emptyRow(int x1, int x2, int j);
            void freeNone(int x1, int y1, int x2, int y2);
            void freeCrop(int x1, int y1, int x2, int y2);
            void freeSplit(int x1, int y1, int x2, int y2);
            void tilesetNoCrop(int x1, int y1, int x2, int y2);
            void tilesetCrop(int x1, int y1, int x2, int y2);
            void newAnimation();
            void renameAnimation();
            void deleteAnimation();

            // UI methods
            void adjust();
            void showGeneralPanel();
            void showMainView();
            void showSpritePanel();
            void showMenuBar();
            void showMenuBarMenu();
    };

}

#endif
