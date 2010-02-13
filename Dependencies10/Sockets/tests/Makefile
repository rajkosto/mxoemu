SOCKETS =	..
PLATFORM =	linux-x86-32
LIBS =		-L$(SOCKETS) -lSockets 

INCLUDE =	-I$(SOCKETS)
include		$(SOCKETS)/Makefile.version
include		$(SOCKETS)/Makefile.Defines.$(PLATFORM)

CPPFLAGS =	$(CFLAGS)

#LIBS +=		-lsctp

PROGS =		events resolve sockets_test base64 semtest \
		echoserver stressclient http httpd retry resume \
		http_post x copy crlf https sloppy_http chunked \
		threadstress listener 

all:		$(PROGS)

events:		events.o 
		$(CXX) -o $@ $^ $(LIBS)

resolve:	resolve.o 
		$(CXX) -o $@ $^ $(LIBS)

sockets_test:	sockets_test.o 
		$(CXX) -o $@ $^ $(LIBS)

base64:		base64.o 
		$(CXX) -o $@ $^ $(LIBS)

semtest:	semtest.o 
		$(CXX) -o $@ $^ $(LIBS)

echoserver:	echoserver.o 
		$(CXX) -o $@ $^ $(LIBS)

stressclient:	stressclient.o 
		$(CXX) -o $@ $^ $(LIBS)

http:		http.o 
		$(CXX) -o $@ $^ $(LIBS)

httpd:		httpd.o 
		$(CXX) -o $@ $^ $(LIBS)

retry:		retry.o 
		$(CXX) -o $@ $^ $(LIBS)

resume:		resume.o 
		$(CXX) -o $@ $^ $(LIBS)

http_post:	http_post.o 
		$(CXX) -o $@ $^ $(LIBS)

x:		x.o 
		$(CXX) -o $@ $^ $(LIBS)

copy:		copy.o 
		$(CXX) -o $@ $^ $(LIBS)

crlf:		crlf.o 
		$(CXX) -o $@ $^ $(LIBS)

https:		https.o
		$(CXX) -o $@ $^ $(LIBS)

sloppy_http:	sloppy_http.o
		$(CXX) -o $@ $^ $(LIBS)

chunked:	chunked.o
		$(CXX) -o $@ $^ $(LIBS)

threadstress:	threadstress.o
		$(CXX) -o $@ $^ $(LIBS)

listener:	listener.o
		$(CXX) -o $@ $^ $(LIBS)

clean:
		rm -f *.o *~ slask *.d $(PROGS)

-include	*.d
