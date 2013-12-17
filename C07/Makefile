all: reader writer rw test_writer

rw: rw.c produce.o
	gcc $< produce.o -o $@

reader: reader.c
	gcc $< -o $@

writer: writer.c 
	gcc $< -o $@


test_writer: test_writer.c
	gcc $< -o $@

produce.o: produce.c
	gcc -c $< -o $@

clean:
	rm -f reader writer test_writer rw produce.o
