CXX      = g++
CXXFLAGS = -Wall -O2
LDLIBS   = -lglut -lGLU -lGL -lm
TARGET   = sample_GL
SRCS     = sample_GL.cpp GLMetaseq.cpp
OBJS     = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
