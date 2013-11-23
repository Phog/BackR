CXX := g++
LDD := g++

FLAGS_RELEASE := -Wall -Wextra -O2 -std=c++0x -DWEB_ROOT=$(WEB_ROOT)
FLAGS_DEBUG   := -Wall -Wextra -g -std=c++0x -DWEB_ROOT=$(WEB_ROOT)
INCLUDES      := -I/usr/include/mysql -I/usr/include/mysql++
LDD_FLAGS     := -lmysqlpp -lmysqlclient -lopencv_objdetect -lopencv_core \
	         -lopencv_contrib -lopencv_imgproc -lopencv_highgui

TARGET_RELEASE := release/BackR
TARGET_DEBUG   := debug/BackR
TARGET_TESTS   := test/BackR

HEADERS   := TaskManager.h Database.h FaceDetector.h FaceTrainer.h
SRC    	  := FaceTrainer.cpp Database.cpp
BACKR     := BackR.cpp TaskManager.cpp FaceDetector.cpp 
TESTS     := test.cpp
OBJ       := $(SRC:.cpp=.o)
BACKR_OBJ := $(BACKR:.cpp=.o)
TESTS_OBJ := $(TESTS:.cpp=.o)

release: $(TARGET_RELEASE)
$(TARGET_RELEASE): $(SRC) $(HEADERS) $(BACKR)
	$(CXX) $(INCLUDES) $(FLAGS_RELEASE) -c $(SRC) $(BACKR)
	$(LDD) $(OBJ) $(BACKR_OBJ) -o $(TARGET_RELEASE) $(LDD_FLAGS)

debug: $(TARGET_DEBUG)
$(TARGET_DEBUG): $(SRC) $(HEADERS) $(BACKR)
	$(CXX) $(INCLUDES) $(FLAGS_DEBUG) -c $(SRC) $(BACKR)
	$(LDD) $(OBJ) $(BACKR_OBJ) -o $(TARGET_DEBUG) $(LDD_FLAGS)

tests: $(TARGET_TESTS)
$(TARGET_TESTS): $(SRC) $(HEADERS) $(TESTS)
	$(CXX) $(INCLUDES) $(FLAGS_DEBUG) -c $(SRC) $(TESTS)
	$(LDD) $(OBJ) $(TESTS_OBJ) -o $(TARGET_TESTS) $(LDD_FLAGS)

clean:
	rm -f $(OBJ) $(BACKR_OBJ) $(TESTS_OBJ) \
	$(TARGET_RELEASE) $(TARGET_DEBUG) $(TARGET_TESTS)
