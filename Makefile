CXX=g++
CXXFLAGS = -g -Wall
TARGET = main
RANDOMFILE = 64k-random.txt


all: $(TARGET) $(RANDOMFILE)

$(TARGET): $(TARGET).cc
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(TARGET).cc

$(RANDOMFILE):
	cat /dev/urandom | tr -cd '[:alnum:]' | head -c 64k > 64k-random.txt

.PHONY: clean
clean:
	rm $(TARGET) $(RANDOMFILE)