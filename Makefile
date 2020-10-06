TARGET=libzpu.a

#CFLAGS+=-O2 -I./
CFLAGS+=-ggdb -I./

all:	$(TARGET)

clean:
	$(RM) *.o
	$(RM) $(TARGET)

$(TARGET):	zpu.o zpu_mem.o zpu_syscall.o
	ar rcs $(TARGET)  zpu.o zpu_mem.o zpu_syscall.o

zpu.o: \
	zpu.c zpu.h 

zpu_mem.o: \
	zpu_mem.c zpu_mem.h 

zpu_syscall.o: \
	zpu_syscall.c zpu_syscall.h 

install: $(TARGET)
	cp $(TARGET) /usr/local/lib/
	cp zpu.h zpu_mem.h zpu_syscall.h /usr/local/include/
	
