OBJECTS = wmexample.o window0.o window1.o

../wmexample: $(OBJECTS) ../libwmapp.a
	$(CXX) $(CFLAGS) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): %.o: %.cc ../*.h
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *\~

.PHONY:	clean

