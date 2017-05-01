#ifndef VKC_PIPELINE_H
#define VKC_PIPELINE_H

#include "stable.h"
#include "vkc_device.h"
#include "vkc_swapchain.h"


/**
 * Struct used to define vertex shader input layout and buffer size.
 */
struct VkVertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
};


/**
 * Class used for the graphics pipeline.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcPipeline
{
    // Objects:
public:
    VkPipeline                      handle;
    VkPipelineLayout                layout;

    VkShaderModule                  vertShader;
    VkShaderModule                  fragShader;

    VkDescriptorPool                descriptorPool;
    VkDescriptorSet                 descriptorSet;
    VkDescriptorSetLayout           setLayout;

private:
    VkDevice                        logicalDevice;

    // Functions:
public:
    VkcPipeline();
    VkcPipeline(
            const VkcSwapchain      *swapchain,
            const VkcDevice         *device
            );
    ~VkcPipeline();

private:
    VkResult createShader(
            VkShaderModule          &shader,
            QString                 fileName,
            const VkcDevice         *device
            );
};

#endif // VKC_PIPELINE_H
