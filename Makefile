scheduler: main.o
	gcc main.o -o scheduler

main.o: main.c
	gcc -c main.c

run: scheduler
	./scheduler

clean:
	rm -f *.o scheduler

.PHONY: run clean