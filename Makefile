CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
TARGET = build\jvm.exe
OUTDIR = output

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

output: all
	if not exist $(OUTDIR) mkdir $(OUTDIR)
	for %%f in (tests\class\*.class) do \
		$(TARGET) -d -o $(OUTDIR)\%%~nf.txt %%f && echo Gerado: $(OUTDIR)\%%~nf.txt

output-all: all
	if not exist $(OUTDIR) mkdir $(OUTDIR)
	for %%f in (tests\class\*.class) do \
		$(TARGET) -d -o $(OUTDIR)\%%~nf.txt %%f && echo Gerado: $(OUTDIR)\%%~nf.txt
	type $(OUTDIR)\*.txt > $(OUTDIR)\all.txt
	@echo Gerado: $(OUTDIR)\all.txt

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

clean-output:
	if exist output rmdir /s /q output

clean-all: clean clean-docs clean-output

.PHONY: all dirs display output output-all run test check sanitize docs clean clean-docs clean-output clean-all