################################################################################

CC		= gcc
CXX		= g++
CPPFLAGS	= -I/usr/include/libxml2 -I/usr/include/x86_64-linux-gnu/qt5
CFLAGS		= -march=native -O3 -funroll-loops -fpic -fstack-protector-all \
		  -Wall -Wextra -Werror -Wno-deprecated-declarations
CXXFLAGS	= -march=native -O3 -funroll-loops -fpic -fstack-protector-all \
		  -Wall -Wextra -Werror
REASON		= @if [ -f $@ ]; then echo "[$@: $?]"; else echo "[$@]"; fi
LDFLAGS		= -lnetpbm -lcurl -L/usr/lib/x86_64-linux-gnu -lxml2 \
		  -lQt5DBus -lQt5Core -lQt5Xml -lcrypto -lX11 -lXtst

.c.o:
	$(REASON)
	$(COMPILE.c) $< $(OUTPUT_OPTION)
 
.cc.o:
	$(REASON)
	$(COMPILE.cc) $< $(OUTPUT_OPTION)

all: ldcd

################################################################################

ld_demo: ld_demo.o loupedeck.o image.o
	$(REASON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

pong: pong.o loupedeck.o image.o
	$(REASON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

fb_demo: fb_demo.o fritzbox.o
	$(REASON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

ppm2raw: ppm2raw.o image.o
	$(REASON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

ldcd: control.o loupedeck.o image.o fritzbox.o input.o querydbus.o
	$(REASON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

################################################################################

tags:
	ctags --format=2 -o $@ *.h *.cc

deps depend: *.h *.c
	$(REASON)
	$(CC) -MM $(CPPFLAGS) *.c > deps

clean:
	$(RM) *.o

distclean: clean
	$(RM) core deps tags *~

-include deps

################################################################################
