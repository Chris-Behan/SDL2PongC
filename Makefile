# #OBJS specifies which files to compile as part of the project
# OBJS = src/main.c

# #CC specifies which compiler we're using
# CC = clang

# #COMPILER_FLAGS specifies the additional compilation options we're using
# COMPILER_FLAGS = -g



# #OBJ_NAME specifies the name of our exectuable
# OBJ_NAME = main

# #This is the target that compiles our executable
# all : $(OBJS)
# 	$(CC) -g $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
#OBJS specifies which files to compile as part of the project
OBJS = src/main2.c

#CC specifies which compiler we're using
CC = gcc

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -w -g

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = main2

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)