lib_srcs := src/gui/abstract_gobj.cpp src/gui/abstract_gshape.cpp src/gui/gui.cpp src/gui/window.cpp
lib_srcs += $(shell find src/gui/picking -name "*.cpp")
lib_srcs += $(shell find src/gui/shapes -name "*.cpp")
lib_srcs += $(shell find src/gui/style -name "*.cpp")
lib_srcs += $(shell find src/gui/transformation -name "*.cpp")
lib_srcs += $(shell find src/gui/XML -name "*.cpp")

ifeq ($(graphics),QT)
include src/gui/qt/djnn-lib.mk

$(build_dir)/src/gui/qt/moc_MyQWindow.cpp: src/gui/qt/my_qwindow.h src/gui/backend.h
	$(moc) $< > $@

lib_objs += $(build_dir)/src/gui/qt/moc_MyQWindow.o
lib_srcgens += $(build_dir)/src/gui/qt/moc_MyQWindow.cpp
lib_srcs += $(shell find src/gui/qt -name "*.cpp")
endif

ifeq ($(graphics),SDL)
include src/gui/sdl/djnn-lib.mk
endif

lib_djnn_deps = core
