target : example.cpp pkt_decoder.h pkt_decoder.cpp 
	g++ -std=c++11 example.cpp pkt_decoder.cpp  -o example.o

run :
	./example.o

clean :
	rm *.o


