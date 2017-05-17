SRCDIR=guvcview/src

build:
	make -C $(SRCDIR) -f Makefile-lib build
	rm $(SRCDIR)/*.o
	make -C $(SRCDIR) build

dist:
	./make-package --dest=. debian

install: build
	echo "Not implemeted"

check: build
	cd tests; ./test-session.py

uninstall:
	echo "Not implemeted"

clean:
	make -C $(SRCDIR) clean
	rm -f $(SRCDIR)/uvc-cli
	rm -f $(SRCDIR)/libguvcview
	rm -f dbus/*.pyc
	rm -rf dist export
	rm -f *.deb

help:
	@echo
	@echo "Usage:"
	@echo "    clean    : remove artifacts."
	@echo "    build    : build program."
	@echo "    dist     : pack a debian package."
	@echo "    check    : unit test."
	@echo
