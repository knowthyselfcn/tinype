//#define _CRT_SECURE_NO_WARNINGS

#include "definition.h"
#include "tool.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkan/vulkan.h"

#ifdef _WIN32
//#include <afxwin.h>
#endif

#include <vector>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#define ENABLE_VALIDATION false
#define VERTEX_BUFFER_BIND_ID 0
#define USE_STAGING true

 


// Set to true when example is created with enabled validation layers
bool enableValidation = false;
// Set to true when the debug marker extension is detected
bool enableDebugMarkers = false;
// fps timer (one second interval)
float fpsTimer = 0.0f;
// Create application wide Vulkan instance
//VkResult createInstance(bool enableValidation);
// Create logical Vulkan device based on physical device
//VkResult createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation);
// Get window title with example name, device, et.
//std::string getWindowTitle();
// Destination dimensions for resizing the window
uint32_t destWidth;
uint32_t destHeight;
// Last frame time, measured using a high performance timer (if available)
float frameTimer = 1.0f;
// Frame counter to display fps
uint32_t frameCounter = 0;
uint32_t lastFPS = 0;
// Vulkan instance, stores all per-application states
VkInstance instance;
// Physical device (GPU) that Vulkan will ise
VkPhysicalDevice physicalDevice;
// Stores physical device properties (for e.g. checking device limits)
VkPhysicalDeviceProperties deviceProperties;
// Stores phyiscal device features (for e.g. checking if a feature is available)
VkPhysicalDeviceFeatures deviceFeatures;
// Stores all available memory (type) properties for the physical device
VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
// Logical device, application's view of the physical device (GPU)
VkDevice device;
// Handle to the device graphics queue that command buffers are submitted to
VkQueue queue ;   /////////
// Color buffer format
VkFormat colorformat = VK_FORMAT_B8G8R8A8_UNORM;
// Depth buffer format
// Depth format is selected during Vulkan initialization
VkFormat depthFormat;
// Command buffer pool
VkCommandPool cmdPool;
// Command buffer used for setup
VkCommandBuffer setupCmdBuffer = VK_NULL_HANDLE;
// Command buffer for submitting a post present image barrier
VkCommandBuffer postPresentCmdBuffer = VK_NULL_HANDLE;
// Command buffer for submitting a pre present image barrier
VkCommandBuffer prePresentCmdBuffer = VK_NULL_HANDLE;
// Pipeline stage flags for the submit info structure
VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
// Contains command buffers and semaphores to be presented to the queue
VkSubmitInfo submitInfo;
// Command buffers used for rendering
std::vector<VkCommandBuffer> drawCmdBuffers;
// Global render pass for frame buffer writes
VkRenderPass renderPass;
// List of available frame buffers (same as number of swap chain images)
std::vector<VkFramebuffer>frameBuffers;
// Active frame buffer index
uint32_t currentBuffer = 0;
// Descriptor set pool
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
// List of shader modules created (stored for cleanup)
std::vector<VkShaderModule> shaderModules;
// Pipeline cache object
VkPipelineCache pipelineCache;
// Wraps the swap chain to present images (framebuffers) to the windowing system
#include "vulkanswapchain.hpp"
VulkanSwapChain swapChain;



struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkPipelineVertexInputStateCreateInfo inputState;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
} vertices;

struct {
    int count;
    VkBuffer buf;
    VkDeviceMemory mem;
} indices;

struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo descriptor;
}  uniformDataVS;

struct {
    glm::mat4 projectionMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
} uboVS;

struct {
    VkPipeline solid;
} pipelines;

VkPipelineLayout pipelineLayout;
VkDescriptorSet descriptorSet;
VkDescriptorSetLayout descriptorSetLayout;
// Synchronization semaphores
struct {
    // Swap chain image presentation
    VkSemaphore presentComplete;
    // Command buffer submission and execution
    VkSemaphore renderComplete;
    // Text overlay submission and execution
    VkSemaphore textOverlayComplete;
} semaphores;
#include "vulkanTextureLoader.hpp"
// Simple texture loader
vkTools::VulkanTextureLoader *textureLoader = nullptr;
// Returns the base asset path (for shaders, models, textures) depending on the os
const std::string getAssetPath()
{
#if defined(__ANDROID__)
    return "";
#else
    return "./../data/";
#endif
}
bool prepared = false;
uint32_t width = 1280;
uint32_t height = 720;

union VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } } ;
//= { { 0 } }; //

float zoom = 0;

// Defines a frame rate independent timer value clamped from -1.0...1.0
// For use in animations, rotations, etc.
float timer = 0.0f;
// Multiplier for speeding up (or slowing down) the global timer
float timerSpeed = 0.25f;

bool paused = false;

bool enableTextOverlay = false;
#include "vulkantextoverlay.hpp"
VulkanTextOverlay *textOverlay;

// Use to adjust mouse rotation speed
float rotationSpeed = 1.0f;
// Use to adjust mouse zoom speed
float zoomSpeed = 1.0f;

glm::vec3 rotation = glm::vec3();
glm::vec3 cameraPos = glm::vec3();
glm::vec2 mousePos;

