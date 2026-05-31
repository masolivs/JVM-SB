CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
TARGET = build\jvm.exe

SRCS = src\main.cpp \
       src\opcodes.cpp \
       src\class_reader.cpp \
       src\constant_pool.cpp \
       src\attributes.cpp \
       src\displayer.cpp \
       src\frame.cpp \
       src\jvm_stack.cpp \
       src\object.cpp \
       src\array.cpp \
       src\method_area.cpp \
       src\interpreter.cpp \
       src\opcodes\arithmetic.cpp \
       src\opcodes\load_store.cpp \
       src\opcodes\stack_ops.cpp \
       src\opcodes\control.cpp \
       src\opcodes\invoke.cpp \
       src\opcodes\field_ops.cpp \
       src\opcodes\object_ops.cpp \
       src\opcodes\array_ops.cpp \
       src\opcodes\convert.cpp \
       src\opcodes\exceptions.cpp

all: dirs $(TARGET)

dirs:
	if not exist build mkdir build
	if not exist build\opcodes mkdir build\opcodes

$(TARGET): $(SRCS)
	$(CXX) $(CFLAGS) $(IFLAGS) $(SRCS) -o $(TARGET)

display: all
	$(TARGET) -d $(CLASS)

run: all
	$(TARGET) tests\class\$(CLASS).class

test: all
	test.bat

check:
	cppcheck --enable=all --language=c++ \
	  --suppress=missingIncludeSystem \
	  -I include src\

sanitize: CFLAGS += -fsanitize=undefined -fno-omit-frame-pointer
sanitize: clean all

docs:
	doxygen Doxyfile

clean:
	if exist build rmdir /s /q build

clean-docs:
	if exist docs\html rmdir /s /q docs\html

clean-all: clean clean-docs

.PHONY: all dirs display run test check sanitize docs clean clean-docs clean-all
