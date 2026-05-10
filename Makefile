CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
TARGET = build\jvm.exe

SRCS = src\main.cpp

OBJS = $(patsubst src\%.cpp, build\%.o, $(SRCS))

all: build $(TARGET)

# cria a pasta build se não existir
build:
	if not exist build mkdir build

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(IFLAGS) $^ -o $@

build\%.o: src\%.cpp
	$(CXX) $(CFLAGS) $(IFLAGS) -c $< -o $@

check:
	cppcheck --enable=all --language=c++ \
	  --suppress=missingIncludeSystem \
	  -I include src\

sanitize: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean all

docs:
	doxygen Doxyfile

display: all
	$(TARGET) -d $(CLASS)

run: all
	$(TARGET) tests\class\$(CLASS).class

clean:
	if exist build rmdir /s /q build

clean-docs:
	if exist docs\html rmdir /s /q docs\html

clean-all: clean clean-docs