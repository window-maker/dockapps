CXX=g++
CXXFLAGS=
TARGET=cpu
	
all: cpu.cc cpuload.cc
	$(CXX) $(CXXFLAGS) -o cpu cpu.cc -lcpumoncc
	$(CXX) $(CXXFLAGS) -o cpuload cpuload.cc -lcpumoncc
	
install: all
	cp cpu /usr/bin
	cp cpuload /usr/bin
	
clean:
	rm -f *~ core *.o cpu cpuload /usr/bin/cpu /usr/bin/cpuload
