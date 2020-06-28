#include "graphics_quads.h"

using GraphicsQuads = uengine::graphics::GraphicsQuads;
using GraphicsBase = uengine::graphics::GraphicsBase;
namespace gh = uengine::graphics::helper;

GraphicsQuads::GraphicsQuads(GraphicsBase * gb_, VkRenderPass * renderPass_, int nbQuadsMax_, float lineWidth_) {
    gb = gb_;
    renderPass = renderPass_;
    nbQuadsMax = nbQuadsMax_; 
    nbWire = 0;
    nbFilled = 0;
    lineWidth = lineWidth_;

    setupVertexBuffers();
    setupIndexBuffers();
    setupUBO();
    setupDescriptorPool();
    setupDescriptorSetLayout();
    setupDescriptorSet();
    setupWirePipeline();
    setupFilledPipeline();
}

GraphicsQuads::~GraphicsQuads() {
    gb->destroyBuffer(wireVertexBuffer, nullptr);
    gb->destroyBuffer(filledVertexBuffer, nullptr);
    gb->freeMemory(wireVertexMemory, nullptr);
    gb->freeMemory(filledVertexMemory, nullptr);
    gb->destroyBuffer(wireIndexBuffer, nullptr);
    gb->destroyBuffer(filledIndexBuffer, nullptr);
    gb->freeMemory(wireIndexMemory, nullptr);
    gb->freeMemory(filledIndexMemory, nullptr);
    gb->destroyBuffer(uboBuffer, nullptr);
    gb->freeMemory(uboMemory, nullptr);
    gb->destroyPipeline(wirePipeline, nullptr);
    gb->destroyPipeline(filledPipeline, nullptr);
    gb->destroyPipelineLayout(wirePipelineLayout, nullptr);
    gb->destroyPipelineLayout(filledPipelineLayout, nullptr);
    gb->destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
    gb->destroyDescriptorPool(descriptorPool, nullptr);
}

void GraphicsQuads::addQuad(float pos1[2], float pos2[2], float pos3[2], float pos4[2], float color[4], bool filled) {
    struct Quad quad = {
        {{glm::make_vec2(pos1), glm::make_vec4(color)},
         {glm::make_vec2(pos2), glm::make_vec4(color)},
         {glm::make_vec2(pos3), glm::make_vec4(color)},
         {glm::make_vec2(pos4), glm::make_vec4(color)}},
        filled
    };

    quads.push_back(quad);
    update();
}

void GraphicsQuads::addRect(float pos1[2], float pos2[2], float color[4], bool filled) {
    struct Quad quad = {
        {{glm::vec2(pos1[0], pos1[1]), glm::make_vec4(color)},
         {glm::vec2(pos1[0], pos2[1]), glm::make_vec4(color)},
         {glm::vec2(pos2[0], pos2[1]), glm::make_vec4(color)},
         {glm::vec2(pos2[0], pos1[1]), glm::make_vec4(color)}},
        filled
    };
    quads.push_back(quad);
    update();
}

void GraphicsQuads::addRect(float x1, float y1, float x2, float y2, float r, float g, float b, float a, bool filled) {
    struct Quad quad = {
        {{glm::vec2(x1, y1), glm::vec4(r, g, b, a)},
         {glm::vec2(x1, y2), glm::vec4(r, g, b, a)},
         {glm::vec2(x2, y2), glm::vec4(r, g, b, a)},
         {glm::vec2(x2, y1), glm::vec4(r, g, b, a)}},
        filled
    };
    quads.push_back(quad);
    update();
}

void GraphicsQuads::clear() {
    quads.clear();
    update();
}

