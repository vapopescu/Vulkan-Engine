#include "vkc_camera.h"


/**
 * Initialize with empty fields.
 */
VkcCamera::VkcCamera()
{
    viewMatrix =            {};
    projectionMatrix =      {};
}


//    float length = r - l;
//    float height = b - t;
//    float depth  = f - n;

//    mat4 matrix;
//    matrix[0][0] = 2 * n / length;
//    matrix[2][0] = - (r + l) / length;

//    matrix[1][1] = 2 * n / height;
//    matrix[2][1] = - (b + t) / height;

//    matrix[2][2] = f / depth;
//    matrix[3][2] = - f * n / depth;

//    matrix[2][3] = 1;
//    matrix[3][3] = 0;


//    mat4 matrix;
//    matrix[0][0] = 2 * n / w;

//    matrix[1][1] = 2 * n / h;

//    matrix[2][2] = f / (f - n);
//    matrix[3][2] = - f * n / (f - n);

//    matrix[2][3] = 1;
//    matrix[3][3] = 0;


void VkcCamera::setProjectionMatrix(float verticalAngle, float aspectRatio, float nearPlane, float farPlane)
{
    float dy = 1 / tan(verticalAngle / 2);
    float dx = dy / aspectRatio;
    float dz = farPlane / (farPlane - nearPlane);
    float dw = -nearPlane * dz;

    projectionMatrix.setRow(0, {dx,  0,  0,  0});
    projectionMatrix.setRow(1, { 0, dy,  0,  0});
    projectionMatrix.setRow(2, { 0,  0, dz, dw});
    projectionMatrix.setRow(3, { 0,  0,  1,  0});
}

/**
 * Get projection matrix * view matrix.
 */
void VkcCamera::getViewProjectionMatrix(QMatrix4x4 &vpMatrix)
{
    vpMatrix = projectionMatrix * viewMatrix;
}
