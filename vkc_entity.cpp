#include "vkc_entity.h"


/**
 * Initialize with empty fields.
 */
VkcEntity::VkcEntity()
{
    vertices =          {};
    indices =           {};

    modelMatrix =       {};

    buffer =            {};
}


/**
 * Initialize and create the entity.
 */
VkcEntity::VkcEntity(VkcDevice device)
{
    VkcEntity();
    create(device);
}


/**
 * We will handle the cleanup ourselves.
 */
VkcEntity::~VkcEntity()
{
    //destroy();
}


/**
 * Create the entity.
 */
void VkcEntity::create(VkcDevice device)
{
    //Load the model.
    vertices.append({-1.0f,  1.0f,  0.0f,  1.0f});
    vertices.append({ 1.0f,  1.0f,  0.0f,  1.0f});
    vertices.append({ 1.0f, -1.0f,  0.0f,  1.0f});
    vertices.append({-1.0f, -1.0f,  0.0f,  1.0f});

    indices = {0, 1, 2, 0, 2, 3};

    //Move entity.
    modelMatrix.translate(0.0f, 0.0f, 1.5f);

    //Create buffer.
    buffer.create(vertices.size() * sizeof(VkVertex) + indices.size() * sizeof(uint32_t),
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, device);

    //Map buffer memory to host.
    void *data = NULL;
    vkMapMemory(device.logical, buffer.memory, 0, VK_WHOLE_SIZE, 0, &data);

    //Copy data to the buffer.
    uint32_t offset = 0;
    memcpy(data + offset, vertices.data(), vertices.size() * sizeof(VkVertex));

    offset += vertices.size() * sizeof(VkVertex);
    memcpy(data + offset, indices.data(), indices.size() * sizeof(uint32_t));

    //Unmap memory.
    vkUnmapMemory(device.logical, buffer.memory);
}


/**
 * Destroy the entity.
 */
void VkcEntity::destroy()
{
    buffer.destroy();
}


/**
 * Register the commands that render the entity.
 */
void VkcEntity::render(VkCommandBuffer commandBuffer, VkcBuffer uniformBuffer, QMatrix4x4 vpMatrix, VkcDevice device)
{
    //Calculate MVP matrix.
    QMatrix4x4 mvpMatrix = vpMatrix * modelMatrix;

    //Map uniform buffer memory to host.
    void *data = NULL;
    vkMapMemory(device.logical, uniformBuffer.memory, 0, VK_WHOLE_SIZE, 0, &data);

    //Copy data to the buffer.
    memcpy(data, mvpMatrix.data(), 16 * sizeof(float));

    //Unmap memory.
    vkUnmapMemory(device.logical, uniformBuffer.memory);


    //Bind vertex and index bufffer.
    VkDeviceSize vboOffsets[] = {0};
    uint32_t iboOffset = vertices.size() * sizeof(VkVertex);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer.handle, vboOffsets);
    vkCmdBindIndexBuffer(commandBuffer, buffer.handle, iboOffset, VK_INDEX_TYPE_UINT32);


    //Draw entity.
    vkCmdDrawIndexed(commandBuffer, indices.size(), 1, 0, 0, 0);
}



