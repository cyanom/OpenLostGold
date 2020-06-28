#include "sprite_editor_overview_renderer.h"

using namespace uengine::sprite_editor;
using namespace uengine::graphics;
using GraphicsBase = uengine::graphics::GraphicsBase;
using GraphicsQuads = uengine::graphics::GraphicsQuads;
namespace gh = uengine::graphics::helper;


SpriteEditorOverviewRenderer::SpriteEditorOverviewRenderer(GraphicsBase * gb_) {
    gb = gb_;

    setupDescriptorPool();
    setupRenderPass();

    setupGrid();
    setupTileset();
    currentSelection.quads = new GraphicsQuads(gb, &renderPass, 1000, 1, false);
    previousSelection.quads = new GraphicsQuads(gb, &renderPass, 1000, 1, false);
}

SpriteEditorOverviewRenderer::~SpriteEditorOverviewRenderer() {
    destroyOffscreen();
    destroyGrid();
    destroyTileset();
    delete currentSelection.quads;
    delete previousSelection.quads;
    gb->destroyRenderPass(renderPass, nullptr);
    gb->destroyDescriptorPool(descriptorPool, nullptr);
}

void SpriteEditorOverviewRenderer::setBackgroundColor(float color[4]) {
    std::copy(color, color + 4, backgroundColor);
}

void SpriteEditorOverviewRenderer::setVP(glm::mat4 vp_) {
    vp = vp_;
    tileset.changed = true;
    grid.changed = true;
}

void SpriteEditorOverviewRenderer::setGridColor(float color1[4], float color2[4]) {
    grid.gridParamData.color1 = glm::make_vec4(color1);
    grid.gridParamData.color2 = glm::make_vec4(color2);
    grid.changed = true;
}

void SpriteEditorOverviewRenderer::setGridUnitSize(int unitSize[2]) {
    grid.gridParamData.unitSize = glm::make_vec4(unitSize);
    grid.changed = true;
}

void SpriteEditorOverviewRenderer::setGridTilesetSize(int tilesetSize[2]) {
    grid.gridParamData.tilesetSize = glm::make_vec4(tilesetSize);
    grid.changed = true;
}

void SpriteEditorOverviewRenderer::setGridViewSize(int viewSize[2]) {
    grid.gridParamData.viewSize = glm::make_vec4(viewSize);
    grid.changed = true;
}

void SpriteEditorOverviewRenderer::setGridOffset(int offset[2]) {
    grid.gridParamData.offset = glm::make_vec4(offset);
    grid.changed = true;
}

void SpriteEditorOverviewRenderer::setGridExtended(bool extended) {
    grid.gridParamData.extended = extended;
    grid.changed = true;
}


void SpriteEditorOverviewRenderer::setTilesetSprite(Sprite * sprite) {
    tileset.sprite = sprite;
    tileset.changed = true;
}

void SpriteEditorOverviewRenderer::setTilesetModel(glm::mat4 model) {
    tileset.model = model;
    tileset.changed = true;
}

void SpriteEditorOverviewRenderer::update() {
    if (tileset.changed) {
        updateTilesetBuffers();
        tileset.changed = false;
    }
    if (grid.changed) {
        updateGridBuffers();
        grid.changed = false;
    }
}

void SpriteEditorOverviewRenderer::render(VkCommandBuffer cb) {
    if (!offscreen.texture) {
        return;
    }
    
    VkClearValue clearValues = {0};
    std::copy(backgroundColor, backgroundColor + 4, clearValues.color.float32);

    VkRenderPassBeginInfo renderPassBeginInfo = gh::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = offscreen.frameBuffer;
    renderPassBeginInfo.renderArea.extent.width = offscreen.width;
    renderPassBeginInfo.renderArea.extent.height = offscreen.height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValues;

    vkCmdBeginRenderPass(cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = gh::viewport((float) offscreen.width, (float) offscreen.height, 0.0f, 1.0f);
    vkCmdSetViewport(cb, 0, 1, &viewport);

    VkRect2D scissor = gh::rect2D(offscreen.width, offscreen.height, 0, 0);
    vkCmdSetScissor(cb, 0, 1, &scissor);

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, grid.pipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, grid.pipelineLayout, 0, 1, &grid.descriptorSet, 0, nullptr);
    vkCmdDraw(cb, 6, 1, 0, 0);

    if (tileset.sprite && !tileset.changed) {
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, tileset.pipeline);
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, tileset.pipelineLayout, 0, 1, &tileset.descriptorSet, 0, nullptr);
        vkCmdDraw(cb, 6, 1, 0, 0);
    }

    currentSelection.quads->render(cb);
    previousSelection.quads->render(cb);

    vkCmdEndRenderPass(cb);
}

