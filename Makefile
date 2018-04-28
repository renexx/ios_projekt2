CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LFLAGS = -lrt -lpthread
run:
	$(CC) $(CFLAGS) project2.c -o project2 $(LFLAGS)

clean:
	rm -rf project2
	rm -rf proj2.out
