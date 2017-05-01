#ifndef VKC_ENTITY_H
#define VKC_ENTITY_H

#include "stable.h"
#include "vkc_device.h"
#include "mgbuffer.h"
#include "vkc_pipeline.h"


/**
 * Class used for entities.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcEntity
{
    // Objects:
protected:
    QVector<VkVertex>           vertices;
    QVector<uint32_t>           indices;

    MgBuffer                    buffer;

    QVector3D                   position;
    QVector3D                   scale;
    QQuaternion                 rotation;

    float                       dir;

    // Functions:
public:
    VkcEntity();
    VkcEntity(
            const VkcDevice     *device
            );
    ~VkcEntity();

    void render(
            VkCommandBuffer     commandBuffer,
            MgBuffer           uniformBuffer,
            QMatrix4x4          vpMatrix,
            const VkcDevice     *device
            );
};

#endif // VKC_ENTITY_H