std::string title = "Vulkan Example";
std::string name = "vulkanExample";

struct
{
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
} depthStencil;

// Gamepad state (only one pad supported)

struct
{
    struct
    {
        float x ;
        float y ;
        float z ;
        float rz ;
    } axes;
} gamePadState;

 
HWND window;
HINSTANCE windowInstance;




















void* my_vkAllocationFunction(
    void*                                       pUserData,
    size_t                                      size,
    size_t                                      alignment,
    VkSystemAllocationScope                     allocationScope)
{


    return NULL;
}


VkResult createInstance(bool _enableValidation)
{
    enableValidation = _enableValidation;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.pEngineName = name.c_str();
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

    // Enable surface extensions depending on os
#if defined(_WIN32)
    enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
    enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
    enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (enabledExtensions.size() > 0)
    {
        if (enableValidation)
        {
            enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }
    if (enableValidation)
    {
        instanceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
        instanceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
    }
    return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    
}


VkResult createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation)
{
    std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &requestedQueues;
    deviceCreateInfo.pEnabledFeatures = NULL;

    // enable the debug marker extension if it is present (likely meaning a debugging tool is present)
    if (vkTools::checkDeviceExtensionPresent(physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
    {
        enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        enableDebugMarkers = true;
    }

    if (enabledExtensions.size() > 0)
    {
        deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }
    if (enableValidation)
    {
        deviceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
        deviceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
    }

    return vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
}


std::string getWindowTitle()
{
    std::string device(deviceProperties.deviceName);
    std::string windowTitle;
    windowTitle = title + " - " + device + " - " + std::to_string(frameCounter) + " fps";
    return windowTitle;
}
  
HWND  setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
{
    windowInstance = hinstance;

    bool fullscreen = false;

    // Check command line arguments
    for (int32_t i = 0; i < __argc; i++)
    {
        if (__argv[i] == std::string("-fullscreen"))
        {
            fullscreen = true;
        }
    }

    WNDCLASSEX wndClass;

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = wndproc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hinstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = name.c_str();
    wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

    if (!RegisterClassEx(&wndClass))
    {
        std::cout << "Could not register window class!\n";
        fflush(stdout);
        exit(1);
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (fullscreen)
    {
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = screenWidth;
        dmScreenSettings.dmPelsHeight = screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if ((width != screenWidth) && (height != screenHeight))
        {
            if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            {
                if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                {
                    fullscreen = FALSE;
                }
                else
                {
                    return FALSE;
                }
            }
        }

    }

    DWORD dwExStyle;
    DWORD dwStyle;

    if (fullscreen)
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    }

    RECT windowRect;
    if (fullscreen)
    {
        windowRect.left = (long)0;
        windowRect.right = (long)screenWidth;
        windowRect.top = (long)0;
        windowRect.bottom = (long)screenHeight;
    }
    else
    {
        windowRect.left = (long)screenWidth / 2 - width / 2;
        windowRect.right = (long)width;
        windowRect.top = (long)screenHeight / 2 - height / 2;
        windowRect.bottom = (long)height;
    }

    AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

    std::string windowTitle = getWindowTitle();
    window = CreateWindowEx(0,
        name.c_str(),
        windowTitle.c_str(),
        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        windowRect.left,
        windowRect.top,
        windowRect.right,
        windowRect.bottom,
        NULL,
        NULL,
        hinstance,
        NULL);

    if (!window)
    {
        printf("Could not create window!\n");
        fflush(stdout);
        return 0;
        exit(1);
    }

    ShowWindow(window, SW_SHOW);
    SetForegroundWindow(window);
    SetFocus(window);

    return window;
}


void createSetupCommandBuffer()
{
    if (setupCmdBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(device, cmdPool, 1, &setupCmdBuffer);
        setupCmdBuffer = VK_NULL_HANDLE; // todo : check if still necessary
    }

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vkTools::initializers::commandBufferAllocateInfo(
        cmdPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1);

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &setupCmdBuffer));

    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK_RESULT(vkBeginCommandBuffer(setupCmdBuffer, &cmdBufInfo));
}

uint32_t getMemoryType(uint32_t typeBits, VkFlags properties)
{
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }

    // todo : throw error
    return 0;
}

void setupDepthStencil()
{
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = NULL;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = depthFormat;
    image.extent = { width, height, 1 };
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image.flags = 0;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkImageViewCreateInfo depthStencilView = {};
    depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthStencilView.pNext = NULL;
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = depthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;

    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &depthStencil.image));
    vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);
    mem_alloc.allocationSize = memReqs.size;
    mem_alloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &mem_alloc, nullptr, &depthStencil.mem));

    VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.mem, 0));
    vkTools::setImageLayout(
        setupCmdBuffer,
        depthStencil.image,
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    depthStencilView.image = depthStencil.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &depthStencil.view));
}


void setupFrameBuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    frameBuffers.resize(swapChain.imageCount);
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        attachments[0] = swapChain.buffers[i].view;
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
    }
}


