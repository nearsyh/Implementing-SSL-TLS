all: http webserver

OPTS = -g -Wno-pointer-sign -std=c++1y

http: http.cpp base64.cpp base64.h
		g++ -o http $(OPTS) http.cpp base64.cpp

webserver: webserver.cpp
		g++ -o webserver $(OPTS) webserver.cpp

clean:
	rm -f http
	rm -f webserver
	rm -f core
	rm -f core.*
	rm -rf *.dSYM
