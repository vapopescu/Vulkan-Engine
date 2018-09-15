#ifndef VKC_ENTITY_H
#define VKC_ENTITY_H

#include "stable.h"
#include "vkc_device.h"
#include "vkc_context.h"
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
    QVector<MgVertex>           vertices;
    QVector<uint32_t>           indices;

    MgBuffer                    buffer;

    QVector3D                   position =      QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D                   scale =         QVector3D(1.0f, 1.0f, 1.0f);
    QQuaternion                 rotation =      QQuaternion(1.0f, 0.0f, 0.0f, 0.0f);

    MgImage                     diffuseMap;
    MgImage                     normalMap;
    MgImage                     specularMap;

    const VkcDevice             *pDevice =      nullptr;

    float                       dir =           1.0f / 60.0f;

    // Functions:
public:
    VkResult create(
            const VkcDevice     *pDevice,
            QString             name
            );
    void destroy();

    VkResult render(
            VkcContext          *pContext,
            VkCommandBuffer     commandBuffer,
            QMatrix4x4          vpMatrix
            );
};

#endif // VKC_ENTITY_H