void flushSetupCommandBuffer()
{
    if (setupCmdBuffer == VK_NULL_HANDLE)
        return;

    VK_CHECK_RESULT(vkEndCommandBuffer(setupCmdBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &setupCmdBuffer;
     
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    vkFreeCommandBuffers(device, cmdPool, 1, &setupCmdBuffer);
    setupCmdBuffer = VK_NULL_HANDLE;
}


void destroyCommandBuffers()
{
    vkFreeCommandBuffers(device, cmdPool, (uint32_t)drawCmdBuffers.size(), drawCmdBuffers.data());
    vkFreeCommandBuffers(device, cmdPool, 1, &prePresentCmdBuffer);
    vkFreeCommandBuffers(device, cmdPool, 1, &postPresentCmdBuffer);
}

void createCommandBuffers()
{
    // Create one command buffer per frame buffer
    // in the swap chain
    // Command buffers store a reference to the
    // frame buffer inside their render pass info
    // so for static usage withouth having to rebuild
    // them each frame, we use one per frame buffer

    drawCmdBuffers.resize(swapChain.imageCount);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vkTools::initializers::commandBufferAllocateInfo(
        cmdPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        (uint32_t)drawCmdBuffers.size());

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));

    // Command buffers for submitting present barriers
    cmdBufAllocateInfo.commandBufferCount = 1;
    // Pre present
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &prePresentCmdBuffer));
    // Post present
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &postPresentCmdBuffer));
}

void buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = NULL;

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = NULL;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;


    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        // Start the first sub pass specified in our default render pass setup by the base class
        // This will clear the color and depth attachment
        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Update dynamic viewport state
        VkViewport viewport = {};
        viewport.height = (float)height;
        viewport.width = (float)width;
        viewport.minDepth = (float) 0.0f;
        viewport.maxDepth = (float) 1.0f;
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        // Update dynamic scissor state
        VkRect2D scissor = {};
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

        // Bind descriptor sets describing shader binding points
        vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

        // Bind the rendering pipeline (including the shaders)
        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);

        // Bind triangle vertices
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertices.buf, offsets);

        // Bind triangle indices
        vkCmdBindIndexBuffer(drawCmdBuffers[i], indices.buf, 0, VK_INDEX_TYPE_UINT32);

        // Draw indexed triangle
        vkCmdDrawIndexed(drawCmdBuffers[i], indices.count, 1, 0, 0, 1);

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        // Add a present memory barrier to the end of the command buffer
        // This will transform the frame buffer color attachment to a
        // new layout for presenting it to the windowing system integration 
        VkImageMemoryBarrier prePresentBarrier = {};
        prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prePresentBarrier.pNext = NULL;
        prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        prePresentBarrier.image = swapChain.buffers[i].image;

        VkImageMemoryBarrier *pMemoryBarrier = &prePresentBarrier;
        vkCmdPipelineBarrier(
            drawCmdBuffers[i],
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_FLAGS_NONE,
            0, nullptr,
            0, nullptr,
            1, &prePresentBarrier);

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
}

void getOverlayText(VulkanTextOverlay *textOverlay)
{
    // Can be overriden in derived class
}
void updateTextOverlay()
{
    if (!enableTextOverlay)
        return;

    textOverlay->beginTextUpdate();

    textOverlay->addText(title, 5.0f, 5.0f, VulkanTextOverlay::alignLeft);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << (frameTimer * 1000.0f) << "ms (" << lastFPS << " fps)";
    textOverlay->addText(ss.str(), 5.0f, 25.0f, VulkanTextOverlay::alignLeft);

    textOverlay->addText(deviceProperties.deviceName, 5.0f, 45.0f, VulkanTextOverlay::alignLeft);

    getOverlayText(textOverlay);

    textOverlay->endTextUpdate();
}

void windowResized()
{
    // Can be overriden in derived class
}

void updateUniformBuffers()
{
    // Update matrices
    uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);

    uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));

    uboVS.modelMatrix = glm::mat4();
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Map uniform buffer and update it
    // If you want to keep a handle to the memory and not unmap it afer updating, 
    // create the memory with the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 
    uint8_t *pData;
    VK_CHECK_RESULT(vkMapMemory(device, uniformDataVS.memory, 0, sizeof(uboVS), 0, (void **)&pData));
    memcpy(pData, &uboVS, sizeof(uboVS));
    vkUnmapMemory(device, uniformDataVS.memory);
}

void viewChanged()
{
    // Before updating the uniform buffer we want to make
    // sure that the device has finished all operations
    // In a real-world application you would use synchronization
    // objects for this
    vkDeviceWaitIdle(device);
    // This function is called by the base example class 
    // each time the view is changed by user input
    updateUniformBuffers();
}

