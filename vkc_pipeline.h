#ifndef VKC_PIPELINE_H
#define VKC_PIPELINE_H

#include <QFile>
#include <QVector>

#include <vulkan.h>
#include <vk_utils.h>

#include <vkc_device.h>

/**
 * Class used for the graphics pipeline.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcPipeline
{
    //Objects:
public:
    VkPipeline                      handle =            VK_NULL_HANDLE;
    VkPipelineLayout                layout =            VK_NULL_HANDLE;

    VkShaderModule                  vertShader =        VK_NULL_HANDLE;
    VkShaderModule                  fragShader =        VK_NULL_HANDLE;

    VkDescriptorPool                descriptorPool =    VK_NULL_HANDLE;
    QVector<VkDescriptorSet>        descriptorSets =    {};
    QVector<VkDescriptorSetLayout>  setLayouts =        {};

private:
    VkDevice                        logicalDevice =     VK_NULL_HANDLE;

    //Functions:
public:
    VkcPipeline();
    VkcPipeline(
            VkRenderPass    renderPass,
            VkcDevice       device
            );
    ~VkcPipeline();

    void create(
            VkRenderPass    renderPass,
            VkcDevice       device
            );
    void destroy();

private:
    void createShader(
            VkShaderModule &shader,
            const char *fileName,
            VkcDevice device
            );
};

#endif // VKC_PIPELINE_H
