CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++17 -g 
LDFLAGS =  

SRC = main.cc kdNode.cc dump.cc
OBJ = $(SRC:.cc=.o)
EXEC = k-nn

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ) $(LBLIBS) -pthread

clean:
	rm -rf $(OBJ) $(EXEC)