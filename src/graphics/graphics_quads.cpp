#include "graphics_quads.h"

using GraphicsQuads = uengine::graphics::GraphicsQuads;
using GraphicsBase = uengine::graphics::GraphicsBase;
namespace gh = uengine::graphics::helper;

GraphicsQuads::GraphicsQuads(GraphicsBase * gb_, VkRenderPass * renderPass_, int nbQuads_, float lineWidth_, bool filled) {
    gb = gb_;
    renderPass = renderPass_;
    nbQuads = nbQuads_; 
    nbAdded = 0;
    vertices = std::vector<Vertex>();
    indices = std::vector<uint16_t>();
    lineWidth = lineWidth_;

    setupVertexBuffer();
    setupIndexBuffer();
    setupUBO();
    setupDescriptorPool();
    setupDescriptorSetLayout();
    setupDescriptorSet();
    setupPipeline();
}

GraphicsQuads::~GraphicsQuads() {
    gb->destroyBuffer(vertexBuffer, nullptr);
    gb->freeMemory(vertexMemory, nullptr);
    gb->destroyBuffer(indexBuffer, nullptr);
    gb->freeMemory(indexMemory, nullptr);
    gb->destroyBuffer(uboBuffer, nullptr);
    gb->freeMemory(uboMemory, nullptr);
    gb->destroyPipeline(pipeline, nullptr);
    gb->destroyPipelineLayout(pipelineLayout, nullptr);
    gb->destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
    gb->destroyDescriptorPool(descriptorPool, nullptr);
}

void GraphicsQuads::addQuad(float pos1[2], float pos2[2], float pos3[2], float pos4[2], float color[4]) {
    glm::vec2 p1 = glm::make_vec2(pos1);
    glm::vec2 p2 = glm::make_vec2(pos2);
    glm::vec2 p3 = glm::make_vec2(pos3);
    glm::vec2 p4 = glm::make_vec2(pos4);
    
    glm::vec4 c = glm::make_vec4(color);
        
    vertices.push_back({p1, c});
    vertices.push_back({p2, c});
    vertices.push_back({p3, c});
    vertices.push_back({p4, c});

    indices.push_back(nbAdded * 4);
    indices.push_back(nbAdded * 4 + 1);
    indices.push_back(nbAdded * 4 + 2);
    indices.push_back(nbAdded * 4 + 3);
    indices.push_back(nbAdded * 4);
    indices.push_back(0xffff);

    update();

    nbAdded++;
}

void GraphicsQuads::addRect(float pos1[2], float pos2[2], float color[4]) {
    glm::vec2 p1 = glm::vec2(pos1[0], pos1[1]);
    glm::vec2 p2 = glm::vec2(pos1[0], pos2[1]);
    glm::vec2 p3 = glm::vec2(pos2[0], pos2[1]);
    glm::vec2 p4 = glm::vec2(pos2[0], pos1[1]);
    
    glm::vec4 c = glm::make_vec4(color);
        
    vertices.push_back({p1, c});
    vertices.push_back({p2, c});
    vertices.push_back({p3, c});
    vertices.push_back({p4, c});

    indices.push_back(nbAdded * 4);
    indices.push_back(nbAdded * 4 + 1);
    indices.push_back(nbAdded * 4 + 2);
    indices.push_back(nbAdded * 4 + 3);
    indices.push_back(nbAdded * 4);
    indices.push_back(0xffff);
    
    update();

    nbAdded++;
}

void GraphicsQuads::addRect(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
    glm::vec2 p1 = glm::vec2(x1, y1);
    glm::vec2 p2 = glm::vec2(x1, y2);
    glm::vec2 p3 = glm::vec2(x2, y2);
    glm::vec2 p4 = glm::vec2(x2, y1);
    
    glm::vec4 c = glm::vec4(r, g, b, a);
        
    vertices.push_back({p1, c});
    vertices.push_back({p2, c});
    vertices.push_back({p3, c});
    vertices.push_back({p4, c});

    indices.push_back(nbAdded * 4);
    indices.push_back(nbAdded * 4 + 1);
    indices.push_back(nbAdded * 4 + 2);
    indices.push_back(nbAdded * 4 + 3);
    indices.push_back(nbAdded * 4);
    indices.push_back(0xffff);
    
    update();

    nbAdded++;
}

void GraphicsQuads::clear() {
    vertices.clear();
    indices.clear();
    nbAdded = 0;
}

void GraphicsQuads::setViewProjection(glm::mat4 vp) {
    ubo.directVP = vp;

    void * data;
    gb->mapMemory(uboMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    gb->unmapMemory(uboMemory);
}

void GraphicsQuads::render(VkCommandBuffer cb) {
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cb, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cb, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
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

void GraphicsQuads::setupPipeline() {
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
    rasterizationState.lineWidth = 3.0;

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

void GraphicsQuads::setupVertexBuffer() {
    gb->createBuffer(nbQuads * 4 * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vertexBuffer, vertexMemory);
}

void GraphicsQuads::setupIndexBuffer() {
    gb->createBuffer(nbQuads * 6 * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     indexBuffer, indexMemory);
}

void GraphicsQuads::setupUBO() {
    gb->createBuffer(sizeof(ubo),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uboBuffer,
                     uboMemory);
}

void GraphicsQuads::update() {
    void * data;
    
    // VERTICES
    gb->mapMemory(vertexMemory, 0, sizeof(Vertex) * vertices.size(), 0, &data);
    memcpy(data, vertices.data(), sizeof(Vertex) * vertices.size());
    gb->unmapMemory(vertexMemory);
    
    // INDICES
    gb->mapMemory(indexMemory, 0, sizeof(uint16_t) * indices.size(), 0, &data);
    memcpy(data, indices.data(), sizeof(uint16_t) * indices.size());
    gb->unmapMemory(indexMemory);
}
