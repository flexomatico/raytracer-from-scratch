CC = g++
LANG_STD = -std=c++17
COMPILER_FLAGS = -Wall -Wfatal-errors
INCLUDE_PATH = -I"./libs/"
SRC_FILES = ./src/*.cpp \
			./src/Raytracer/*.cpp
LINKER_FLAGS = -lSDL2
OBJ_NAME = raytracer


build:
	${CC} ${COMPILER_FLAGS} ${LANG_STD} ${INCLUDE_PATH} ${SRC_FILES} ${LINKER_FLAGS} -o ${OBJ_NAME}

run:
	./${OBJ_NAME}

clean:
	rm ${OBJ_NAME}