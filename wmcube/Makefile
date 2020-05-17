#
# wmCube 0.99-pre1
#
# To make wmcube compile on another OS change the following things:
#
# CXXFLAGS -D: FREEBSD/NETBSD/SOLARIS/OPENBSD/DARWIN
# CPUMONCC_OS: freebsd/netbsd/solaris/openbsd/darwin (you will also have
# to make sure that the makefile in the cpumoncc/$(CPUMONCC_OS) directory
# is working.
# LFLAGS: Modify/Add any necessary libraries here.
#
#
CXX= g++
CXXFLAGS=-O2 -DLINUX
CPUMONCC_OS=linux
WMCUBE_LIBS=-Lwmapp -lwmapp -Lcpumoncc/$(CPUMONCC_OS) -lcpumoncc
LFLAGS=$(WMCUBE_LIBS) -L/usr/X11R6/lib -lXpm -lXext -lX11 
OBJECTS = WmcObject.o
TARGET = wmcube

$(TARGET): wmcube.cc cpumoncc/$(CPUMONCC_OS)/libcpumoncc.a wmapp/libwmapp.a $(OBJECTS)
	@@echo "###############################################################"
	@@echo "#"
	@@echo "# I am now building the actual wmCube application."
	@@echo "#" 
	@@echo "###############################################################"
	$(CXX) $(CXXFLAGS) $(OBJECTS) $< -o $@ $(LFLAGS)

cpumoncc/$(CPUMONCC_OS)/libcpumoncc.a:
	@@echo "###############################################################"
	@@echo "# I am now building the cpumoncc library in 'cpumoncc/$(CPUMONCC_OS)'."
	@@echo "# Please modify/verify the makefile in that directory before"
	@@echo "# reporting an error as a bug."
	@@echo "###############################################################"
	cd cpumoncc/$(CPUMONCC_OS); make; cd ../..
	
wmapp/libwmapp.a:
	@@echo "###############################################################"
	@@echo "# I am now building the wmapp library in 'wmapp'"
	@@echo "# Please modify/verify the makefile in that directory before"
	@@echo "# reporting an error as a bug."
	@@echo "###############################################################"
	cd wmapp; make; cd ..

install: $(TARGET)
	cp -Rf wmc /usr/share/wmcube
	cp wmcube /usr/local/bin
	chmod 755 /usr/local/bin/wmcube
        chown root:root /usr/local/bin/wmcube

clean:
	rm -f *.o *\~ $(TARGET)
	cd wmapp; make clean; cd ..
	cd cpumoncc/$(CPUMONCC_OS); make clean; cd ../..
	
install_clean:
	rm -fr /usr/share/wmcube
	rm /usr/local/bin/wmcube
