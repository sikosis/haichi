NAME = haichi
TYPE = APP
APP_MIME_SIG = application/x-vnd.Sikosis-haichi

SRCS = \
	src/AboutWindow.cpp \
	src/AttrBackend.cpp \
	src/DesktopPreview.cpp \
	src/HaichiApplication.cpp \
	src/HaichiEngine.cpp \
	src/LayoutStore.cpp \
	src/MainWindow.cpp \
	src/PositionScaling.cpp \
	src/TrayView.cpp \
	src/main.cpp

RDEFS = resources/haichi.rdef
LIBS = be $(STDCPPLIBS)
SYSTEM_INCLUDE_PATHS =
LOCAL_INCLUDE_PATHS = src
OPTIMIZE := FULL
WARNINGS = ALL
COMPILER_FLAGS =

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

.PHONY: check prepare-release
check: $(TARGET_DIR)/$(NAME)
	$(MAKE) -C tests check

prepare-release:
	./scripts/prepare-release.sh
