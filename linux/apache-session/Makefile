BINDIR=/usr/local/bin

build: dist

dist:
	./make-package debian --dest=.

install:
	cp -f apache-session.sh $(BINDIR)/apache-session
	chmod a+x $(BINDIR)/apache-session
	mkdir -p /etc/apache2/session
	cp -f apache*.conf /etc/apache2/session/

uninstall:
	rm -f $(BINDIR)/apache-session
	rm -rf /etc/apache2/session

clean:
	rm -rf dist *.deb

