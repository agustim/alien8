CXX=gcc
CXXFLAGS=-lcurl -lX11 -lXext -L/usr/local/lib -lalleg -L. -I.


alien8:
	$(CXX) *.c $(CXXFLAGS) -o alien8

clean:
	rm -f $(OBJECTS) alien8
