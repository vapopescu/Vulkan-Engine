#ifndef MGCAMERA_H
#define MGCAMERA_H

#include "stable.h"


/**
 * Class used for managing view and projection matrices.
 */
class MgCamera
{
    // Objects:
private:
    QMatrix4x4              axisMatrix;
    QMatrix4x4              projectionMatrix;

    QVector3D               position;
    QVector3D               lookAt;
    QVector3D               up;

    // Functions:
public:
    MgCamera();
    ~MgCamera();

    void setProjectionMatrix(
            float           verticalAngle,
            float           aspectRatio,
            float           nearPlane,
            float           farPlane
            );

    void getViewProjectionMatrix(
            QMatrix4x4*     pVPMatrix
            );
};

#endif // MGCAMERA_H