void GraphicsQuads::update() {
    Vertex * wireVertexData;
    Vertex * filledVertexData;
    uint16_t * wireIndexData;
    uint16_t * filledIndexData;
    
    nbFilled = 0;
    nbWire = 0;

    gb->mapMemory(wireVertexMemory, 0, sizeof(Vertex) * nbQuadsMax * 4, 0, (void **) &wireVertexData);
    gb->mapMemory(filledVertexMemory, 0, sizeof(Vertex) * nbQuadsMax * 4, 0, (void **) &filledVertexData);
    gb->mapMemory(wireIndexMemory, 0, sizeof(uint16_t) * nbQuadsMax * 6, 0, (void **) &wireIndexData);
    gb->mapMemory(filledIndexMemory, 0, sizeof(uint16_t) * nbQuadsMax * 5, 0, (void **) &filledIndexData);
    for (auto quad : quads) {
        if (quad.filled) {
            memcpy((void *) (filledVertexData + nbFilled * 4), quad.vertices, sizeof(Vertex) * 4);
            uint16_t indices[5] = {
                (uint16_t) (nbFilled * 4),
                (uint16_t) (nbFilled * 4 + 1),
                (uint16_t) (nbFilled * 4 + 2),
                (uint16_t) (nbFilled * 4 + 3),
                (uint16_t) 0xffff
            };
            memcpy((void *) (filledIndexData + 5 * nbFilled), indices, sizeof(uint16_t) * 5);
            nbFilled++;
        } else {
            memcpy((void *) (wireVertexData + nbWire * 4), quad.vertices, sizeof(Vertex) * 4);
            uint16_t indices[6] = {
                (uint16_t) (nbWire * 4),
                (uint16_t) (nbWire * 4 + 1),
                (uint16_t) (nbWire * 4 + 2),
                (uint16_t) (nbWire * 4 + 3),
                (uint16_t) (nbWire * 4),
                (uint16_t) 0xffff
            };
            memcpy((void *) (wireIndexData + 6 * nbWire), indices, sizeof(uint16_t) * 6);
            nbWire++;
        }
    }
    gb->unmapMemory(wireIndexMemory);
    gb->unmapMemory(filledIndexMemory);
    gb->unmapMemory(wireVertexMemory);
    gb->unmapMemory(filledVertexMemory);
}

void GraphicsQuads::setViewProjection(glm::mat4 vp) {
    ubo.vp = vp;

    void * data;
    gb->mapMemory(uboMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    gb->unmapMemory(uboMemory);
}

void GraphicsQuads::render(VkCommandBuffer cb) {
    VkDeviceSize offsets[] = {0};
    
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, filledPipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, filledPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    VkBuffer filledVertexBuffers[] = {filledVertexBuffer};
    vkCmdBindVertexBuffers(cb, 0, 1, filledVertexBuffers, offsets);
    vkCmdBindIndexBuffer(cb, filledIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cb, nbFilled * 5, 1, 0, 0, 0);

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, wirePipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, wirePipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    VkBuffer wireVertexBuffers[] = {wireVertexBuffer};
    vkCmdBindVertexBuffers(cb, 0, 1, wireVertexBuffers, offsets);
    vkCmdBindIndexBuffer(cb, wireIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cb, nbWire * 6, 1, 0, 0, 0);
}

void GraphicsQuads::setupDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        gh::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16),
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
        gh::descriptorPoolCreateInfo(
            poolSizes.size(),
            poolSizes.data(),
            1);

    gb->createDescriptorPool(&descriptorPoolInfo, nullptr, &descriptorPool);
}

void GraphicsQuads::setupDescriptorSetLayout() {
    // Binding
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0),
    };

    // Layouts
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = gh::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint16_t>(setLayoutBindings.size()));
    gb->createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &descriptorSetLayout);
}

