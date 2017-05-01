#ifndef STABLE_H
#define STABLE_H

#include <QMainWindow>
#include <QApplication>
#include <QWidget>

#include <QFile>
#include <QTimer>
#include <QVector>
#include <QImage>
#include <QPainter>

#include <QMouseEvent>

#include <QtMath>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>

#include <QDebug>

#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan.h>
#include <vk_layer.h>

#define mgAssert(result) if (result < 0) return result

#endif // STABLE_H
