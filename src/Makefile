CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lminisat

TARGET = roger-apk
SRCS = roger-apk.cpp cli-apk.cpp list-apk.cpp sync-apk.cpp install-apk.cpp delete-apk.cpp upgrade-apk.cpp bomb-apk.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
