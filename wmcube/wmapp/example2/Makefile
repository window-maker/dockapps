OBJECTS = wmairtrafficcontrol.o wmradar.o

../wmatc: $(OBJECTS) ../libwmapp.a
	$(CXX) $(CFLAGS) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): %.o: %.cc ../*.h wmradar.h
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *\~

.PHONY:	clean

