CXX = g++
AR = ar rcs

COMPILE_FLAGS = -std=c++11 -Wall -Wextra -O2 -I /usr/local/include
SOURCES = nRF24L01.cpp
HEADERS = nRF24L01.h
OUTPUT = nRF24_Freedom

all: $(SOURCES) $(HEADERS)
	$(CXX) $(COMPILE_FLAGS) -c $(SOURCES) -o $(OUTPUT).o
	$(AR) lib$(OUTPUT).a $(OUTPUT).o

install: lib$(OUTPUT).a
	install -m 0755 lib$(OUTPUT).a /usr/local/lib

uninstall:
	rm -f /usr/local/lib/lib$(OUTPUT).a