void SpriteEditorOverviewRenderer::resize(int32_t width, int32_t height) {
    if (offscreen.width == width && offscreen.height == height)
        return;

    if (offscreen.texture)
        destroyOffscreen();

    if (width == 0 || height == 0)
        return;
    
    setupOffscreen(width, height);
}

ImTextureID SpriteEditorOverviewRenderer::getTexture() {
    return offscreen.texture;
}

/* Descriptor pool */ 

void SpriteEditorOverviewRenderer::setupDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        gh::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16),
        gh::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
        gh::descriptorPoolCreateInfo(
            poolSizes.size(),
            poolSizes.data(),
            2);

    gb->createDescriptorPool(&descriptorPoolInfo, nullptr, &descriptorPool);
}


/*--------------------- Render pass ---------------------*/

void SpriteEditorOverviewRenderer::setupRenderPass() {
    // Render pass creation
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = nullptr;
    
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachmentDescription;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    gb->createRenderPass(&renderPassInfo, nullptr, &renderPass);
}

/*------------------ Offscreen ------------------*/

void SpriteEditorOverviewRenderer::setupOffscreen(int32_t width, int32_t height) {
    offscreen.width = width;
    offscreen.height = height;

    // Image creation
    VkImageCreateInfo image = gh::imageCreateInfo();
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = VK_FORMAT_R8G8B8A8_UNORM;
    image.extent.width = offscreen.width;
    image.extent.height = offscreen.height;
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    gb->createImage(&image, nullptr, &offscreen.image);

    // Image memory allocation
    VkMemoryAllocateInfo memAlloc = gh::memoryAllocateInfo();
    VkMemoryRequirements memReqs;
    gb->getImageMemoryRequirements(offscreen.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = gb->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    gb->allocateMemory(&memAlloc, nullptr, &offscreen.mem);
    
    gb->bindImageMemory(offscreen.image, offscreen.mem, 0);

    // Image view creation
    VkImageViewCreateInfo colorImageView = gh::imageViewCreateInfo();
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = offscreen.image;

    gb->createImageView(&colorImageView, nullptr, &offscreen.view);

    // Sampler creation
    VkSamplerCreateInfo samplerInfo = gh::samplerCreateInfo();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    gb->createSampler(&samplerInfo, nullptr, &offscreen.sampler);

    // Framebuffer creation
    VkFramebufferCreateInfo frameBufferCreateInfo = gh::frameBufferCreateInfo();
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 1;
    frameBufferCreateInfo.pAttachments = &offscreen.view;
    frameBufferCreateInfo.width = offscreen.width;
    frameBufferCreateInfo.height = offscreen.height;
    frameBufferCreateInfo.layers = 1;

    gb->createFramebuffer(&frameBufferCreateInfo, nullptr, &offscreen.frameBuffer);

    offscreen.texture = ImGui_ImplVulkan_AddTexture(offscreen.sampler, offscreen.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void SpriteEditorOverviewRenderer::destroyOffscreen() {
    gb->destroyFramebuffer(offscreen.frameBuffer, nullptr);
    gb->destroySampler(offscreen.sampler, nullptr);
    gb->destroyImageView(offscreen.view, nullptr);
    gb->destroyImage(offscreen.image, nullptr);
    gb->freeMemory(offscreen.mem, nullptr);
    offscreen.texture = nullptr;
}


/* ---------------------Grid ---------------------*/

void SpriteEditorOverviewRenderer::setupGrid() {
    setupGridBuffers();
    setupGridDescriptorSetLayout();
    setupGridDescriptorSet();
    setupGridPipeline();
}

void SpriteEditorOverviewRenderer::setupGridBuffers() {
    VkDeviceSize gridParamBufferSize = sizeof(grid.gridParamData);
    gb->createBuffer(
        gridParamBufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        grid.gridParamBuffer,
        grid.gridParamMemory);
    
    VkDeviceSize inverseMVPBufferSize = sizeof(grid.inverseMVPData);
    gb->createBuffer(
        inverseMVPBufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        grid.inverseMVPBuffer,
        grid.inverseMVPMemory);
}

void SpriteEditorOverviewRenderer::setupGridDescriptorSetLayout() {
    // Binding
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0),
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            1)
    };

    // Layouts
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = gh::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
    gb->createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &grid.descriptorSetLayout);
}

