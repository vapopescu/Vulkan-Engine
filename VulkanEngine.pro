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

PRECOMPILED_HEADER = stable.h

HEADERS += \
    vkc_swapchain.h \
    vkc_device.h \
    vkc_entity.h \
    vkc_pipeline.h \
    vkc_instance.h \
    vkc_context.h \
    mgwindow.h \
    mgimage.h \
    stable.h \
    mgtexture2d.h \
    mgcamera.h \
    mgbuffer.h

SOURCES += \
    main.cpp \
    vkc_swapchain.cpp \
    vkc_device.cpp \
    vkc_entity.cpp \
    vkc_pipeline.cpp \
    vkc_instance.cpp \
    vkc_context.cpp \
    mgwindow.cpp \
    mgimage.cpp \
    mgtexture2d.cpp \
    mgcamera.cpp \
    mgbuffer.cpp

FORMS += \
    mgwindow.ui

DISTFILES += \
    shader.vert \
    shader.frag

INCLUDEPATH += \
    $$(VULKAN_SDK)/Include/vulkan

contains(QT_ARCH, i386) {
    LIBS += \
        -L$$(VULKAN_SDK)/Lib32/ -lvulkan-1
} else {
    LIBS += \
        -L$$(VULKAN_SDK)/Lib/ -lvulkan-1
}