void windowResize()
{
    if (!prepared)
    {
        return;
    }
    prepared = false;

    // Recreate swap chain
    width = destWidth;
    height = destHeight;
    createSetupCommandBuffer();
    swapChain.create(setupCmdBuffer, &width, &height);
    //setupSwapChain();

    // Recreate the frame buffers

    vkDestroyImageView(device, depthStencil.view, nullptr);
    vkDestroyImage(device, depthStencil.image, nullptr);
    vkFreeMemory(device, depthStencil.mem, nullptr);
    setupDepthStencil();

    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
    }
    setupFrameBuffer();

    flushSetupCommandBuffer();

    // Command buffers need to be recreated as they may store
    // references to the recreated frame buffer
    destroyCommandBuffers();
    createCommandBuffers();
    buildCommandBuffers();

    vkQueueWaitIdle(queue);
    vkDeviceWaitIdle(device);

    if (enableTextOverlay)
    {
        textOverlay->reallocateCommandBuffers();
        updateTextOverlay();
    }

    // Notify derived class
    windowResized();
    viewChanged();

    prepared = true;
}

void keyPressed(uint32_t keyCode)
{
    // Can be overriden in derived class
}

void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        prepared = false;
        DestroyWindow(hWnd);
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        ValidateRect(window, NULL);
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 0x50:
            paused = !paused;
            break;
        case VK_F1:
            if (enableTextOverlay)
            {
                textOverlay->visible = !textOverlay->visible;
            }
            break;
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        keyPressed((uint32_t)wParam);
        break;
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
        mousePos.x = (float)LOWORD(lParam);
        mousePos.y = (float)HIWORD(lParam);
        break;
    case WM_MOUSEWHEEL:
    {
        short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        zoom += (float)wheelDelta * 0.005f * zoomSpeed;
        viewChanged();
        break;
    }
    case WM_MOUSEMOVE:
        if (wParam & MK_RBUTTON)
        {
            int32_t posx = LOWORD(lParam);
            int32_t posy = HIWORD(lParam);
            zoom += (mousePos.y - (float)posy) * .005f * zoomSpeed;
            mousePos = glm::vec2((float)posx, (float)posy);
            viewChanged();
        }
        if (wParam & MK_LBUTTON)
        {
            int32_t posx = LOWORD(lParam);
            int32_t posy = HIWORD(lParam);
            rotation.x += (mousePos.y - (float)posy) * 1.25f * rotationSpeed;
            rotation.y -= (mousePos.x - (float)posx) * 1.25f * rotationSpeed;
            mousePos = glm::vec2((float)posx, (float)posy);
            viewChanged();
        }
        if (wParam & MK_MBUTTON)
        {
            int32_t posx = LOWORD(lParam);
            int32_t posy = HIWORD(lParam);
            cameraPos.x -= (mousePos.x - (float)posx) * 0.01f;
            cameraPos.y -= (mousePos.y - (float)posy) * 0.01f;
            viewChanged();
            mousePos.x = (float)posx;
            mousePos.y = (float)posy;
        }
        break;
    case WM_SIZE:
        if ((prepared) && (wParam != SIZE_MINIMIZED))
        {
            destWidth = LOWORD(lParam);
            destHeight = HIWORD(lParam);
            if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_MINIMIZED))
            {
                windowResize();
            }
        }
        break;
    case WM_EXITSIZEMOVE:
        if ((prepared) && ((destWidth != width) || (destHeight != height)))
        {
            windowResize();
        }
        break;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    handleMessages(hWnd, uMsg, wParam, lParam); 
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void createCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
}


void setupRenderPass()
{
    VkAttachmentDescription attachments[2] = {};

    // Color attachment
    attachments[0].format = colorformat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthReference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = NULL;

    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}


void createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}


VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
#if defined(__ANDROID__)
    shaderStage.module = vkTools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device, stage);
#else
    shaderStage.module = vkTools::loadShader(fileName.c_str(), device, stage);
#endif
    shaderStage.pName = "main"; // todo : make param
    assert(shaderStage.module != NULL);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

void basePrepare()
{
    if (enableValidation)
    {
        vkDebug::setupDebugging(instance, VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_NULL_HANDLE);
    }
    if (enableDebugMarkers)
    {
        vkDebug::DebugMarker::setup(device);
    }
    createCommandPool();
    createSetupCommandBuffer();
    //setupSwapChain();
    swapChain.create(setupCmdBuffer, &width, &height);
    createCommandBuffers();
    setupDepthStencil();
    setupRenderPass();
    createPipelineCache();
    setupFrameBuffer();
    flushSetupCommandBuffer();
    // Recreate setup command buffer for derived class
    createSetupCommandBuffer();
    // Create a simple texture loader class
    textureLoader = new vkTools::VulkanTextureLoader(physicalDevice, device, queue, cmdPool);
#if defined(__ANDROID__)
    textureLoader->assetManager = androidApp->activity->assetManager;
#endif
    if (enableTextOverlay)
    {
        // Load the text rendering shaders
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.push_back(loadShader(getAssetPath() + "shaders/base/textoverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
        shaderStages.push_back(loadShader(getAssetPath() + "shaders/base/textoverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
        textOverlay = new VulkanTextOverlay(
            physicalDevice,
            device,
            queue,
            frameBuffers,
            colorformat,
            depthFormat,
            &width,
            &height,
            shaderStages
            );
        updateTextOverlay();
    }
}

// Create synchronzation semaphores
void prepareSemaphore()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = NULL;

    // This semaphore ensures that the image is complete
    // before starting to submit again
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));

    // This semaphore ensures that all commands submitted
    // have been finished before submitting the image to the queue
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));
}


