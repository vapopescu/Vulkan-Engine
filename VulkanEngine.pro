#-------------------------------------------------
#
# Project created by QtCreator 2016-06-17T16:29:53
#
#-------------------------------------------------

QT += core gui
unix: QT += x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = "Vulkan Engine"
TEMPLATE = app

HEADERS += \
    appwindow.h \
    vkc_swapchain.h \
    vkc_image.h \
    vkc_device.h \
    vkc_entity.h \
    vkc_pipeline.h \
    vkc_camera.h \
    vkc_buffer.h \
    vkc_instance.h \
    ui_form.h

SOURCES += \
    main.cpp \
    appwindow.cpp \
    vkc_swapchain.cpp \
    vkc_image.cpp \
    vkc_device.cpp \
    vkc_entity.cpp \
    vkc_pipeline.cpp \
    vkc_camera.cpp \
    vkc_buffer.cpp \
    vkc_instance.cpp \
    ui_form.cpp

FORMS += \
    appwindow.ui \
    gui.ui

DISTFILES += \
    shader.vert \
    shader.frag

QMAKE_CXXFLAGS += \
    -Wno-pointer-arith

VK_SDK = "C:/VulkanSDK/1.0.17.0"

INCLUDEPATH +=          $$VK_SDK/Include/vulkan

unix|win32: LIBS +=     -L$$VK_SDK/Bin32/   -lvulkan-1
win64: LIBS +=          -L$$VK_SDK/Bin/     -lvulkan-1
