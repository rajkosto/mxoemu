# platforms:
#  linux-x86-32
#  win32-cygwin
#  win32-mingw
#  solaris9-sparc-64
#  macosx
#  solaris8
PLATFORM =	linux-x86-32

# .h files installed to $(DESTDIR)/$(PREFIX)/include/Sockets
# static lib .a files installed to $(DESTDIR)/$(PREFIX)/lib
PREFIX =	/usr/local

# include paths
INCLUDE =	-I/usr/include/libxml2

include		Makefile.version

# CXX, CFLAGS, LIBS
include		Makefile.Defines.$(PLATFORM)

# uncomment these lines if the library should be in its own namespace
#CFLAGS +=	-DSOCKETS_NAMESPACE=sockets
#CFLAGS +=	-DSOCKETS_NAMESPACE_STR='"sockets"'

# Enable insane amounts of debug output to stdout/stderr:
#CFLAGS +=	-D_DEBUG


CPPFLAGS =	$(CFLAGS) 

PROGS =		Sockets-config

all:		libSockets.a $(PROGS)

libSockets.a:	$(OBJS)
		ar cru $@ $(OBJS)
		ranlib $@

Sockets-config:	Sockets-config.o
		$(CXX) -o $@ $^

clean:
		rm -f *.o *~ slask *.d $(PROGS) *.a */*~

-include	*.d


# everything which follows is www.alhem.net/Sockets website maintenance stuff, please ignore
HTDOCS =	/usr/local/apache/www.alhem.net/htdocs

diff:
		diff -b -B -C 3 /usr/src/Sockets-$(DIFF_VERSION) . | \
			/usr/local/bin/d2html > \
			$(HTDOCS)/Sockets/latest_diff.html

install:	all 
		@mkdir -p $(DESTDIR)/$(PREFIX)/lib
		cp libSockets.a $(DESTDIR)/$(PREFIX)/lib
		@mkdir -p $(DESTDIR)/$(PREFIX)/include/Sockets
		cp -a *.h $(DESTDIR)/$(PREFIX)/include/Sockets
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/SSLSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/HttpsGetSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/HttpsSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/EventSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/PoolSocket.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/SocketThread.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/CircularBuffer.*
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/*Crypt.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/CTcpSocket.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/Min*Socket.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/Min*Handler.h
		@rm -f $(DESTDIR)/$(PREFIX)/include/Sockets/Uid.h
		@mkdir -p $(DESTDIR)/$(PREFIX)/bin
		install Sockets-config $(DESTDIR)/$(PREFIX)/bin

# no binary files, zip will translate lf to cr lf
FILES =		*.h *.cpp Makefile Makefile.* Project/*.ds* \
		INSTALL Changelog README.macosx gpl.txt mkdot.sh \
		Project.net/Sockets/*.vcproj Project.net/Sockets/*.sln \
		Project.net/Test/*.vcproj \
		DevCpp/*.dev tests/Makefile tests/*.cpp \
		OSX.zip

tar:		clean
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		tar czf Sockets-$(VERSION).tar.gz $(FILES)
		/usr/local/bin/tarfix.sh Sockets-$(VERSION)
		cp Sockets-$(VERSION).tar.gz $(HTDOCS)/Sockets
		cp Sockets-$(VERSION).zip $(HTDOCS)/Sockets
		cp tests/sockets_test.cpp /usr/local/apache/www.alhem.net/htdocs/Sockets/
#		$(MAKE) -C ../SocketsLite tar

tmptar:		clean
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		tar czf Sockets-$(VERSION).tar.gz $(FILES)
		/usr/local/bin/tarfix.sh Sockets-$(VERSION)
		cp Sockets-$(VERSION).tar.gz $(HTDOCS)/Sockets-tmp
		cp Sockets-$(VERSION).zip $(HTDOCS)/Sockets-tmp
		cp tests/sockets_test.cpp /usr/local/apache/www.alhem.net/htdocs/Sockets-tmp
		$(MAKE) -C ../SocketsLite tmptar

docs:		clean
		doxygen dox2.cfg
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		rm -f tests/*.cpp
		./mkdot.sh
		rm -rf /usr/local/apache/www.alhem.net/htdocs/Sockets/html
		doxygen doxygen.cfg
		./packdocs.sh Sockets-$(VERSION)-doxygendocs
		cvs up
#		$(MAKE) -C ../SocketsLite docs

tmpdocs:	clean
		rm -f MinderSocket_T.h
		rm -f uuid.h
		rm -f Stdin.*
		rm -f sockets_test.cpp
		rm -f ListenSocketBase.*
		rm -f CircularBuffer.*
		rm -f ICrypt.* NullCrypt.* CTcpSocket.*
		rm -f Min*Socket.* Min*Handler.*
		rm -f Uid.*
		rm -f tests/*.cpp
		./mkdot-tmp.sh
		rm -rf /usr/local/apache/www.alhem.net/htdocs/Sockets-tmp/html
		doxygen doxygen-tmp.cfg
		cvs up
		$(MAKE) -C ../SocketsLite tmpdocs
