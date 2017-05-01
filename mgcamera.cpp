#include "mgcamera.h"


/**
 * Initialize the camera.
 */
MgCamera::MgCamera()
{
    // Set default position and rotation.
    position = QVector3D(0.0f, -1.5f, 0.0f);
    rotation = QQuaternion(1.0f, 0.0f, 0.0f, 0.0f);

    // Set coordinate system (right handed, Z up)
    QVector3D right =   QVector3D( 1.0f,  0.0f,  0.0f);
    QVector3D front =   QVector3D( 0.0f,  1.0f,  0.0f);
    QVector3D up =      QVector3D( 0.0f,  0.0f,  1.0f);

    axisMatrix.setColumn(0, right.toVector4D());
    axisMatrix.setColumn(1, up.toVector4D());
    axisMatrix.setColumn(2, -front.toVector4D());
}


/**
 * Destroy the camera.
 */
MgCamera::~MgCamera()
{

}


//    float length = r - l;
//    float height = b - t;
//    float depth  = f - n;

//    mat4 matrix;
//    matrix[0][0] = 2 * n / length;
//    matrix[2][0] = - (r + l) / length;

//    matrix[1][1] = 2 * n / height;
//    matrix[2][1] = - (b + t) / height;

//    matrix[2][2] = F * f - N * n / depth;
//    matrix[3][2] = (N - F) * f * n / depth;

//    matrix[2][3] = 1;
//    matrix[3][3] = 0;


//    mat4 matrix;
//    matrix[0][0] = 2 * n / w;

//    matrix[1][1] = 2 * n / h;

//    matrix[2][2] = F * f - N * n / (f - n);
//    matrix[3][2] = (N - F) * f * n / (f - n);

//    matrix[2][3] = 1;
//    matrix[3][3] = 0;

/**
 * Set the projection matrix as perspective.
 */
void MgCamera::setProjectionMatrix(float verticalAngle, float aspectRatio, float nearPlane, float farPlane)
{
    float dy = 1 / tan(verticalAngle / 2);
    float dx = dy / aspectRatio;
    float dz = -nearPlane / (farPlane - nearPlane);
    float dw = -farPlane * dz;

    projectionMatrix.setRow(0, {dx,  0,  0,  0});
    projectionMatrix.setRow(1, { 0, dy,  0,  0});
    projectionMatrix.setRow(2, { 0,  0, dz, dw});
    projectionMatrix.setRow(3, { 0,  0,  1,  0});
}

/**
 * Get view-projection matrix.
 */
void MgCamera::getViewProjectionMatrix(QMatrix4x4 *pVPMatrix)
{
    QMatrix4x4 viewMatrix = axisMatrix;
    viewMatrix.rotate(rotation.inverted());
    viewMatrix.translate(-position);

    *pVPMatrix = projectionMatrix * viewMatrix;
}
