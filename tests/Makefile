NAME = haichi_layout_tests
TYPE = APP

SRCS = \
	LayoutMathTest.cpp \
	../src/PositionScaling.cpp

LIBS = be $(STDCPPLIBS)
LOCAL_INCLUDE_PATHS = ../src
OPTIMIZE := FULL
WARNINGS = ALL
COMPILER_FLAGS =

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

.PHONY: check
check: $(TARGET_DIR)/$(NAME)
	$(TARGET_DIR)/$(NAME)
