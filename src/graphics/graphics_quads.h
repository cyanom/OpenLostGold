#ifndef GRAPHICS_QUADS_H
#define GRAPHICS_QUADS_H

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "graphics_base.h"
#include "graphics_helper.h"

namespace uengine::graphics {

    class GraphicsQuads {
        public:
            GraphicsQuads(GraphicsBase * gb, VkRenderPass * renderPass, int nbQuads, float lineWidth, bool filled);
            ~GraphicsQuads();
            
            void addQuad(float pos1[2], float pos2[2], float pos3[2], float pos4[2], float color[4]);
            void addRect(float pos[2], float size[2], float color[4]);
            void clear();

            void updateView(glm::mat4 vp);
            void render(VkCommandBuffer cb);

        private:
            GraphicsBase * gb;
            VkRenderPass * renderPass;
            int nbQuads;
            float lineWidth;
            int nbAdded;

            // VERTICES
            struct Vertex {
                glm::vec2 pos;
                glm::vec4 color;
            };
            std::vector<Vertex> vertices;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexMemory;
            // INDICES
            std::vector<uint16_t> indices;
            VkBuffer indexBuffer;
            VkDeviceMemory indexMemory;
            // VP MATRIX
            struct DirectVPData {
                glm::mat4 directVP;
            } directVPData;
            VkBuffer directVPBuffer;
            VkDeviceMemory directVPMemory;

            VkDescriptorPool descriptorPool;
            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorSet descriptorSet;
            VkPipelineLayout pipelineLayout;
            VkPipeline pipeline;

            void updateQuads();

            void setupVertexBuffer();
            void setupIndexBuffer();
            void setupUBO();
            void setupDescriptorPool();
            void setupDescriptorSetLayout();
            void setupDescriptorSet();
            void setupPipeline();
    };

}

#endif
