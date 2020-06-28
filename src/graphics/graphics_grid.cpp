#include "graphics_grid.h"

using GraphicsGrid = uengine::graphics::GraphicsGrid;
using GraphicsBase = uengine::graphics::GraphicsBase;
namespace gh = uengine::graphics::helper;

GraphicsGrid::GraphicsGrid(GraphicsBase * gb_, VkRenderPass * renderPass_) {
    gb = gb_;
    renderPass = renderPass_;
    model = glm::mat4(1.0f);
    vp = glm::mat4(1.0f);
    ubo = {};

    setupUBO();
    setupDescriptorPool();
    setupDescriptorSetLayout();
    setupDescriptorSet();
    setupPipeline();
}

GraphicsGrid::~GraphicsGrid() {
    gb->destroyBuffer(buffer, nullptr);
    gb->freeMemory(memory, nullptr);
    gb->destroyPipeline(pipeline, nullptr);
    gb->destroyPipelineLayout(pipelineLayout, nullptr);
    gb->destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
    gb->destroyDescriptorPool(descriptorPool, nullptr);
}

/*
 *  External methods
 */

void GraphicsGrid::setBoundaries(float pos1[2], float pos2[2]) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3((pos2[0] + pos1[0]) / 2, (pos2[1] + pos1[1]) / 2, 1.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(std::abs(pos2[0] - pos1[0]) / 2, std::abs(pos2[1] - pos1[1]) / 2, 1.0f));
    ubo.invMVP = glm::inverse(vp * model);
    ubo.model = model;
    updateUBO();
}

void GraphicsGrid::setColor1(float color[4]) {
    ubo.color1 = glm::make_vec4(color);
    updateUBO();
}

void GraphicsGrid::setColor2(float color[4]) {
    ubo.color2 = glm::make_vec4(color);
    updateUBO();
}

void GraphicsGrid::setXOffset(float offset) {
    ubo.offset = glm::vec2(ubo.offset.x, offset);
    updateUBO();
}

void GraphicsGrid::setYOffset(float offset) {
    ubo.offset = glm::vec2(offset, ubo.offset.y);
    updateUBO();
}

void GraphicsGrid::setOffset(float offset[2]) {
    ubo.offset = glm::make_vec2(offset);
    updateUBO();
}

void GraphicsGrid::setXTileSize(float tileSize) {
    ubo.tileSize = glm::vec2(ubo.tileSize.x, tileSize);
    updateUBO();
}

void GraphicsGrid::setYTileSize(float tileSize) {
    ubo.tileSize = glm::vec2(tileSize, ubo.tileSize.y);
    updateUBO();
}

void GraphicsGrid::setTileSize(float tileSize[2]) {
    ubo.tileSize = glm::make_vec2(tileSize);
    updateUBO();
}

void GraphicsGrid::setScreenSize(float screenSize[2]) {
    ubo.screenSize = glm::make_vec2(screenSize);
    updateUBO();
}

void GraphicsGrid::setExtended(bool state) {
    ubo.extended = state;
    updateUBO();
}

void GraphicsGrid::setViewProjection(glm::mat4 vp_) {
    vp = vp_;
    ubo.invMVP = glm::inverse(vp * model);
    updateUBO();
}

void GraphicsGrid::render(VkCommandBuffer cb) {
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdDraw(cb, 6, 1, 0, 0);
}

/*
 *  Internal methods
 */

void GraphicsGrid::setupDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        gh::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16),
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
        gh::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 1);

    gb->createDescriptorPool(&descriptorPoolInfo, nullptr, &descriptorPool);
}

void GraphicsGrid::setupDescriptorSetLayout() {
    // Binding
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        gh::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                       VK_SHADER_STAGE_FRAGMENT_BIT,
                                       0),
    };

    // Layouts
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = gh::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint16_t>(setLayoutBindings.size()));
    gb->createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &descriptorSetLayout);
}

void GraphicsGrid::setupDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo =
        gh::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

    gb->allocateDescriptorSets(&allocInfo, &descriptorSet);

    VkDescriptorBufferInfo bufferInfo =
        gh::descriptorBufferInfo(buffer, 0, sizeof(ubo));
    
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        gh::writeDescriptorSet(descriptorSet,
                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                               0,
                               &bufferInfo)
    };

    gb->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void GraphicsGrid::setupPipeline() {
    // Shaders
    VkShaderModule vertShaderModule = gb->createShaderModule("res/shaders/grid/vert.spv");
    VkShaderModule fragShaderModule = gb->createShaderModule("res/shaders/grid/frag.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = gh::pipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gh::pipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputState = gh::pipelineVertexInputStateCreateInfo();
    vertexInputState.vertexBindingDescriptionCount = 0;
    vertexInputState.pVertexBindingDescriptions = nullptr;
    vertexInputState.vertexAttributeDescriptionCount = 0;
    vertexInputState.pVertexAttributeDescriptions = nullptr;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        gh::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            0,
            VK_FALSE);

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

    gb->createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout);

    // Actual pipeline creation
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gh::pipelineCreateInfo(pipelineLayout, *renderPass, 0);
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    
    gb->createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);

    // Cleanup
    gb->destroyShaderModule(vertShaderModule, nullptr);
    gb->destroyShaderModule(fragShaderModule, nullptr);
}

void GraphicsGrid::setupUBO() {
    gb->createBuffer(sizeof(ubo),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     buffer,
                     memory);
}

void GraphicsGrid::updateUBO() {
    void * data;
    gb->mapMemory(memory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    gb->unmapMemory(memory);
}
