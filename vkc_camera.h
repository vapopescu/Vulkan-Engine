#ifndef VKC_CAMERA_H
#define VKC_CAMERA_H

#include <QtMath>
#include <QMatrix4x4>
#if DEBUG == 1
#include <QDebug>
#endif

/**
 * Class used for managing view and projection matrices.
 *
 * Classes named "Vkc[class]" stand for "Vulkan custom class".
 */
class VkcCamera
{
    //Objects:
private:
    QMatrix4x4      viewMatrix;
    QMatrix4x4      projectionMatrix;

    //Functions:
public:
    VkcCamera();
    ~VkcCamera();

    void setProjectionMatrix(
            float verticalAngle,
            float aspectRatio,
            float nearPlane,
            float farPlane
            );

    void getViewProjectionMatrix(
            QMatrix4x4      &vpMatrix
            );
};

#endif // VKC_CAMERA_H
