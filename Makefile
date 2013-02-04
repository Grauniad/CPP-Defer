test.out: defer.h test.cpp
	g++ -o test.out -std=c++0x test.cpp
