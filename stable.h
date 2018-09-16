#ifndef STABLE_H
#define STABLE_H

// Qt
#include <QMainWindow>
#include <QApplication>
#include <QWidget>

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QImage>
#include <QPainter>

#include <QMouseEvent>

#include <QtMath>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>

#include <QDebug>

// Vulkan
#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan\vulkan.h>
#include <vulkan\vk_layer.h>

// Assimp
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

// Custom
#define mgAssert(result) \
{ \
    if (result < 0) \
    { \
        qDebug(QString("ERROR:   [@qDebug]").leftJustified(32, ' ') \
                .append("- Function returned %1 in %2 at line %3.") \
                .arg(result).arg(__FILE__).arg(__LINE__).toStdString().c_str()); \
        return result; \
    } \
}

#define mgOffsetOf(s,m) (reinterpret_cast<size_t>(&((static_cast<s*>(nullptr))->m)))

#endif // STABLE_H
