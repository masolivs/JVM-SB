CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
TARGET = build/leitor
OUTDIR = output

SRCS = src/main.cpp \
       src/class_reader.cpp \
       src/constant_pool.cpp \
       src/attributes.cpp \
       src/displayer.cpp \
       src/opcodes.cpp

OBJS = $(patsubst src/%.cpp, build/%.o, $(SRCS))

all: dirs $(TARGET)

dirs:
	mkdir -p build

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(IFLAGS) $^ -o $@

build/%.o: src/%.cpp
	$(CXX) $(CFLAGS) $(IFLAGS) -c $< -o $@

display: all
	$(TARGET) exemplos/$(CLASS).class

test: all
	@for f in exemplos/*.class; do \
		$(TARGET) $$f > /dev/null 2>&1 \
		&& echo "PASS $$f" \
		|| echo "FAIL $$f"; \
	done

txt: all
	mkdir -p $(OUTDIR)
	$(TARGET) exemplos/$(CLASS).class > $(OUTDIR)/$(CLASS).txt
	@echo "Gerado: $(OUTDIR)/$(CLASS).txt"

txt-all: all
	mkdir -p $(OUTDIR)
	@for f in exemplos/*.class; do \
		name=$$(basename $$f .class); \
		$(TARGET) $$f > $(OUTDIR)/$$name.txt && echo "Gerado: $(OUTDIR)/$$name.txt"; \
	done

TARGET_VER ?= 8

javac:
	javac -source $(TARGET_VER) -target $(TARGET_VER) -d exemplos $(JAVA)

javac-all:
	javac -source $(TARGET_VER) -target $(TARGET_VER) -d exemplos exemplos/*.java

check:
	cppcheck --enable=all --language=c++ \
	  --suppress=missingIncludeSystem \
	  -I include src/

sanitize: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean all

sanitize-thread: CFLAGS += -fsanitize=thread -fno-omit-frame-pointer
sanitize-thread: clean all

docs:
	doxygen Doxyfile

clean:
	rm -rf build

clean-docs:
	rm -rf docs/html

clean-output:
	rm -rf $(OUTDIR)

clean-all: clean clean-docs clean-output

.PHONY: all dirs display test txt txt-all javac javac-all check sanitize sanitize-thread docs clean clean-docs clean-output clean-all