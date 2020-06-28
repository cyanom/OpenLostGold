#ifndef SPRITE_EDITOR_H
#define SPRITE_EDITOR_H

#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "drawable.h"
#include "sprite_editor_overview.h"

namespace uengine::sprite_editor {

    class SpriteEditor: public uengine::graphics::Drawable {
        public:
            SpriteEditor(uengine::graphics::GraphicsBase * gb);
            ~SpriteEditor();
            
            void update();

            // Virtual function implementation
            void renderUI();
            void render(VkCommandBuffer cb);
            void resize(int32_t width, int32_t height);

        private:
            // Tileset editor data
            SpriteEditorOverview * seo;
    };

}

#endif
