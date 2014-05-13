all: fishsticks
fishsticks: fishsticks.c
	gcc -Wall -g -o fishsticks fishsticks.c
clean:
	rm -f fishsticks