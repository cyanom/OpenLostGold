#include "sprite_editor_overview_renderer.h"

using namespace uengine::sprite_editor;
using namespace uengine::graphics;
using GraphicsBase = uengine::graphics::GraphicsBase;
using GraphicsGrid = uengine::graphics::GraphicsGrid;
using GraphicsQuads = uengine::graphics::GraphicsQuads;
namespace gh = uengine::graphics::helper;


SpriteEditorOverviewRenderer::SpriteEditorOverviewRenderer(GraphicsBase * gb_) {
    gb = gb_;
    setupRenderPass();
    grid = new GraphicsGrid(gb, &renderPass);
    currentSelectionQuads = new GraphicsQuads(gb, &renderPass, 1000, 3);
    previousSelectionQuads = new GraphicsQuads(gb, &renderPass, 1000, 3);
}

SpriteEditorOverviewRenderer::~SpriteEditorOverviewRenderer() {
    delete grid;
    delete currentSelectionQuads;
    delete previousSelectionQuads;
    destroyOffscreen();
    gb->destroyRenderPass(renderPass, nullptr);
}

/*
 *  External methods
 */

void SpriteEditorOverviewRenderer::setBackgroundColor(float color[4]) {
    std::copy(color, color + 4, backgroundColor);
}

void SpriteEditorOverviewRenderer::setViewProjection(glm::mat4 vp) {
    grid->setViewProjection(vp);
    currentSelectionQuads->setViewProjection(vp);
    previousSelectionQuads->setViewProjection(vp);
}

GraphicsGrid * SpriteEditorOverviewRenderer::getGrid() {
    return grid;
}

GraphicsQuads * SpriteEditorOverviewRenderer::getCurrentSelectionQuads() {
    return currentSelectionQuads;
}

GraphicsQuads * SpriteEditorOverviewRenderer::getPreviousSelectionQuads() {
    return previousSelectionQuads;
}

void SpriteEditorOverviewRenderer::update() {
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
    
    grid->render(cb);
    currentSelectionQuads->render(cb);
    previousSelectionQuads->render(cb);

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
    float size[2] = {(float) width, (float) height};
    grid->setScreenSize(size);
}

ImTextureID SpriteEditorOverviewRenderer::getTexture() {
    return offscreen.texture;
}

/*
 *  Internal methods
 */

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
