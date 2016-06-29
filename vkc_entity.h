#ifndef VKC_ENTITY_H
#define VKC_ENTITY_H

#include <QMatrix4x4>
#if DEBUG == 1
#include <QDebug>
#endif

#include <vulkan.h>

#include <vkc_device.h>
#include <vkc_buffer.h>
#include <vkc_pipeline.h>


/**
 * Class used for entities.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcEntity
{
    //Objects:
protected:
    QVector<VkVertex>   vertices;
    QVector<uint32_t>   indices;

    QMatrix4x4          modelMatrix;

    VkcBuffer           *buffer;

    //Functions:
public:
    VkcEntity();
    VkcEntity(
            const VkcDevice     *device
            );
    ~VkcEntity();

    void render(
            VkCommandBuffer     commandBuffer,
            const VkcBuffer     *uniformBuffer,
            QMatrix4x4          vpMatrix,
            const VkcDevice     *device
            );
};

#endif // VKC_ENTITY_H
