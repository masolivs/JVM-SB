CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
TARGET = build/leitor

SRCS = src/main.cpp \
       src/class_reader.cpp \
       src/constant_pool.cpp \
       src/attributes.cpp \
       src/displayer.cpp \
       src/opcodes.cpp

OBJS = $(patsubst src/%.cpp, build/%.o, $(SRCS))

all: dirs $(TARGET)

dirs:
	@mkdir -p build

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(IFLAGS) $^ -o $@

build/%.o: src/%.cpp
	$(CXX) $(CFLAGS) $(IFLAGS) -c $< -o $@

run: all
	$(TARGET) $(CLASS)

clean:
	rm -rf build

.PHONY: all dirs run clean
