#ifndef VKC_PIPELINE_H
#define VKC_PIPELINE_H

#include "stable.h"
#include "mgimage.h"
#include "mgbuffer.h"
#include "vkc_device.h"
#include "vkc_swapchain.h"

/**
 * Struct used to define vertex shader input layout and buffer size.
 */
struct MgVertex {
    float x, y, z;
    float u, v, w;
    float nx, ny, nz;
    float tx, ty, tz;
    float bx, by, bz;
    float i0, i1, i2, i3;
    float w0, w1, w2, w3;
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
    VkDescriptorSetLayout           descriptorSetLayout;

    QHash<uint32_t, MgBuffer>       bindBuffers;

private:
    const VkcDevice                 *pDevice;

    // Functions:
public:
    VkcPipeline();
    VkcPipeline(
            const VkcSwapchain      *swapchain,
            const VkcDevice         *device
            );
    ~VkcPipeline();

    VkResult bindFloatv(
            uint32_t                binding,
            float                   *values,
            uint32_t                size,
            uint32_t                maxSize = 0
            );
    void bindImage(
            uint32_t                binding,
            MgImage                 image
            );

private:
    VkResult createShader(
            VkShaderModule          &shader,
            QString                 fileName,
            const VkcDevice         *device
            );
};

#endif // VKC_PIPELINE_H
