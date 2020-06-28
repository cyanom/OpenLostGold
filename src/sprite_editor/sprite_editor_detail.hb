#ifndef SPRITE_EDITOR_DETAIL_H
#define SPRITE_EDITOR_DETAIL_H

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
#include "sprite_editor_detail_renderer.h"

namespace uengine::sprite_editor {

    class SpriteEditorDetail {
        public:
            SpriteEditorDetail(uengine::graphics::GraphicsBase * gb);
            ~SpriteEditorDetail();

            void update();

            // Virtual function implementation
            void renderUI();
            void render(VkCommandBuffer cb);
            void resize(int width, int height);

        private:
            // Data

            uengine::graphics::GraphicsBase * gb;

            struct Overview {
                ImVec2 size = ImVec2(0, 0);
 
                struct MenuBar {
                    ImVec2 size = ImVec2(0, 19);
                } mb;

                struct GeneralPanel {
                    ImVec2 pos = ImVec2(0, 0);
                    ImVec2 size = ImVec2(0, 0);

                    uengine::graphics::Skin * selectedSkin = nullptr;
                    uengine::graphics::Animation * selectedAnimation = nullptr;
                    uengine::graphics::Frame * selectedFrame = nullptr;
                    
                    std::vector<uengine::graphics::Frame *> frames;
                    std::vector<uengine::graphics::SpritePreview *> framePreviews;

                    float ingameUnitSize[2] = {48.0f, 48.0f};
                } gp;

                struct MainView {
                    ImVec2 pos = ImVec2(0, 0);
                    ImVec2 size = ImVec2(0, 0);
                    
                    glm::mat4 view;
                    glm::mat4 projection;
                   
                    SpriteEditorDetailRenderer * sedr;
                    
                    float backgroundColor[4] = {0.0f, 1.0f, 1.0f, 1.0f};

                    uengine::graphics::Sprite * sprite = nullptr;

                    struct Grid {
                        float color1[4] = {1.0f, 0.0f, 0.0f, 1.0f};
                        float color2[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                        float size[2] = {1.0f, 1.0f}
                        float pixelSize[2] = {1.0f/32, 1.0f/32};
                        float offset[2] = {0.0f, 0.0f};
                        bool extended = false;
                    } grid;

                    struct Box {
                        float color[4] = {0.0f, 0.0f, 1.0f, 1.0f};
                        float size[2] = {48.0f, 48.0f};
                    }

                    struct Parameters {
                        float scale = 0.0;
                        float position[2] = {0.0, 0.0};
                    } parameters;

                    struct Interaction {
                        ImVec2 dragDelta = ImVec2(0, 0);
                        ImVec2 leftClickPos = ImVec2(0, 0);
                        ImVec2 rightClickPos = ImVec2(0, 0);
                    } interaction;
                } mv;

                struct SpritePanel {
                    ImVec2 pos = ImVec2(0, 0);
                    ImVec2 size = ImVec2(0, 0);
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
