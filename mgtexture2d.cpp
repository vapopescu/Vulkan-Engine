#include "mgtexture2d.h"

/**
 * Create the texture.
 */
VkResult MgTexture2D::create(const VkcDevice* pDevice, const QString filePath)
{
    QImage imageData;

    if (!imageData.load(filePath))
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Prepare the QImage.
    imageData = imageData.mirrored().rgbSwapped();
    imageData.convertToFormat(QImage::Format_RGBA8888);

    int width = qNextPowerOfTwo(imageData.width() - 1);
    int height = qNextPowerOfTwo(imageData.height() - 1);

    imageData = imageData.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int maxDimension = imageData.width() > imageData.height() ? imageData.width() : imageData.height();
    uint32_t mipLevelCount = 1;

    while (maxDimension > 1)
    {
        maxDimension /= 2;
        ++mipLevelCount;
    }

    // Load the QImage into the buffer.
    mgAssert(loadImageData(pDevice, &imageData, mipLevelCount));

    MgImageInfo imageInfo =
    {
        VK_IMAGE_TYPE_2D,                           // VkImageType               type;
        {                                           // VkExtent3D                extent;
          (uint32_t) imageData.width(),                 // uint32_t              width;
          (uint32_t) imageData.height(),                // uint32_t              height;
          1                                             // uint32_t              depth;
        },
        VK_FORMAT_R8G8B8A8_UNORM,                   // VkFormat                  format;
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   // VkImageLayout             layout;
        0 |                                         // VkImageUsageFlags         usage;
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT,
        {                                           // VkImageSubresourceRange   resourceRange;
            VK_IMAGE_ASPECT_COLOR_BIT,                  // VkImageAspectFlags    aspectMask;
            0,                                          // uint32_t              baseMipLevel;
            1,                                          // uint32_t              levelCount;
            0,                                          // uint32_t              baseArrayLayer;
            1,                                          // uint32_t              layerCount;
        },

        VK_NULL_HANDLE,                             // const VkImage             image;
        true,                                       // bool                      createView;
        true                                        // bool                      createSampler;
    };

    mgAssert(MgImage::create(pDevice, &imageInfo));

    return VK_SUCCESS;
}

/**
 * Load data into the image buffer.
 */
VkResult MgTexture2D::loadImageData(const VkcDevice *pDevice, const QImage *pImageData, int mipLevelCount)
{
    // If image buffer exists, destroy it.
    imageBuffer.destroy();

    // Create image buffer.
    uint32_t size = pImageData->width() * pImageData->height() * 4;
    imageBuffer.create(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, pDevice);

    // Map buffer memory to host.
    void *data;
    mgAssert(vkMapMemory(pDevice->logical, imageBuffer.memory, 0, VK_WHOLE_SIZE, 0, &data));

    // Copy data to the buffer.
    memcpy(data, pImageData->bits(), pImageData->byteCount());

    // Unmap memory.
    vkUnmapMemory(pDevice->logical, imageBuffer.memory);

    // If image is not at creation, prepare it.
    if (handle != VK_NULL_HANDLE)
        mgAssert(loadImage(pDevice));

    return VK_SUCCESS;
}
