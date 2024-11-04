CXX = g++
CXXFLAGS = -std=c++11 `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4` -lcfitsio

all: fit_thumbnailer

fit_thumbnailer: fit_thumbnailer.cpp
	$(CXX) $(CXXFLAGS) -o fit_thumbnailer fit_thumbnailer.cpp $(LDFLAGS) -g

clean:
	rm -f fit_thumbnailer

