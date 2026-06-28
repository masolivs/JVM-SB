CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
TARGET = build/jvm.exe
OUTDIR = output

SRCS = src/main.cpp \
       src/opcodes.cpp \
       src/class_reader.cpp \
       src/constant_pool.cpp \
       src/attributes.cpp \
       src/displayer.cpp \
       src/frame.cpp \
       src/jvm_stack.cpp \
       src/object.cpp \
       src/array.cpp \
       src/method_area.cpp \
       src/interpreter.cpp \
       src/opcodes/arithmetic.cpp \
       src/opcodes/load_store.cpp \
       src/opcodes/stack_ops.cpp \
       src/opcodes/control.cpp \
       src/opcodes/invoke.cpp \
       src/opcodes/field_ops.cpp \
       src/opcodes/object_ops.cpp \
       src/opcodes/array_ops.cpp \
       src/opcodes/convert.cpp \
       src/opcodes/exceptions.cpp

all: dirs $(TARGET)

dirs:
	mkdir -p build/opcodes

$(TARGET): $(SRCS)
	$(CXX) $(CFLAGS) $(IFLAGS) $(SRCS) -o $(TARGET)

display: all # modo leitor
	./$(TARGET) -d $(CLASS)

output: all
	mkdir -p $(OUTDIR)
	for f in tests/class/*.class; do \
		./$(TARGET) -d -o $(OUTDIR)/$$(basename $$f .class).txt $$f && echo "Gerado: $(OUTDIR)/$$(basename $$f .class).txt"; \
	done

output-all: all
	mkdir -p $(OUTDIR)
	for f in tests/class/*.class; do \
		./$(TARGET) -d -o $(OUTDIR)/$$(basename $$f .class).txt $$f && echo "Gerado: $(OUTDIR)/$$(basename $$f .class).txt"; \
	done
	cat $(OUTDIR)/*.txt > $(OUTDIR)/all.txt
	@echo "Gerado: $(OUTDIR)/all.txt"

run: all # modo interpretador 
	./$(TARGET) tests/class/$(CLASS).class

run-all: all
	@for f in tests/class/*.class; do \
		echo "Executando: $$f"; \
		./$(TARGET) $$f; \
		echo ""; \
	done

test: all
	bash test.sh

check:
	cppcheck --enable=all --language=c++ \
	  --suppress=missingIncludeSystem \
	  -I include src/

sanitize: CFLAGS += -fsanitize=undefined -fno-omit-frame-pointer
sanitize: clean all

docs:
	doxygen Doxyfile

clean:
	rm -rf build

clean-docs:
	rm -rf docs/html

clean-output:
	rm -rf output

clean-all: clean clean-docs clean-output

.PHONY: all dirs display output output-all run test check sanitize docs clean clean-docs clean-output clean-all