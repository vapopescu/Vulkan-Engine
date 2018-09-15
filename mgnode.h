#ifndef MGNODE_H
#define MGNODE_H

#include "stable.h"


class MgNode
{
    // Objects
protected:
    QVector3D           position =      QVector3D( 0.0f,  0.0f,  0.0f);
    QQuaternion         rotation =      QQuaternion( 1.0f,  0.0f,  0.0f,  0.0f);
    QVector3D           scale =         QVector3D( 1.0f,  1.0f,  1.0f);

    QMatrix4x4          matrix =        QMatrix4x4();

    MgNode              *parent =       nullptr;
    QVector<MgNode*>    children =      {};

public:
    void setParent(
            MgNode *parent
            );
    void addChild(
            MgNode *child
            );

    void removeParent();
    void removeChild(
            MgNode *child
            );

    void computeMatrix();
    void getMatrix(
            QMatrix4x4 *matrix
            );
};

#endif // MGNODE_H
