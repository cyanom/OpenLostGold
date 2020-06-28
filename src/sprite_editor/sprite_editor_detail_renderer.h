#ifndef SPRITE_EDITOR_DETAIl_RENDERER_H
#define SPRITE_EDITOR_DETAIL_RENDERER_H

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

    class SpriteEditorDetailRenderer {
    public:
        SpriteEditorDetailRenderer(uengine::graphics::GraphicsBase * gb);
        ~SpriteEditorDetailRenderer();

        void setBackgroundColor(float color[4]);
        void setVP(glm::mat4 vp);
        
        void setGridModel(glm::mat4 model);
        void setGridColor(float color1[4], float color2[4]);
        void setGridViewSize(float viewSize[2]);
        void setGridPixelSize(float unitSize[2]);
        void setGridOffset(float offset[2]);
        void setGridExtended(bool extented);
 
        void setBoxModel(glm::mat4 model);
        void setBoxColor(float color[4]);

        void setSprite(uengine::graphics::Sprite * sprite);
        SpriteBox * getSpriteBox();

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

            glm::mat4 model;

            struct InverseMVPData {
                glm::mat4 inverseMVP;
            } inverseMVPData;

            struct GridParamData {
                alignas(16) glm::vec4 color1;
                alignas(16) glm::vec4 color2;
                alignas(8) glm::vec2 viewSize;
                alignas(8) glm::vec2 pixelSize;
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

        struct Box {
            bool changed = false;

            struct directVPData {
                glm::mat4 directVP;
            } directVPData;

            struct BoxData {
                alignas(16) glm::mat4 model;
                alignas(16) glm::vec4 color;
            } boxData;

            VkBuffer directVPBuffer;
            VkBuffer boxDataBuffer;
            VkDeviceMemory directVPMemory;
            VkDeviceMemory boxDataMemory;

            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSet descriptorSet;
            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;
        } box;
        
        struct {
            bool changed = false;

            struct directVPData {
                alignas(16) glm::mat4 directVP;
            } directVPData;
        
            uengine::graphics::SpriteBox * spriteBox;

            VkBuffer directVPBuffer;
            VkBuffer spriteBoxBuffer;
            VkDeviceMemory directVPMemory;
            VkDeviceMemory spriteBoxMemory;

            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSet descriptorSet;
            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;
        } spriteBoxData;

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

        // Box methods
        void setupBox();
        void setupBoxBuffers();
        void setupBoxDescriptorSetLayout();
        void setupBoxDescriptorSet();
        void setupBoxPipeline();
        void updateBoxBuffers();
        void destroyBox();

        // SpriteBox methods
        void setupSpriteBox();
        void setupSpriteBoxBuffers();
        void setupSpriteBoxDescriptorSetLayout();
        void setupSpriteBoxDescriptorSet();
        void setupSpriteBoxPipeline();
        void updateSpriteBoxBuffers();
        void destroySpriteBox();
    };

}

#endif
