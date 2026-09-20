CXX       ?= g++
TARGET    ?= main
BUILD_DIR ?= build


# General configuration

CPPFLAGS := -I333_Solver_cmd -Icpp-httplib -MMD -MP
CXXFLAGS := -Wall -fexceptions -std=c++23
LDLIBS   :=

SRCS     := main.cpp \
		    handle_request.cpp \
		    build_input.cpp \
            333_Solver_cmd/Coord.cpp \
            333_Solver_cmd/Cubie.cpp \
            333_Solver_cmd/Move.cpp \
            333_Solver_cmd/Solver.cpp \
            333_Solver_cmd/Table.cpp

OBJS     := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS     := $(OBJS:.o=.d)


# Build mode (release|debug)

BUILD ?= release

ifeq ($(BUILD),debug)
	CXXFLAGS += -g
else ifeq ($(BUILD),release)
	CXXFLAGS += -O2
else
	$(error Unknown BUILD configuration: $(BUILD))
endif


# Platform (Windows|Linux)

ifeq ($(OS),Windows_NT)
	CPPFLAGS  += -D_WIN32_WINNT=0x0A00
	LDLIBS    += -lws2_32
	EXTENSION := .exe
else
	EXTENSION :=
endif

BIN := $(BUILD_DIR)/$(TARGET)$(EXTENSION)


# Targets

.DEFAULT_GOAL := build

-include $(DEPS)

$(BIN): $(OBJS)
	$(CXX) $(OBJS) -o $(BIN) $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp
ifeq ($(OS),Windows_NT)
	if not exist "$(dir $@)" mkdir "$(dir $@)"
else
	mkdir -p "$(dir $@)"
endif
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

update:
	cd 333_Solver_cmd && git fetch && git rebase origin/main
	cd cpp-httplib    && git fetch && git rebase origin/master

clean:
ifeq ($(OS),Windows_NT)
	if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
else
	rm -rf "$(BUILD_DIR)"
endif

build: $(BIN)
