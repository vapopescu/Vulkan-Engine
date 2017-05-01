#include "vkc_entity.h"


/**
 * Initialize with empty fields.
 */
VkcEntity::VkcEntity()
{
    position =  QVector3D(0.0f, 0.0f, 0.0f);
    scale =     QVector3D(1.0f, 1.0f, 1.0f);
    rotation =  QQuaternion(1.0f, 0.0f, 0.0f, 0.0f);

    dir = 0.1f / 15.0f;
}


/**
 * Create the entity.
 */
VkcEntity::VkcEntity(const VkcDevice *device) : VkcEntity()
{
    // Load the model.
    vertices.append({-1.0f,  0.0f,  1.0f,    0.0f,  1.0f,    0.0f, -1.0f,  0.0f});
    vertices.append({-1.0f,  0.0f, -1.0f,    0.0f,  0.0f,    0.0f, -1.0f,  0.0f});
    vertices.append({ 1.0f,  0.0f,  1.0f,    1.0f,  1.0f,    0.0f, -1.0f,  0.0f});
    vertices.append({ 1.0f,  0.0f, -1.0f,    1.0f,  0.0f,    0.0f, -1.0f,  0.0f});

    indices = {0, 1, 2, 2, 1, 3};

    // Load position, scale and rotation data.
    // /@todo

    // Create buffer.
    buffer.create(vertices.size() * sizeof(VkVertex) + indices.size() * sizeof(uint32_t),
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, device);

    // Map buffer memory to host.
    uint8_t *data;
    vkMapMemory(device->logical, buffer.memory, 0, VK_WHOLE_SIZE, 0,(void**) &data);

    // Copy data to the buffer.
    uint32_t offset = 0;
    memcpy(data + offset, vertices.data(), vertices.size() * sizeof(VkVertex));

    offset += vertices.size() * sizeof(VkVertex);
    memcpy(data + offset, indices.data(), indices.size() * sizeof(uint32_t));

    // Unmap memory.
    vkUnmapMemory(device->logical, buffer.memory);
}


/**
 * Destroy the entity.
 */
VkcEntity::~VkcEntity()
{
    buffer.destroy();
}


/**
 * Register the commands that render the entity.
 */
void VkcEntity::render(VkCommandBuffer commandBuffer, MgBuffer uniformBuffer, QMatrix4x4 vpMatrix, const VkcDevice *device)
{
    // Wiggle, wiggle, wiggle.
    position += QVector3D(dir, 0.0f, 0.0f);
    float pos = position.x();

    if(pos > 0.1f || pos < -0.1f)
        dir *= -1.0f;

    // Calculate model matrix.
    QMatrix4x4 modelMatrix;
    modelMatrix.translate(position);
    modelMatrix.rotate(rotation);
    modelMatrix.scale(scale);

    // Calculate MVP matrix.
    QMatrix4x4 mvpMatrix = vpMatrix * modelMatrix;

    // Map uniform buffer memory to host.
    void *data = nullptr;
    vkMapMemory(device->logical, uniformBuffer.memory, 0, VK_WHOLE_SIZE, 0, &data);

    // Copy data to the buffer.
    memcpy(data, mvpMatrix.data(), 16 * sizeof(float));

    // Unmap memory.
    vkUnmapMemory(device->logical, uniformBuffer.memory);

    // Bind vertex and index bufffer.
    VkDeviceSize vboOffsets[] = {0};
    uint32_t iboOffset = vertices.size() * sizeof(VkVertex);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer.handle, vboOffsets);
    vkCmdBindIndexBuffer(commandBuffer, buffer.handle, iboOffset, VK_INDEX_TYPE_UINT32);

    // Draw entity.
    vkCmdDrawIndexed(commandBuffer, indices.size(), 1, 0, 0, 0);
}



