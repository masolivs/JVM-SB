CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
TARGET = build\leitor.exe
OUTDIR = output

SRCS = src\main.cpp \
       src\class_reader.cpp \
       src\constant_pool.cpp \
       src\attributes.cpp \
       src\displayer.cpp \
       src\opcodes.cpp

OBJS = $(patsubst src\%.cpp, build\%.o, $(SRCS))

all: build $(TARGET)

build:
	if not exist build mkdir build

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(IFLAGS) $^ -o $@

build\%.o: src\%.cpp
	$(CXX) $(CFLAGS) $(IFLAGS) -c $< -o $@

# Exibir um .class
display: all
	$(TARGET) exemplos\$(CLASS).class

# Rodar todos os testes
test: all
	test.bat

# Gerar .txt de um arquivo
txt: all
	if not exist $(OUTDIR) mkdir $(OUTDIR)
	$(TARGET) exemplos\$(CLASS).class > $(OUTDIR)\$(CLASS).txt
	@echo Gerado: $(OUTDIR)\$(CLASS).txt

# Gerar .txt de todos os exemplos
txt-all: all
	txt-all.bat

# Compilar .java para .class
TARGET_VER ?= 8

javac:
	javac -source $(TARGET_VER) -target $(TARGET_VER) -d exemplos $(JAVA)

javac-all:
	javac -source $(TARGET_VER) -target $(TARGET_VER) -d exemplos exemplos\*.java

# Análise estática
check:
	cppcheck --enable=all --language=c++ \
	  --suppress=missingIncludeSystem \
	  -I include src\

# Análise dinâmica

# sanitize: funciona no Windows (MSYS2) com UBSan apenas
# sanitize-thread: funciona apenas no Linux/WSL
sanitize: CFLAGS += -fsanitize=undefined -fno-omit-frame-pointer
sanitize: clean all

sanitize-thread: CFLAGS += -fsanitize=thread -fno-omit-frame-pointer
sanitize-thread: clean all

# Documentação
docs:
	doxygen Doxyfile

# Limpeza
clean:
	if exist build rmdir /s /q build

clean-docs:
	if exist docs\html rmdir /s /q docs\html

clean-output:
	if exist $(OUTDIR) rmdir /s /q $(OUTDIR)

clean-all: clean clean-docs clean-output

.PHONY: all display test txt txt-all javac javac-all check sanitize sanitize-thread docs clean clean-docs clean-output clean-all