#ifndef SPRITE_EDITOR_TILESET_RENDERER_H
#define SPRITE_EDITOR_TILESET_RENDERER_H

#include <array>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "drawable.h"
#include "sprite.h"
#include "graphics_base.h"
#include "graphics_helper.h"
#include "graphics_quads.h"
#include "graphics_grid.h"

namespace uengine::sprite_editor {

    class SpriteEditorOverviewRenderer {
    public:
        SpriteEditorOverviewRenderer(uengine::graphics::GraphicsBase * gb);
        ~SpriteEditorOverviewRenderer();

        void setBackgroundColor(float color[4]);
        void setViewProjection(glm::mat4 vp);
        
        uengine::graphics::GraphicsGrid * getGrid();
        uengine::graphics::GraphicsQuads * getCurrentSelectionQuads();
        uengine::graphics::GraphicsQuads * getPreviousSelectionQuads();

        void update();
        void render(VkCommandBuffer cb);
        void resize(int32_t width, int32_t height);
        ImTextureID getTexture();

    private:
        uengine::graphics::GraphicsBase * gb;
        VkDescriptorPool descriptorPool;
        VkRenderPass renderPass;

        float backgroundColor[4] = {0.0, 0.0, 0.0, 0.0};
        uengine::graphics::GraphicsGrid * grid;
        uengine::graphics::GraphicsQuads * currentSelectionQuads;
        uengine::graphics::GraphicsQuads * previousSelectionQuads;

        // Offscreen methods
        
        struct Offscreen {
            int32_t width=0, height=0;
            VkDeviceMemory mem;
            VkImage image;
            VkImageView view;
            VkFramebuffer frameBuffer;		
            VkSampler sampler;
            ImTextureID texture = nullptr;
        } offscreen;

        void setupOffscreen(int32_t width, int32_t height);
        void destroyOffscreen();
        
        // Renderpass methods
        
        void setupRenderPass();
    };

}

#endif
