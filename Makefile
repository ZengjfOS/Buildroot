all:
	$(CC) src/main.c -o buzzertool

install:
	cp -v buzzertool $(DESTDIR)/usr/sbin

clean:
	rm buzzertool
