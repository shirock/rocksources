LIBGUVCVIEW_SRCDIR=guvcview
build:
	make -C guvcview/src build

dist:
	./make-package --dest=. debian

install: build
	echo "Not implemeted"

check: build
	cd tests; ./test-session.py

uninstall:
	echo "Not implemeted"

clean:
	make -C guvcview/src clean
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