// Setups vertex and index buffers for an indexed triangle,
// uploads them to the VRAM and sets binding points and attribute
// descriptions to match locations inside the shaders
void prepareVertices(bool useStagingBuffers)
{
    struct Vertex {
        float pos[3];
        float col[3];
    };

    // Setup vertices
    std::vector<Vertex> vertexBuffer = {
        { { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };
    int vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);

    // Setup indices
    std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
    uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    indices.count = indexBuffer.size();

    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    void *data;

    if (useStagingBuffers)
    {
        // Static data like vertex and index buffer should be stored on the device memory 
        // for optimal (and fastest) access by the GPU
        //
        // To achieve this we use so-called "staging buffers" :
        // - Create a buffer that's visible to the host (and can be mapped)
        // - Copy the data to this buffer
        // - Create another buffer that's local on the device (VRAM) with the same size
        // - Copy the data from the host to the device using a command buffer
        // - Delete the host visible (staging) buffer
        // - Use the device local buffers for rendering

        struct StagingBuffer {
            VkDeviceMemory memory;
            VkBuffer buffer;
        };

        struct {
            StagingBuffer vertices;
            StagingBuffer indices;
        } stagingBuffers;

        // Buffer copies are done on the queue, so we need a command buffer for them
        VkCommandBufferAllocateInfo cmdBufInfo = {};
        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufInfo.commandPool = cmdPool;
        cmdBufInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufInfo.commandBufferCount = 1;

        VkCommandBuffer copyCommandBuffer;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufInfo, &copyCommandBuffer));

        // Vertex buffer
        VkBufferCreateInfo vertexBufferInfo = {};
        vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferInfo.size = vertexBufferSize;
        // Buffer is used as the copy source
        vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        // Create a host-visible buffer to copy the vertex data to (staging buffer)
        VK_CHECK_RESULT(vkCreateBuffer(device, &vertexBufferInfo, nullptr, &stagingBuffers.vertices.buffer));
        vkGetBufferMemoryRequirements(device, stagingBuffers.vertices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &stagingBuffers.vertices.memory));
        // Map and copy
        VK_CHECK_RESULT(vkMapMemory(device, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data));
        memcpy(data, vertexBuffer.data(), vertexBufferSize);
        vkUnmapMemory(device, stagingBuffers.vertices.memory);
        VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0));

        // Create the destination buffer with device only visibility
        // Buffer will be used as a vertex buffer and is the copy destination
        vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VK_CHECK_RESULT(vkCreateBuffer(device, &vertexBufferInfo, nullptr, &vertices.buf));
        vkGetBufferMemoryRequirements(device, vertices.buf, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &vertices.mem));
        VK_CHECK_RESULT(vkBindBufferMemory(device, vertices.buf, vertices.mem, 0));

        // Index buffer
        VkBufferCreateInfo indexbufferInfo = {};
        indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexbufferInfo.size = indexBufferSize;
        indexbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        // Copy index data to a buffer visible to the host (staging buffer)
        VK_CHECK_RESULT(vkCreateBuffer(device, &indexbufferInfo, nullptr, &stagingBuffers.indices.buffer));
        vkGetBufferMemoryRequirements(device, stagingBuffers.indices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &stagingBuffers.indices.memory));
        VK_CHECK_RESULT(vkMapMemory(device, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data));
        memcpy(data, indexBuffer.data(), indexBufferSize);
        vkUnmapMemory(device, stagingBuffers.indices.memory);
        VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));

        // Create destination buffer with device only visibility
        indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VK_CHECK_RESULT(vkCreateBuffer(device, &indexbufferInfo, nullptr, &indices.buf));
        vkGetBufferMemoryRequirements(device, indices.buf, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &indices.mem));
        VK_CHECK_RESULT(vkBindBufferMemory(device, indices.buf, indices.mem, 0));
        indices.count = indexBuffer.size();

        VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.pNext = NULL;

        VkBufferCopy copyRegion = {};

        // Put buffer region copies into command buffer
        // Note that the staging buffer must not be deleted before the copies 
        // have been submitted and executed
        VK_CHECK_RESULT(vkBeginCommandBuffer(copyCommandBuffer, &cmdBufferBeginInfo));

        // Vertex buffer
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(
            copyCommandBuffer,
            stagingBuffers.vertices.buffer,
            vertices.buf,
            1,
            &copyRegion);
        // Index buffer
        copyRegion.size = indexBufferSize;
        vkCmdCopyBuffer(
            copyCommandBuffer,
            stagingBuffers.indices.buffer,
            indices.buf,
            1,
            &copyRegion);

        VK_CHECK_RESULT(vkEndCommandBuffer(copyCommandBuffer));

        // Submit copies to the queue
        VkSubmitInfo copySubmitInfo = {};
        copySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        copySubmitInfo.commandBufferCount = 1;
        copySubmitInfo.pCommandBuffers = &copyCommandBuffer;

        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &copySubmitInfo, VK_NULL_HANDLE));
        VK_CHECK_RESULT(vkQueueWaitIdle(queue));

        vkFreeCommandBuffers(device, cmdPool, 1, &copyCommandBuffer);

        // Destroy staging buffers
        vkDestroyBuffer(device, stagingBuffers.vertices.buffer, nullptr);
        vkFreeMemory(device, stagingBuffers.vertices.memory, nullptr);
        vkDestroyBuffer(device, stagingBuffers.indices.buffer, nullptr);
        vkFreeMemory(device, stagingBuffers.indices.memory, nullptr);
    }
    else
    {
        // Don't use staging
        // Create host-visible buffers only and use these for rendering
        // This is not advised for real world applications and will
        // result in lower performances at least on devices that
        // separate between host visible and device local memory

        // Vertex buffer
        VkBufferCreateInfo vertexBufferInfo = {};
        vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferInfo.size = vertexBufferSize;
        vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        // Copy vertex data to a buffer visible to the host
        VK_CHECK_RESULT(vkCreateBuffer(device, &vertexBufferInfo, nullptr, &vertices.buf));
        vkGetBufferMemoryRequirements(device, vertices.buf, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &vertices.mem));
        VK_CHECK_RESULT(vkMapMemory(device, vertices.mem, 0, memAlloc.allocationSize, 0, &data));
        memcpy(data, vertexBuffer.data(), vertexBufferSize);
        vkUnmapMemory(device, vertices.mem);
        VK_CHECK_RESULT(vkBindBufferMemory(device, vertices.buf, vertices.mem, 0));

        // Index buffer
        VkBufferCreateInfo indexbufferInfo = {};
        indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexbufferInfo.size = indexBufferSize;
        indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        // Copy index data to a buffer visible to the host
        memset(&indices, 0, sizeof(indices));
        VK_CHECK_RESULT(vkCreateBuffer(device, &indexbufferInfo, nullptr, &indices.buf));
        vkGetBufferMemoryRequirements(device, indices.buf, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &indices.mem));
        VK_CHECK_RESULT(vkMapMemory(device, indices.mem, 0, indexBufferSize, 0, &data));
        memcpy(data, indexBuffer.data(), indexBufferSize);
        vkUnmapMemory(device, indices.mem);
        VK_CHECK_RESULT(vkBindBufferMemory(device, indices.buf, indices.mem, 0));
        indices.count = indexBuffer.size();
    }

    // Binding description
    vertices.bindingDescriptions.resize(1);
    vertices.bindingDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    vertices.bindingDescriptions[0].stride = sizeof(Vertex);
    vertices.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Attribute descriptions
    // Describes memory layout and shader attribute locations
    vertices.attributeDescriptions.resize(2);
    // Location 0 : Position
    vertices.attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
    vertices.attributeDescriptions[0].location = 0;
    vertices.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertices.attributeDescriptions[0].offset = 0;
    // Location 1 : Color
    vertices.attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
    vertices.attributeDescriptions[1].location = 1;
    vertices.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertices.attributeDescriptions[1].offset = sizeof(float) * 3;

    // Assign to vertex input state
    vertices.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertices.inputState.pNext = NULL;
    vertices.inputState.flags = VK_FLAGS_NONE;
    vertices.inputState.vertexBindingDescriptionCount = vertices.bindingDescriptions.size();
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void prepareUniformBuffers()
{
    // Prepare and initialize a uniform buffer block containing shader uniforms
    // In Vulkan there are no more single uniforms like in GL
    // All shader uniforms are passed as uniform buffer blocks 
    VkMemoryRequirements memReqs;

    // Vertex shader uniform buffer block
    VkBufferCreateInfo bufferInfo = {};
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = 0;
    allocInfo.memoryTypeIndex = 0;

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uboVS);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // Create a new buffer
    VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &uniformDataVS.buffer));
    // Get memory requirements including size, alignment and memory type 
    vkGetBufferMemoryRequirements(device, uniformDataVS.buffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    // Get the memory type index that supports host visibile memory access
    // Most implementations offer multiple memory tpyes and selecting the 
    // correct one to allocate memory from is important
    allocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    // Allocate memory for the uniform buffer
    VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &(uniformDataVS.memory)));
    // Bind memory to buffer
    VK_CHECK_RESULT(vkBindBufferMemory(device, uniformDataVS.buffer, uniformDataVS.memory, 0));

    // Store information in the uniform's descriptor
    uniformDataVS.descriptor.buffer = uniformDataVS.buffer;
    uniformDataVS.descriptor.offset = 0;
    uniformDataVS.descriptor.range = sizeof(uboVS);

    updateUniformBuffers();
}


