CXX := g++
LDD := g++

FLAGS_RELEASE := -Wall -Wextra -O2 -std=c++0x
FLAGS_DEBUG   := -Wall -Wextra -g -std=c++0x
INCLUDES      := -I/usr/include/mysql -I/usr/include/mysql++
LDD_FLAGS     := -lmysqlpp -lmysqlclient

TARGET_RELEASE := release/BackR
TARGET_DEBUG   := debug/BackR

SRC := BackR.cpp TaskManager.cpp Database.cpp
OBJ := $(SRC:.cpp=.o)

release: $(TARGET_RELEASE)
$(TARGET_RELEASE): $(SRC)
	$(CXX) $(INCLUDES) $(FLAGS_RELEASE) -c $(SRC)
	$(LDD) $(OBJ) -o $(TARGET_RELEASE) $(LDD_FLAGS)

debug: $(TARGET_DEBUG)
$(TARGET_DEBUG): $(SRC)
	$(CXX) $(INCLUDES) $(FLAGS_DEBUG) -c $(SRC)
	$(LDD) $(OBJ) -o $(TARGET_DEBUG) $(LDD_FLAGS)

clean:
	rm -f $(OBJ) $(TARGET_RELEASE) $(TARGET_DEBUG)
