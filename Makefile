SRC_DIR := src
INC_DIRS := include
OBJ_DIR := bin
BIN_DIR := bin

CPP_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_FILES))

CXX := mpic++
CXX_FLAGS := $(foreach D,$(INC_DIRS),-I$(D))


EXECUTABLE := $(BIN_DIR)/kradzieje

ifeq ($(DEBUG), 1)
	CXXFLAGS += -DDEBUG
endif


all: $(EXECUTABLE)

$(EXECUTABLE): $(CPP_OBJS)
	$(CXX) $(CXX_FLAGS) $(SRC) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(EXECUTABLE)
	mpirun -np 8 $(EXECUTABLE) 8
	#mpirun -H localhost:4,lab-sec-18:4 -np 8 $(EXECUTABLE) 8

debug: $(EXECUTABLE)
	mpirun -np 8 $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(CPP_OBJS)