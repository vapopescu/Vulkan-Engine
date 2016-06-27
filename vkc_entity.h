#ifndef VKC_ENTITY_H
#define VKC_ENTITY_H

#include <QMatrix4x4>

#include <vulkan.h>
#include <vk_utils.h>

#include <vkc_device.h>
#include <vkc_buffer.h>


/**
 * Class used for entities.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcEntity
{
    //Objects:
public:
    QVector<VkVertex>   vertices =          {};
    QVector<uint32_t>   indices =           {};

    QMatrix4x4          modelMatrix =       {};

    VkcBuffer           buffer =            {};

    //Functions:
public:
    VkcEntity();
    VkcEntity(
            VkcDevice       device
            );
    ~VkcEntity();

    void create(
            VkcDevice       device
            );
    void destroy();

    void render(
            VkCommandBuffer commandBuffer,
            VkcBuffer       uniformBuffer,
            QMatrix4x4      vpMatrix,
            VkcDevice       device
            );
};

#endif // VKC_ENTITY_H
