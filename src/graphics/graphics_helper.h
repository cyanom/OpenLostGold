#ifndef GRAPHICS_HELPER_H
#define GRAPHICS_HELPER_H

#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <set>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <chrono>

#include <vulkan/vulkan.h>

namespace uengine::graphics::helper {

    /* Descriptor */

    VkDescriptorPoolSize descriptorPoolSize(
        VkDescriptorType type,
        uint32_t descriptorCount);

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
        uint32_t poolSizeCount,
        VkDescriptorPoolSize* pPoolSizes,
        uint32_t maxSets);

    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
        VkDescriptorType type,
        VkShaderStageFlags stageFlags,
        uint32_t binding,
        uint32_t descriptorCount = 1);

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
        const VkDescriptorSetLayoutBinding * pBindings,
        uint32_t bindingCount = 1);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
        VkDescriptorPool descriptorPool,
        const VkDescriptorSetLayout * pSetLayouts,
        uint32_t descriptorSetCount = 1);

    VkDescriptorBufferInfo descriptorBufferInfo(
        VkBuffer buffer,
        VkDeviceSize offset,
        VkDeviceSize range);

    VkDescriptorImageInfo descriptorImageInfo(
        VkSampler sampler,
        VkImageView imageView,
        VkImageLayout imageLayout);

    VkWriteDescriptorSet writeDescriptorSet(
        VkDescriptorSet dstSet,
        VkDescriptorType type,
        uint32_t binding,
        VkDescriptorBufferInfo * bufferInfo,
        uint32_t descriptorCount = 1);

    VkWriteDescriptorSet writeDescriptorSet(
        VkDescriptorSet dstSet,
        VkDescriptorType type,
        uint32_t binding,
        VkDescriptorImageInfo * imageInfo,
        uint32_t descriptorCount = 1);


    /* Shaders */

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits stage);


    /* Common structures */

    VkViewport viewport(
        float width,
        float height,
        float minDepth,
        float maxDepth);

    VkRect2D rect2D(
        int32_t width,
        int32_t height,
        int32_t offsetX,
        int32_t offsetY);


    /* Render pass */

    VkRenderPassBeginInfo renderPassBeginInfo();


    /* Images/Framebuffers */
    
    VkImageCreateInfo imageCreateInfo();

    VkMemoryAllocateInfo memoryAllocateInfo();

    VkImageViewCreateInfo imageViewCreateInfo();

    VkSamplerCreateInfo samplerCreateInfo();

    VkFramebufferCreateInfo frameBufferCreateInfo();


    /* Pipelines */

    // Pipeline creation structure
    VkGraphicsPipelineCreateInfo pipelineCreateInfo(
        VkPipelineLayout layout,
        VkRenderPass renderPass,
        VkPipelineCreateFlags flags = 0);

    // Vertex input
    VkVertexInputBindingDescription vertexInputBindingDescription(
        uint32_t binding,
        uint32_t stride,
        VkVertexInputRate inputRate);

    VkVertexInputAttributeDescription vertexInputAttributeDescription(
        uint32_t binding,
        uint32_t location,
        VkFormat format,
        uint32_t offset);

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
        VkPrimitiveTopology topology,
        VkPipelineInputAssemblyStateCreateFlags flags,
        VkBool32 primitiveRestartEnable);

    // Viewport
    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
        uint32_t viewportCount,
        uint32_t scissorCount,
        VkPipelineViewportStateCreateFlags flags = 0);

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
        VkPolygonMode polygonMode,
        VkCullModeFlags cullMode,
        VkFrontFace frontFace,
        VkPipelineRasterizationStateCreateFlags flags = 0);

    // Multisampling
    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
        VkSampleCountFlagBits rasterizationSamples,
        VkPipelineMultisampleStateCreateFlags flags = 0);

    // Color blend attachment
    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
        VkColorComponentFlags colorWriteMask,
        VkBool32 blendEnable);

    // Color blend state
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
        uint32_t attachmentCount,
        const VkPipelineColorBlendAttachmentState * pAttachments);

    // Dynamic state
    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
        const VkDynamicState * pDynamicStates,
        uint32_t dynamicStateCount,
        VkPipelineDynamicStateCreateFlags flags = 0);

    // Layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
        const VkDescriptorSetLayout* pSetLayouts,
        uint32_t setLayoutCount);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
        uint32_t setLayoutCount = 1);
}

#endif