void setupDescriptorSetLayout()
{
    // Setup layout of descriptors used in this example
    // Basically connects the different shader stages to descriptors
    // for binding uniform buffers, image samplers, etc.
    // So every shader binding should map to one descriptor set layout
    // binding

    // Binding 0 : Uniform buffer (Vertex shader)
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = NULL;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, NULL, &descriptorSetLayout));

    // Create the pipeline layout that is used to generate the rendering pipelines that
    // are based on this descriptor set layout
    // In a more complex scenario you would have different pipeline layouts for different
    // descriptor set layouts that could be reused
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void preparePipelines()
{
    // Create our rendering pipeline used in this example
    // Vulkan uses the concept of rendering pipelines to encapsulate
    // fixed states
    // This replaces OpenGL's huge (and cumbersome) state machine
    // A pipeline is then stored and hashed on the GPU making
    // pipeline changes much faster than having to set dozens of 
    // states
    // In a real world application you'd have dozens of pipelines
    // for every shader set used in a scene
    // Note that there are a few states that are not stored with
    // the pipeline. These are called dynamic states and the 
    // pipeline only stores that they are used with this pipeline,
    // but not their states

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // The layout used for this pipeline
    pipelineCreateInfo.layout = pipelineLayout;
    // Renderpass this pipeline is attached to
    pipelineCreateInfo.renderPass = renderPass;

    // Vertex input state
    // Describes the topoloy used with this pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // This pipeline renders vertex data as triangle lists
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // Solid polygon mode
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    // No culling
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.lineWidth = 1.0f;

    // Color blend state
    // Describes blend modes and color masks
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // One blend attachment state
    // Blending is not used in this example
    VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
    blendAttachmentState[0].colorWriteMask = 0xf;
    blendAttachmentState[0].blendEnable = VK_FALSE;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = blendAttachmentState;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    // One viewport
    viewportState.viewportCount = 1;
    // One scissor rectangle
    viewportState.scissorCount = 1;

    // Enable dynamic states
    // Describes the dynamic states to be used with this pipeline
    // Dynamic states can be set even after the pipeline has been created
    // So there is no need to create new pipelines just for changing
    // a viewport's dimensions or a scissor box
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    // The dynamic state properties themselves are stored in the command buffer
    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = dynamicStateEnables.size();

    // Depth and stencil state
    // Describes depth and stenctil test and compare ops
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    // Basic depth compare setup with depth writes and depth test enabled
    // No stencil used 
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = depthStencilState.back;

    // Multi sampling state
    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.pSampleMask = NULL;
    // No multi sampling used in this example
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Load shaders
    // Shaders are loaded from the SPIR-V format, which can be generated from glsl
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = loadShader(getAssetPath() + "shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(getAssetPath() + "shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    // Assign states
    // Assign pipeline state create information
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &vertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Create rendering pipeline
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.solid));
}


