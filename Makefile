DESTDIR = /usr/local
CXXFLAGS = -O2 -march=native -std=c++11 -pthread -lX11
ifeq ($(CHASSIS),laptop)
	CXXFLAGS += -DLAPTOP
endif

mtbar : mtbar.cpp modules.cpp

install : mtbar
	@cp -fv mtbar $(DESTDIR)/bin
	@chmod 755 $(DESTDIR)/bin/mtbar

clean :
	@rm -fv *.o mtbar

.PHONY : install clean
