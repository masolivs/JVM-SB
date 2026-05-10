CXX    = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
IFLAGS = -I include
SRC    = $(shell find src -name '*.cpp')
OBJ    = $(patsubst src/%.cpp, build/%.o, $(SRC))
TARGET = build/jvm

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(IFLAGS) $^ -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(IFLAGS) -c $< -o $@

check:
	cppcheck --enable=all --language=c++ \
	  --suppress=missingIncludeSystem \
	  -I include src/

sanitize: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: clean all

docs:
	doxygen Doxyfile

display: all
	./$(TARGET) -d $(CLASS)

run: all
	./$(TARGET) tests/class/$(CLASS).class

clean:
	rm -rf build/

clean-docs:
	rm -rf docs/html/

clean-all: clean clean-docs