#include "sprite_preview.h"

using namespace std::chrono;

using namespace uengine::graphics;
using GraphicsBase = uengine::graphics::GraphicsBase;
namespace gh = uengine::graphics::helper;

SpritePreview::SpritePreview(GraphicsBase * gb_, Sprite * sprite, int32_t w, int32_t h) {
    gb = gb_;
    spriteBox = new SpriteBox(sprite);

    setupDescriptorPool();
    setupRenderPass();
    setupOffscreen(w, h);
    
    setupBuffers();
    setupDescriptorSetLayout();
    setupDescriptorSet();
    setupPipeline();

}

SpritePreview::~SpritePreview() {
    delete spriteBox;
    destroyOffscreen();
    gb->destroyRenderPass(renderPass, nullptr);
    gb->destroyDescriptorPool(descriptorPool, nullptr);
    gb->destroyPipeline(pipeline, nullptr);
    gb->destroyPipelineLayout(pipelineLayout, nullptr);
    gb->destroyBuffer(directVPBuffer, nullptr);
    gb->freeMemory(directVPMemory, nullptr);
    gb->destroyBuffer(spriteBoxBuffer, nullptr);
    gb->freeMemory(spriteBoxMemory, nullptr);
    gb->destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
}

ImTextureID SpritePreview::getTexture() {
    return offscreen.texture;
}

void SpritePreview::update() {
    std::chrono::time_point<std::chrono::high_resolution_clock> time = std::chrono::high_resolution_clock::now();
    float dt = ((std::chrono::duration<float>) (time - lastTime)).count();
    lastTime = time;
    spriteBox->update(dt);
    
    void * data;
    gb->mapMemory(spriteBoxMemory, 0, sizeof(SpriteBoxData), 0, &data);
        memcpy(data, spriteBox->getData(), sizeof(SpriteBoxData));
    gb->unmapMemory(spriteBoxMemory);
}

void SpritePreview::setViewProjection(glm::mat4 vp) {
    directVPData.directVP = vp;
    
    void * data;
    gb->mapMemory(directVPMemory, 0, sizeof(directVPData), 0, &data);
        memcpy(data, &directVPData, sizeof(directVPData));
    gb->unmapMemory(directVPMemory);
}

void SpritePreview::setBackgroundColor(float color_[4]) {
    color[0] = color_[0];
    color[1] = color_[1];
    color[2] = color_[2];
    color[3] = color_[3];
}

SpriteBox * SpritePreview::getSpriteBox() {
    return spriteBox;
}

void SpritePreview::render(VkCommandBuffer cb) {
    VkClearValue clearValues = {0};
    std::copy(color, color + 4, clearValues.color.float32);

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

    // Sprite
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdDraw(cb, 6, 1, 0, 0);

    vkCmdEndRenderPass(cb);
}


/*----------------- Descriptor pool ----------------*/ 

void SpritePreview::setupDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        gh::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16),
        gh::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
        gh::descriptorPoolCreateInfo(
            poolSizes.size(),
            poolSizes.data(),
            3);

    gb->createDescriptorPool(&descriptorPoolInfo, nullptr, &descriptorPool);
}


/*--------------------- Render pass ---------------------*/

void SpritePreview::setupRenderPass() {
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


/*----------------- Offscreen -----------------*/

void SpritePreview::setupOffscreen(int32_t width, int32_t height) {
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

void SpritePreview::destroyOffscreen() {
    gb->destroyFramebuffer(offscreen.frameBuffer, nullptr);
    gb->destroySampler(offscreen.sampler, nullptr);
    gb->destroyImageView(offscreen.view, nullptr);
    gb->destroyImage(offscreen.image, nullptr);
    gb->freeMemory(offscreen.mem, nullptr);
}


/*---------------- Buffers and Pipeline -----------------*/

void SpritePreview::setupBuffers() {
    VkDeviceSize directVPBufferSize = sizeof(directVPData);
    gb->createBuffer(
        directVPBufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        directVPBuffer,
        directVPMemory);
    
    VkDeviceSize spriteBoxBufferSize = sizeof(SpriteBoxData);
    gb->createBuffer(
        spriteBoxBufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        spriteBoxBuffer,
        spriteBoxMemory);
}

void SpritePreview::setupDescriptorSetLayout() {
    // Binding
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0),
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            1),
        gh::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            2)
    };

    // Layouts
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo =
        gh::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
    gb->createDescriptorSetLayout(&descriptorLayoutInfo, nullptr, &descriptorSetLayout);
}

void SpritePreview::setupDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo =
        gh::descriptorSetAllocateInfo(
            descriptorPool,
            &descriptorSetLayout,
            1);

    gb->allocateDescriptorSets(&allocInfo, &descriptorSet);

    VkDescriptorBufferInfo directVPBufferInfo =
        gh::descriptorBufferInfo(
            directVPBuffer,
            0,
            sizeof(directVPData));
    
    VkDescriptorBufferInfo spriteBoxBufferInfo =
        gh::descriptorBufferInfo(
            spriteBoxBuffer,
            0,
            sizeof(SpriteBoxData));
    
    VkDescriptorImageInfo imageInfo =
        gh::descriptorImageInfo(
            spriteBox->getSprite()->getSampler(),
            spriteBox->getSprite()->getImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        gh::writeDescriptorSet(
            descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            0,
            &directVPBufferInfo),
        gh::writeDescriptorSet(
            descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            &spriteBoxBufferInfo),
        gh::writeDescriptorSet(
            descriptorSet,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            2,
            &imageInfo)
    };

    gb->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void SpritePreview::setupPipeline() {
    // Shaders
    VkShaderModule vertShaderModule = gb->createShaderModule("res/shaders/sprite_preview/vert.spv");
    VkShaderModule fragShaderModule = gb->createShaderModule("res/shaders/sprite_preview/frag.spv");

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
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = gh::pipelineCreateInfo(pipelineLayout, renderPass, 0);
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