void GraphicsQuads::setupDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo =
        gh::descriptorSetAllocateInfo(
            descriptorPool,
            &descriptorSetLayout,
            1);

    gb->allocateDescriptorSets(&allocInfo, &descriptorSet);

    VkDescriptorBufferInfo bufferInfo =
        gh::descriptorBufferInfo(uboBuffer, 0, sizeof(ubo));
    
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        gh::writeDescriptorSet(descriptorSet,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                               0,
                               &bufferInfo)
    };

    gb->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void GraphicsQuads::setupWirePipeline() {
    // Shaders
    VkShaderModule vertShaderModule = gb->createShaderModule("res/shaders/quads/vert.spv");
    VkShaderModule fragShaderModule = gb->createShaderModule("res/shaders/quads/frag.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = gh::pipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gh::pipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputState = gh::pipelineVertexInputStateCreateInfo();
    
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    
    bindingDescriptions = {
        gh::vertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
    };
    attributeDescriptions = {
        gh::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos)),
        gh::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)),
    };

    vertexInputState.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        gh::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
            0,
            VK_TRUE);

    // Viewport
    VkPipelineViewportStateCreateInfo viewportState = gh::pipelineViewportStateCreateInfo(1, 1, 0);

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizationState =
        gh::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_LINE,
            VK_CULL_MODE_FRONT_BIT,
            VK_FRONT_FACE_CLOCKWISE,
            0);
    rasterizationState.lineWidth = lineWidth;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleState = 
        gh::pipelineMultisampleStateCreateInfo(
            VK_SAMPLE_COUNT_1_BIT,
            0);
    
    // Color blend attachment
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = 
        gh::pipelineColorBlendAttachmentState(
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            VK_TRUE);
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_MAX;

    // Color blend state
    VkPipelineColorBlendStateCreateInfo colorBlendState =
        gh::pipelineColorBlendStateCreateInfo(
            1,
            &colorBlendAttachmentState);

    // Dynamic state
    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState =
        gh::pipelineDynamicStateCreateInfo(
            dynamicStateEnables.data(),
            dynamicStateEnables.size(),
            0);

    // Pipeline layout creation
    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
        gh::pipelineLayoutCreateInfo(
            &descriptorSetLayout,
            1);

    gb->createPipelineLayout(&pipelineLayoutInfo, nullptr, &wirePipelineLayout);

    // Actual pipeline creation
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gh::pipelineCreateInfo(wirePipelineLayout, *renderPass, 0);
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    
    gb->createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &wirePipeline);

    // Cleanup
    gb->destroyShaderModule(vertShaderModule, nullptr);
    gb->destroyShaderModule(fragShaderModule, nullptr);
}

void GraphicsQuads::setupFilledPipeline() {
    // Shaders
    VkShaderModule vertShaderModule = gb->createShaderModule("res/shaders/quads/vert.spv");
    VkShaderModule fragShaderModule = gb->createShaderModule("res/shaders/quads/frag.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = gh::pipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gh::pipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputState = gh::pipelineVertexInputStateCreateInfo();
    
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    
    bindingDescriptions = {
        gh::vertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
    };
    attributeDescriptions = {
        gh::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos)),
        gh::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)),
    };

    vertexInputState.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        gh::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            0,
            VK_TRUE);

    // Viewport
    VkPipelineViewportStateCreateInfo viewportState = gh::pipelineViewportStateCreateInfo(1, 1, 0);

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizationState =
        gh::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE,
            VK_FRONT_FACE_CLOCKWISE,
            0);

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleState = 
        gh::pipelineMultisampleStateCreateInfo(
            VK_SAMPLE_COUNT_1_BIT,
            0);
    
    // Color blend attachment
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = 
        gh::pipelineColorBlendAttachmentState(
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            VK_TRUE);
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_MAX;

    // Color blend state
    VkPipelineColorBlendStateCreateInfo colorBlendState =
        gh::pipelineColorBlendStateCreateInfo(
            1,
            &colorBlendAttachmentState);

    // Dynamic state
    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState =
        gh::pipelineDynamicStateCreateInfo(
            dynamicStateEnables.data(),
            dynamicStateEnables.size(),
            0);

    // Pipeline layout creation
    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
        gh::pipelineLayoutCreateInfo(
            &descriptorSetLayout,
            1);

    gb->createPipelineLayout(&pipelineLayoutInfo, nullptr, &filledPipelineLayout);

    // Actual pipeline creation
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gh::pipelineCreateInfo(filledPipelineLayout, *renderPass, 0);
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    
    gb->createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &filledPipeline);

    // Cleanup
    gb->destroyShaderModule(vertShaderModule, nullptr);
    gb->destroyShaderModule(fragShaderModule, nullptr);
}

void GraphicsQuads::setupVertexBuffers() {
    gb->createBuffer(nbQuadsMax * 4 * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     wireVertexBuffer, wireVertexMemory);
    gb->createBuffer(nbQuadsMax * 4 * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     filledVertexBuffer, filledVertexMemory);
}

void GraphicsQuads::setupIndexBuffers() {
    gb->createBuffer(nbQuadsMax * 6 * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     wireIndexBuffer, wireIndexMemory);
    gb->createBuffer(nbQuadsMax * 5 * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     filledIndexBuffer, filledIndexMemory);
}

void GraphicsQuads::setupUBO() {
    gb->createBuffer(sizeof(ubo),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uboBuffer,
                     uboMemory);
}