void setupDescriptorPool()
{
    // We need to tell the API the number of max. requested descriptors per type
    VkDescriptorPoolSize typeCounts[1];
    // This example only uses one descriptor type (uniform buffer) and only
    // requests one descriptor of this type
    typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCounts[0].descriptorCount = 1;
    // For additional types you need to add new entries in the type count list
    // E.g. for two combined image samplers :
    // typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // typeCounts[1].descriptorCount = 2;

    // Create the global descriptor pool
    // All descriptors used in this example are allocated from this pool
    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = NULL;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;
    // Set the max. number of sets that can be requested
    // Requesting descriptors beyond maxSets will result in an error
    descriptorPoolInfo.maxSets = 1;

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}


void setupDescriptorSet()
{
    // Allocate a new descriptor set from the global descriptor pool
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

    // Update the descriptor set determining the shader binding points
    // For every binding point used in a shader there needs to be one
    // descriptor set matching that binding point

    VkWriteDescriptorSet writeDescriptorSet = {};

    // Binding 0 : Uniform buffer
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformDataVS.descriptor;
    // Binds this uniform buffer to binding point 0
    writeDescriptorSet.dstBinding = 0;

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
}

void prepare()
{
    basePrepare();
    prepareSemaphore();
    prepareVertices(USE_STAGING);
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSet();
    buildCommandBuffers();
    prepared = true;
}
void draw()
{
    // Get next image in the swap chain (back/front buffer)
    VK_CHECK_RESULT(swapChain.acquireNextImage(semaphores.presentComplete, &currentBuffer));

    // Add a post present image memory barrier
    // This will transform the frame buffer color attachment back
    // to it's initial layout after it has been presented to the
    // windowing system
    VkImageMemoryBarrier postPresentBarrier = {};
    postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    postPresentBarrier.pNext = NULL;
    postPresentBarrier.srcAccessMask = 0;
    postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    postPresentBarrier.image = swapChain.buffers[currentBuffer].image;

    // Use dedicated command buffer from example base class for submitting the post present barrier
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK_RESULT(vkBeginCommandBuffer(postPresentCmdBuffer, &cmdBufInfo));

    // Put post present barrier into command buffer
    vkCmdPipelineBarrier(
        postPresentCmdBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        1, &postPresentBarrier);

    VK_CHECK_RESULT(vkEndCommandBuffer(postPresentCmdBuffer));

    // Submit the image barrier to the current queue
    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &postPresentCmdBuffer;

    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    // Make sure that the image barrier command submitted to the queue 
    // has finished executing
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    // The submit infor strcuture contains a list of
    // command buffers and semaphores to be submitted to a queue
    // If you want to submit multiple command buffers, pass an array
    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &pipelineStages;
    // The wait semaphore ensures that the image is presented 
    // before we start submitting command buffers agein
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    // Submit the currently active command buffer
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
    // The signal semaphore is used during queue presentation
    // to ensure that the image is not rendered before all
    // commands have been submitted
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;

    // Submit to the graphics queue
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    // Present the current buffer to the swap chain
    // We pass the signal semaphore from the submit info
    // to ensure that the image is not rendered until
    // all commands have been submitted
    VK_CHECK_RESULT(swapChain.queuePresent(queue, currentBuffer, semaphores.renderComplete));
}


