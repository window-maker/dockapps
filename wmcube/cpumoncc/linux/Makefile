######################################################
#
# To compile cpumoncc for a different OS/architecture you
# will need to modify the variablese below to match your system.
#
# For system wide installation type (as root):
#
# make install
#
# This will install the static library in $(LIBINSTALL) and
# the header files in $(HINSTALL)/cpumoncc.
#
#####################################################

CXX=g++
CXXFLAGS=
HINSTALL=/usr/include
LIBINSTALL=/usr/lib

#####################################################

TARGET=libcpumoncc.a

$(TARGET): CpuMonitor.o
	ar rcs $(TARGET) $^

CpuMonitor.o: copy_base

copy_base:
	cp ../base/BaseCpuMonitor.h .
        
install: $(TARGET)
	mkdir -p $(HINSTALL)/cpumoncc
	cp *.h $(HINSTALL)/cpumoncc
	install -m 644 $(TARGET) $(LIBINSTALL)

clean:
	rm -f *~ core *.o cpu $(TARGET) BaseCpuMonitor.h
#	rm -fr $(HINSTALL)/cpumoncc
#	rm -f $(LIBINSTALL)/$(TARGET)
