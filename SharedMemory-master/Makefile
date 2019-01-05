# List of files

#CFLAGS += -Wall	

all: checker coordinator

checker: checker.c
	$(CC) $(CFLAGS) $< -o$@

coordinator: coordinator.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f checker coordinator