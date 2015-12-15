#-------------------------------------------------
#
# Project created by QtCreator 2015-12-12T17:44:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cpp-size
TEMPLATE = app

SOURCES += \
	src/main.cpp\
    src/ui/dialog.cpp \
    contrib/cpp_dep/cpp_dep.cpp

HEADERS  += \
	src/ui/dialog.hpp \
	src/ui/include_tree_widget_item.hpp \
    contrib/cpp_dep/cpp_dep.hpp

FORMS += forms/dialog.ui

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += contrib
INCLUDEPATH += src
INCLUDEPATH += $(BOOST_ROOT)
