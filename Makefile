all:
	$(CC) src/main.c -o buzzertool

install:
	cp -v buzzertool /usr/sbin

clean:
	rm buzzertool