void SpriteEditorOverviewRenderer::setupGridDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo =
        gh::descriptorSetAllocateInfo(
            descriptorPool,
            &grid.descriptorSetLayout,
            1);

    gb->allocateDescriptorSets(&allocInfo, &grid.descriptorSet);

    VkDescriptorBufferInfo gridParamBufferInfo =
        gh::descriptorBufferInfo(
            grid.gridParamBuffer,
            0,
            sizeof(grid.gridParamData));
    
    VkDescriptorBufferInfo inverseMVPBufferInfo =
        gh::descriptorBufferInfo(
            grid.inverseMVPBuffer,
            0,
            sizeof(grid.inverseMVPData));
    
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        gh::writeDescriptorSet(
            grid.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            0,
            &gridParamBufferInfo),
        gh::writeDescriptorSet(
            grid.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            &inverseMVPBufferInfo)
    };

    gb->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void SpriteEditorOverviewRenderer::setupGridPipeline() {
    // Shaders
    VkShaderModule vertShaderModule = gb->createShaderModule("res/shaders/tileset_view/grid/vert.spv");
    VkShaderModule fragShaderModule = gb->createShaderModule("res/shaders/tileset_view/grid/frag.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = gh::pipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gh::pipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputState = gh::pipelineVertexInputStateCreateInfo();
    vertexInputState.vertexBindingDescriptionCount = 0;
    vertexInputState.vertexAttributeDescriptionCount = 0;

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
            VK_CULL_MODE_FRONT_BIT,
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
            &grid.descriptorSetLayout,
            1);

    gb->createPipelineLayout(&pipelineLayoutInfo, nullptr, &grid.pipelineLayout);

    // Actual pipeline creation
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gh::pipelineCreateInfo(grid.pipelineLayout, renderPass, 0);
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    
    gb->createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &grid.pipeline);

    // Cleanup
    gb->destroyShaderModule(vertShaderModule, nullptr);
    gb->destroyShaderModule(fragShaderModule, nullptr);
}

void SpriteEditorOverviewRenderer::updateGridBuffers() {
    glm::mat4 m = vp * tileset.model;
    grid.inverseMVPData.inverseMVP = glm::inverse(m);
    
    void * data;
    // MVP matrix
    gb->mapMemory(grid.inverseMVPMemory, 0, sizeof(grid.inverseMVPData), 0, &data);
        memcpy(data, &grid.inverseMVPData, sizeof(grid.inverseMVPData));
    gb->unmapMemory(grid.inverseMVPMemory);
    
    // Parameters
    gb->mapMemory(grid.gridParamMemory, 0, sizeof(grid.gridParamData), 0, &data);
        memcpy(data, &grid.gridParamData, sizeof(grid.gridParamData));
    gb->unmapMemory(grid.gridParamMemory);
}

void SpriteEditorOverviewRenderer::destroyGrid() {
    gb->destroyPipeline(grid.pipeline, nullptr);
    gb->destroyPipelineLayout(grid.pipelineLayout, nullptr);
    gb->destroyBuffer(grid.inverseMVPBuffer, nullptr);
    gb->destroyBuffer(grid.gridParamBuffer, nullptr);
    gb->freeMemory(grid.inverseMVPMemory, nullptr);
    gb->freeMemory(grid.gridParamMemory, nullptr);
    gb->destroyDescriptorSetLayout(grid.descriptorSetLayout, nullptr);
}


/*--------------------- Tileset ---------------------*/