void render()
{
    if (!prepared)
        return;
    draw();
    
    vkDeviceWaitIdle(device);
}

#include <chrono>
void renderLoop()
{
    destWidth = width;
    destHeight = height;

    MSG msg;
    while (TRUE)
    {
        auto tStart = std::chrono::high_resolution_clock::now();
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        render();
        frameCounter++;
        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        frameTimer = (float)tDiff / 1000.0f;
        // Convert to clamped timer value
        if (!paused)
        {
            timer += timerSpeed * frameTimer;
            if (timer > 1.0)
            {
                timer -= 1.0f;
            }
        }
        fpsTimer += (float)tDiff;
        if (fpsTimer > 1000.0f)
        {
            std::string windowTitle = getWindowTitle();
            if (!enableTextOverlay)
            {
                SetWindowText(window, windowTitle.c_str());
            }
            lastFPS = frameCounter;
            updateTextOverlay();
            fpsTimer = 0.0f;
            frameCounter = 0;
        }
    }

}


void setupConsole(std::string title)
{
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    FILE *stream;
    freopen_s(&stream, "CONOUT$", "w+", stdout);
    SetConsoleTitle(TEXT(title.c_str()));
    if (enableValidation)
    {
        std::cout << "Validation enabled:\n";
    }
}


void initVulkan(bool enableValidation)
{
    VkResult err;

    // Vulkan instance
    err = createInstance(enableValidation);
    if (err)
    {
        vkTools::exitFatal("Could not create Vulkan instance : \n" + vkTools::errorString(err), "Fatal error");
    }

#if defined(__ANDROID__)
    loadVulkanFunctions(instance);
#endif

    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
    assert(gpuCount > 0);
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
    if (err)
    {
        vkTools::exitFatal("Could not enumerate phyiscal devices : \n" + vkTools::errorString(err), "Fatal error");
    }

    // Note :
    // This example will always use the first physical device reported,
    // change the vector index if you have multiple Vulkan devices installed
    // and want to use another one
    physicalDevice = physicalDevices[0];

    // Find a queue that supports graphics operations
    uint32_t graphicsQueueIndex = 0;
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
    assert(queueCount >= 1);

    std::vector<VkQueueFamilyProperties> queueProps;
    queueProps.resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

    for (graphicsQueueIndex = 0; graphicsQueueIndex < queueCount; graphicsQueueIndex++)
    {
        if (queueProps[graphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            break;
    }
    assert(graphicsQueueIndex < queueCount);

    // Vulkan device
    std::array<float, 1> queuePriorities = { 0.0f };
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities.data();

    VK_CHECK_RESULT(createDevice(queueCreateInfo, enableValidation));

    // Store properties (including limits) and features of the phyiscal device
    // So examples can check against them and see if a feature is actually supported
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    // Gather physical device memory properties
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    // Get the graphics queue
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    // Find a suitable depth format
    VkBool32 validDepthFormat = vkTools::getSupportedDepthFormat(physicalDevice, &depthFormat);
    assert(validDepthFormat);

    swapChain.connect(instance, physicalDevice, device);

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkTools::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands for the text overlay have been sumbitted and executed
    // Will be inserted after the render complete semaphore if the text overlay is enabled
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.textOverlayComplete));

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    submitInfo = vkTools::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;
}

void VulkanExampleBase(bool enableValidation)
{
    // Check for validation command line flag
#if defined(_WIN32)
    for (int32_t i = 0; i < __argc; i++)
    {
        if (__argv[i] == std::string("-validation"))
        {
            enableValidation = true;
        }
    }
#elif defined(__ANDROID__)
    // Vulkan library is loaded dynamically on Android
    bool libLoaded = loadVulkanLibrary();
    assert(libLoaded);
#elif defined(__linux__)
    initxcbConnection();
#endif

#if !defined(__ANDROID__)
    // Android Vulkan initialization is handled in APP_CMD_INIT_WINDOW event
    initVulkan(enableValidation);
#endif

#if defined(_WIN32)
    // Enable console if validation is active
    // Debug message callback will output to it
    setupConsole("VulkanExample");
    if (enableValidation)
    {
        setupConsole("VulkanExample");
    }
#endif
}

//int main(void)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    //VulkanSwapChain swapChain111;
    VulkanExampleBase(true);
    setupWindow(hInstance, WndProc);

    swapChain.initSurface(windowInstance, window);
    prepare();
    renderLoop();

 
    return 0;
}