#ifndef MGTEXTURE2D_H
#define MGTEXTURE2D_H

#include "stable.h"
#include "mgimage.h"

/**
 * Subclass used for 2D textures.
 */
class MgTexture2D : public MgImage
{
public:
    VkResult create(
            const VkcDevice*    pDevice,
            const QString       filePath
            );

    VkResult loadImageData(
            const VkcDevice*    pDevice,
            const QImage*       pImageData,
            int                 mipLevelCount
            );
};

#endif // MGTEXTURE2D_H
