#ifndef GRAPHICS_BASE_H
#define GRAPHICS_BASE_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>
#include <set>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "drawable.h"

namespace uengine::graphics {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    class GraphicsBase {
        public:

            GraphicsBase(int width, int height);

            ~GraphicsBase();

            bool shouldClose();
            void poolEvents();
            void draw();
            void addDrawable(uengine::graphics::Drawable * drawable);
            VkDevice * getDevice();
            
            // Tools
            void createTextureImage(std::string filename, VkImage * textureImage, VkDeviceMemory * textureImageMemory, VkImageView * textureImageView, VkSampler * textureSampler, uint8_t ** data, int * w, int * h, bool keepData);
            void deleteTextureImage(VkImage * textureImage, VkDeviceMemory * textureImageMemory, VkImageView * textureImageView, VkSampler * textureSampler, uint8_t * data);
            
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            
            void createRenderPass(const VkRenderPassCreateInfo * info, const VkAllocationCallbacks * callback, VkRenderPass * renderPass);
            void destroyRenderPass(VkRenderPass renderPass, const VkAllocationCallbacks * callback);
            void createDescriptorPool(const VkDescriptorPoolCreateInfo * info, const VkAllocationCallbacks * callback, VkDescriptorPool * pool);
            void destroyDescriptorPool(VkDescriptorPool pool, const VkAllocationCallbacks * callback);
            void createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo * info, const VkAllocationCallbacks * callback, VkDescriptorSetLayout * setLayout);
            void destroyDescriptorSetLayout(VkDescriptorSetLayout layout, const VkAllocationCallbacks * callback);
            void allocateDescriptorSets(const VkDescriptorSetAllocateInfo * info, VkDescriptorSet * set);
            void updateDescriptorSets(uint32_t writeCount, const VkWriteDescriptorSet * writeSet, uint32_t copyCount, const VkCopyDescriptorSet * copySet);
            void createPipelineLayout(const VkPipelineLayoutCreateInfo * info, const VkAllocationCallbacks * callback, VkPipelineLayout * layout);
            void destroyPipelineLayout(VkPipelineLayout layout, const VkAllocationCallbacks * callback);
            void createGraphicsPipelines(VkPipelineCache cache, uint32_t count, const VkGraphicsPipelineCreateInfo * info, const VkAllocationCallbacks * callback, VkPipeline * pipeline);
            void destroyPipeline(VkPipeline pipeline, const VkAllocationCallbacks * callback);

            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            void destroyBuffer(VkBuffer buffer, const VkAllocationCallbacks * callback);

            void mapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void ** data);
            void unmapMemory(VkDeviceMemory memory);

            std::vector<char> readFile(const std::string& filename);
            VkShaderModule createShaderModule(std::string shader);
            void destroyShaderModule(VkShaderModule module, const VkAllocationCallbacks * callback);

            void getImageMemoryRequirements(VkImage image, VkMemoryRequirements * requirements);
            void allocateMemory(const VkMemoryAllocateInfo * info,  const VkAllocationCallbacks * callback, VkDeviceMemory * memory);
            void freeMemory(VkDeviceMemory memory, const VkAllocationCallbacks * callback);
            void bindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
            void createImage(const VkImageCreateInfo * info, const VkAllocationCallbacks * callback, VkImage * image);
            void destroyImage(VkImage image, const VkAllocationCallbacks * callback);
            void createImageView(const VkImageViewCreateInfo * info, const VkAllocationCallbacks * callback, VkImageView * imageView);
            void destroyImageView(VkImageView imageView, const VkAllocationCallbacks * callback);
            void createSampler(const VkSamplerCreateInfo * info, const VkAllocationCallbacks * callback, VkSampler * sampler);
            void destroySampler(VkSampler sampler, const VkAllocationCallbacks * callback);
            void createFramebuffer(const VkFramebufferCreateInfo * info, const VkAllocationCallbacks * callback, VkFramebuffer * framebuffer);
            void destroyFramebuffer(VkFramebuffer framebuffer, const VkAllocationCallbacks * callback);

        private:

            /* Top level variables */
            
            std::vector<uengine::graphics::Drawable *> drawables;


            /* Low level variables */
            
            GLFWwindow* window;

            VkInstance instance;

            VkSurfaceKHR surface;

            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;

            VkQueue graphicsQueue;
            VkQueue presentQueue;
            uint32_t graphicsFamily;
            uint32_t presentFamily;

            VkSwapchainKHR swapChain;
            std::vector<VkImage> swapChainImages;
            VkFormat swapChainImageFormat;
            VkExtent2D swapChainExtent;

            std::vector<VkImageView> swapChainImageViews;
            std::vector<VkFramebuffer> swapChainFramebuffers;

            VkCommandPool commandPool;
            std::vector<VkCommandBuffer> commandBuffers;
            VkRenderPass renderPass;

            VkDescriptorPool descriptorPool;

            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            std::vector<VkFence> imagesInFlight;

            size_t currentFrame = 0;
            uint32_t imageIndex = 0;

            bool framebufferResized = false;

            VkDebugUtilsMessengerEXT debugMessenger;


            /* Methods */

            void initWindow_(int width, int height);
            static void framebufferResizeCallback_(GLFWwindow* window, int width, int height);
            void initVulkan_();
            void initImgui_();

            bool acquire_();
            void render_();
            void present_();


            // Initialization
            void createImage_(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
            void createImageView_(VkImage * image, VkFormat format, VkImageView * imageView);
            void createTextureSampler_(VkSampler * textureSampler);
            void transitionImageLayout_(VkImage * image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
            void copyBufferToImage_(VkBuffer * buffer, VkImage * image, uint32_t width, uint32_t height);
            VkCommandBuffer beginSingleTimeCommands_();
            void endSingleTimeCommands_(VkCommandBuffer commandBuffer);

            void createSyncObjects_();
            void createCommandPool_();
            void createCommandBuffers_();
            void createRenderPass_();
            void createImageViews_();
            void createFramebuffers_();
            void createDescriptorPool_();
            void createInstance_();
            std::vector<const char*> getRequiredExtensions_();
            void createSurface_();
            void pickPhysicalDevice_();
            bool isDeviceSuitable_(VkPhysicalDevice device);
            QueueFamilyIndices findQueueFamilies_(VkPhysicalDevice device);
            bool checkDeviceExtensionSupport_(VkPhysicalDevice device);
            void createLogicalDevice_();
            void createSwapChain_();
            SwapChainSupportDetails querySwapChainSupport_(VkPhysicalDevice device);
            VkSurfaceFormatKHR chooseSwapSurfaceFormat_(const std::vector<VkSurfaceFormatKHR>& availableFormats);
            VkPresentModeKHR chooseSwapPresentMode_(const std::vector<VkPresentModeKHR>& availablePresentModes);
            VkExtent2D chooseSwapExtent_(const VkSurfaceCapabilitiesKHR& capabilities);
            void setupDebugMessenger_();
            void populateDebugMessengerCreateInfo_(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
            bool checkValidationLayerSupport_();
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback_(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
            void recreateSwapChain_();
            void cleanupSwapChain_();
            void cleanup_();
    };

}

#endif
