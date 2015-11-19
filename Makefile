CXXFLAGS=-lcurl -lX11 -lXext `allegro-config --libs` -L. -I. -lfmodex

OBJECTS = *.c

alien8: $(OBJECTS)
	$(CXX) $(OBJECTS) -o alien8

clean:
	rm -f $(OBJECTS) alien8
