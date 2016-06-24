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
    vk_settings.h \
    vk_context.h

SOURCES += \
    main.cpp \
    appwindow.cpp \
    vkc_swapchain.cpp \
    vkc_image.cpp \
    vkc_device.cpp \
    vk_context.cpp

FORMS += \
    appwindow.ui

DISTFILES += \
    shader.vert \
    shader.frag

QMAKE_CXXFLAGS += \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-missing-field-initializers

VK_SDK = "C:/Program Files/VulkanSDK 1.0.13.0"

INCLUDEPATH +=          $$VK_SDK/Include/vulkan

unix|win32: LIBS +=     -L$$VK_SDK/Bin32/   -lvulkan-1
win64: LIBS +=          -L$$VK_SDK/Bin/     -lvulkan-1
