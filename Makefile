CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LFLAGS = -lrt -lpthread
run:
	$(CC) $(CFLAGS) proj2.c -o proj2 $(LFLAGS)

clean:
	rm -rf proj2
	rm -rf proj2.out
