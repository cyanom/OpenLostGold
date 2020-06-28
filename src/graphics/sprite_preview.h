#ifndef SPRITE_PREVIEW_H
#define SPRITE_PREVIEW_H

#include <array>
#include <chrono>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "sprite.h"
#include "graphics_base.h"
#include "graphics_helper.h"

namespace uengine::graphics {

    class SpritePreview {
    public:
        SpritePreview(GraphicsBase * gb, Sprite * sprite, int32_t w, int32_t h);
        ~SpritePreview();

        ImTextureID getTexture();
        void update();
        void setViewProjection(glm::mat4 vp);
        void setBackgroundColor(float color[4]);
        SpriteBox * getSpriteBox();

        void render(VkCommandBuffer cb);

    private:
        uengine::graphics::GraphicsBase * gb;
        
        SpriteBox * spriteBox;
        glm::mat4 vp;
        float color[4] = {0, 0, 0, 0};
        std::chrono::time_point<std::chrono::high_resolution_clock> lastTime = std::chrono::high_resolution_clock::now();

        VkDescriptorPool descriptorPool;
        VkRenderPass renderPass;

        struct Offscreen {
            int32_t width=0, height=0;
            VkDeviceMemory mem;
            VkImage image;
            VkImageView view;
            VkFramebuffer frameBuffer;		
            VkSampler sampler;
            ImTextureID texture;
        } offscreen;

        struct directVPData {
            alignas(16) glm::mat4 directVP;
        } directVPData;

        VkBuffer directVPBuffer;
        VkDeviceMemory directVPMemory;
        VkBuffer spriteBoxBuffer;
        VkDeviceMemory spriteBoxMemory;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;

        // Descriptor pool
        void setupDescriptorPool();
        
        // Render Pass
        void setupRenderPass();

        // Offscreen methods
        void setupOffscreen(int32_t width, int32_t height);
        void destroyOffscreen();

        // Sprite
        void setupBuffers();
        void setupDescriptorSetLayout();
        void setupDescriptorSet();
        void setupPipeline();
    };

}

#endif
