SRC_DIR := src
OBJ_DIR := obj
MODULES := membership query filesystem ml_cluster
SRC_DIRS  := $(addprefix $(SRC_DIR)/,$(MODULES))
SRC_FILES := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp))
SRC_FILES += $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CC := g++
LDFLAGS := -pthread  -lpthread
CPPFLAGS := -g -Wall -std=c++17
CXXFLAGS := 

all: client server

client: $(OBJ_FILES) client.cpp
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

server: $(OBJ_FILES) server.cpp
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

mytest: $(OBJ_FILES) mytest.cpp
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -fr obj/*
	rm client server

