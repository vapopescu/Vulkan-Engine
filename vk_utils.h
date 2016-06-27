#ifndef VK_UTILS_H
#define VK_UTILS_H

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG == 1
#include <QDebug>
#endif


typedef enum Resolution{
    WIDTH = 1600,
    HEIGHT = 900
} Resolution;

#define CLEAR_COLOR  {0.8f, 0.8f, 0.9f, 1.1f}


/**
 * Struct used to define vertex shader input layout and buffer size.
 */
struct VkVertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
};

#endif // VK_UTILS_H
