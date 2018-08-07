CC=g++

CFLAGS= -pthread -O3

app:main

$(app) : $(app).cpp
	$(CC) $^ -o $@ $(CFLAGS) 

clean:
	rm $(app)
