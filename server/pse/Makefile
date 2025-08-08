include ../../config.mk

OBJS = parser.tab.o scanner.o ast.o module.o

all: $(OBJS)

# 分别编译每个目标文件
ast.o: ast.c
	$(CC) $(CFLAGS) -c -o $@ $<

module.o: module.c
	$(CC) $(CFLAGS) -c -o $@ $<

parser.tab.o: scanner.c

scanner.c: scanner.l
	flex --header-file=scanner.h --outfile=scanner.c scanner.l

parser.tab.c: parser.y
	bison -Wno-deprecated -vd parser.y

.PHONY: clean
clean:
	rm -f *.o parser.tab* scanner.[ch] *.output