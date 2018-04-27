CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

run:
	$(CC) $(CFLAGS) project2.c -o project2

clean:
	rm -rf project2
	