void SpriteEditorOverviewRenderer::setupTileset() {
    setupTilesetBuffers();
    setupTilesetDescriptorSetLayout();
    setupTilesetDescriptorSet();
    setupTilesetPipeline();
}

void SpriteEditorOverviewRenderer::setupTilesetBuffers() {
    VkDeviceSize tilesetMVPBufferSize = sizeof(tileset.tilesetMVPData);
    gb->createBuffer(
        tilesetMVPBufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        tileset.tilesetMVPBuffer,
        tileset.tilesetMVPMemory);
}

void SpriteEditorOverviewRenderer::setupTilesetDescriptorSetLayout() {
    // Binding
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0),
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            1)
    };

    // Layouts
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = gh::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
    gb->createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &tileset.descriptorSetLayout);
}

void SpriteEditorOverviewRenderer::setupTilesetDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo =
        gh::descriptorSetAllocateInfo(
            descriptorPool,
            &tileset.descriptorSetLayout,
            1);

    gb->allocateDescriptorSets(&allocInfo, &tileset.descriptorSet);

    VkDescriptorBufferInfo tilesetMVPBufferInfo =
        gh::descriptorBufferInfo(
            tileset.tilesetMVPBuffer,
            0,
            sizeof(tileset.tilesetMVPData));
    
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        gh::writeDescriptorSet(
            tileset.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            0,
            &tilesetMVPBufferInfo)
    };

    gb->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void SpriteEditorOverviewRenderer::setupTilesetPipeline() {
    // Shaders
    VkShaderModule vertShaderModule = gb->createShaderModule("res/shaders/tileset_view/tileset/vert.spv");
    VkShaderModule fragShaderModule = gb->createShaderModule("res/shaders/tileset_view/tileset/frag.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = gh::pipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gh::pipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputState = gh::pipelineVertexInputStateCreateInfo();
    vertexInputState.vertexBindingDescriptionCount = 0;
    vertexInputState.vertexAttributeDescriptionCount = 0;

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
            VK_CULL_MODE_FRONT_BIT,
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
            &tileset.descriptorSetLayout,
            1);

    gb->createPipelineLayout(&pipelineLayoutInfo, nullptr, &tileset.pipelineLayout);

    // Actual pipeline creation
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gh::pipelineCreateInfo(tileset.pipelineLayout, renderPass, 0);
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    
    gb->createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &tileset.pipeline);

    // Cleanup
    gb->destroyShaderModule(vertShaderModule, nullptr);
    gb->destroyShaderModule(fragShaderModule, nullptr);
}

void SpriteEditorOverviewRenderer::updateTilesetBuffers() {
    tileset.tilesetMVPData.tilesetMVP = vp * tileset.model;

    void * data;
    // Update MVP matrix
    gb->mapMemory(tileset.tilesetMVPMemory, 0, sizeof(tileset.tilesetMVPData), 0, &data);
        memcpy(data, &tileset.tilesetMVPData, sizeof(tileset.tilesetMVPData));
    gb->unmapMemory(tileset.tilesetMVPMemory);

    // update image
    VkDescriptorImageInfo tilesetImageInfo =
        gh::descriptorImageInfo(
            tileset.sprite->getSampler(),
            tileset.sprite->getImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        gh::writeDescriptorSet(
            tileset.descriptorSet,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            &tilesetImageInfo)
    };

    gb->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void SpriteEditorOverviewRenderer::destroyTileset() {
    gb->destroyPipeline(tileset.pipeline, nullptr);
    gb->destroyPipelineLayout(tileset.pipelineLayout, nullptr);
    gb->destroyBuffer(tileset.tilesetMVPBuffer, nullptr);
    gb->freeMemory(tileset.tilesetMVPMemory, nullptr);
    gb->destroyDescriptorSetLayout(tileset.descriptorSetLayout, nullptr);
}


/*--------------------- Quads ---------------------*/

GraphicsQuads * SpriteEditorOverviewRenderer::getCurrentSelectionQuads() {
    return currentSelection.quads;
}

GraphicsQuads * SpriteEditorOverviewRenderer::getPreviousSelectionQuads() {
    return previousSelection.quads;
}
