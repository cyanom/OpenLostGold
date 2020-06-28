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

namespace uengine::sprite_editor {

    class SpriteEditorOverviewRenderer {
    public:
        SpriteEditorOverviewRenderer(uengine::graphics::GraphicsBase * gb);
        ~SpriteEditorOverviewRenderer();

        void setBackgroundColor(float color[4]);
        void setVP(glm::mat4 vp);
        
        void setGridColor(float color1[4], float color2[4]);
        void setGridUnitSize(int unitSize[2]);
        void setGridTilesetSize(int tilesetSize[2]);
        void setGridViewSize(int viewSize[2]);
        void setGridOffset(int offset[2]);
        void setGridExtended(bool extented);
        
        void setTilesetSprite(uengine::graphics::Sprite * sprite);
        void setTilesetModel(glm::mat4 model);
        
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

        struct Offscreen {
            int32_t width=0, height=0;
            VkDeviceMemory mem;
            VkImage image;
            VkImageView view;
            VkFramebuffer frameBuffer;		
            VkSampler sampler;
            ImTextureID texture = nullptr;
        } offscreen;

        float backgroundColor[4] = {0.0, 0.0, 0.0, 0.0};
        glm::mat4 vp;

        struct Grid {
            bool changed = false;
            struct InverseMVPData {
                glm::mat4 inverseMVP;
            } inverseMVPData;

            struct GridParamData {
                alignas(16) glm::vec4 color1;
                alignas(16) glm::vec4 color2;
                alignas(8) glm::vec2 unitSize;
                alignas(8) glm::vec2 tilesetSize;
                alignas(8) glm::vec2 viewSize;
                alignas(8) glm::vec2 offset;
                alignas(4) float extended;
            } gridParamData;

            VkBuffer inverseMVPBuffer;
            VkBuffer gridParamBuffer;
            VkDeviceMemory inverseMVPMemory;
            VkDeviceMemory gridParamMemory;

            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSet descriptorSet;
            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;
        } grid;

        struct Tileset {
            bool changed = false;
            uengine::graphics::Sprite * sprite;
            glm::mat4 model;
            
            struct TilesetMVPData {
                glm::mat4 tilesetMVP;
            } tilesetMVPData;

            VkBuffer tilesetMVPBuffer;
            VkDeviceMemory tilesetMVPMemory;

            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSet descriptorSet;
            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;
        } tileset;

        struct CurrentSelection {
            bool changed = false;
            uengine::graphics::GraphicsQuads * quads;
        } currentSelection;

        struct PreviousSelection {
            bool changed = false;
            uengine::graphics::GraphicsQuads * quads;
        } previousSelection;

        // Descriptor pool
        void setupDescriptorPool();
        
        // Render Pass
        void setupRenderPass();

        // Offscreen methods
        void setupOffscreen(int32_t width, int32_t height);
        void destroyOffscreen();
     
        // Grid methods
        void setupGrid();
        void setupGridBuffers();
        void setupGridDescriptorSetLayout();
        void setupGridDescriptorSet();
        void setupGridPipeline();
        void updateGridBuffers();
        void destroyGrid();

        // Tileset methods
        void setupTileset();
        void setupTilesetBuffers();
        void setupTilesetDescriptorSetLayout();
        void setupTilesetDescriptorSet();
        void setupTilesetPipeline();
        void updateTilesetBuffers();
        void destroyTileset();
    };

}

#endif
