# Compiler
CXX = g++

# Compiler flags
# -g for debugging, -Wall for all warnings
CXXFLAGS = -g -Wall

# Libraries to link against
# lGL: OpenGL, lGLEW: GLEW, lglut: GLUT
LIBS = -lGLEW -lGL -lglut -lsfml-audio -lsfml-system

# Target executable name
TARGET = arjuna_game

# Source files
SRCS = main.cpp

# Default rule: build the executable
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# Rule to clean up the directory
clean:
	rm -f $(TARGET)

# Rule to run the game
run: all
	./$(TARGET)
