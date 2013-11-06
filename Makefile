CXX := g++
LDD := g++

FLAGS_RELEASE := -Wall -Wextra -O2 -std=c++0x
FLAGS_DEBUG   := -Wall -Wextra -g -std=c++0x
INCLUDES      := -I/usr/include/mysql -I/usr/include/mysql++
LDD_FLAGS     := -lmysqlpp -lmysqlclient -lopencv_objdetect -lopencv_core \
	         -lopencv_contrib -lopencv_imgproc -lopencv_highgui

TARGET_RELEASE := release/BackR
TARGET_DEBUG   := debug/BackR

HEADERS := TaskManager.h Database.h FaceDetector.h
SRC 	:= BackR.cpp TaskManager.cpp Database.cpp FaceDetector.cpp
OBJ 	:= $(SRC:.cpp=.o)

release: $(TARGET_RELEASE)
$(TARGET_RELEASE): $(SRC) $(HEADERS)
	$(CXX) $(INCLUDES) $(FLAGS_RELEASE) -c $(SRC)
	$(LDD) $(OBJ) -o $(TARGET_RELEASE) $(LDD_FLAGS)

debug: $(TARGET_DEBUG)
$(TARGET_DEBUG): $(SRC) $(HEADERS)
	$(CXX) $(INCLUDES) $(FLAGS_DEBUG) -c $(SRC)
	$(LDD) $(OBJ) -o $(TARGET_DEBUG) $(LDD_FLAGS)

clean:
	rm -f $(OBJ) $(TARGET_RELEASE) $(TARGET_DEBUG)
