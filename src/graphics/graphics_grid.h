#ifndef GRAPHICS_GRID_H
#define GRAPHICS_GRID_H

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "graphics_base.h"
#include "graphics_helper.h"

namespace uengine::graphics {

    class GraphicsGrid {
        public:
            GraphicsGrid(GraphicsBase * gb, VkRenderPass * renderPass);
            ~GraphicsGrid();
            
            void setBoundaries(float pos1[2], float pos2[2]);
            void setColor1(float color[4]);
            void setColor2(float color[4]);
            void setXOffset(float offset);
            void setYOffset(float offset);
            void setOffset(float offset[2]);
            void setXTileSize(float tileSize);
            void setYTileSize(float tileSize);
            void setTileSize(float tileSize[2]);
            void setScreenSize(float screenSize[2]);
            void setExtended(bool state);
            void setViewProjection(glm::mat4 vp);
            
            void render(VkCommandBuffer cb);

        private:
            GraphicsBase * gb;
            VkRenderPass * renderPass;

            glm::mat4 vp;
            glm::mat4 model;
            struct {
                alignas(16) glm::mat4 invMVP;
                alignas(16) glm::mat4 model;
                alignas(16) glm::vec4 color1;
                alignas(16) glm::vec4 color2;
                alignas(8) glm::vec2 offset;
                alignas(8) glm::vec2 tileSize;
                alignas(8) glm::vec2 screenSize;
                alignas(4) float extended;
            } ubo;
            VkBuffer buffer;
            VkDeviceMemory memory;

            VkDescriptorPool descriptorPool;
            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSet descriptorSet;
            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;

            void setupDescriptorPool();
            void setupDescriptorSetLayout();
            void setupDescriptorSet();
            void setupPipeline();
            void setupUBO();
            void updateUBO();
    };

}

#endif
