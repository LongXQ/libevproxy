cc = gcc
flag = -std=gnu99
file = ../event*.c ../util.c

all:
	$cc $flag -o main main.c $file
	$cc $flag -o signal signal.c $file
	$cc $flag -o timeout timeout.c $file
	$cc $flag -o user user.c $file

clean:
	rm -f main signal timeout user

