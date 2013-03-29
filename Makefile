#amcsh Makefile
CFLAGS = -Wall
LDFLAGS = -lutil -lm

all:amcsh amcshd
amcshd: amcshd.o otp.o hmac.o sha1.o functions.o
amcsh: functions.o

clean:
	-rm *.o amcsh amcshd

.PHONY: clean

sources = amcsh.c  amcshd.c  hmac.c  otp.c  sha1.c functions.c
include $(sources:.c=.d)
%.d: %.c
	set -e;rm -rf $@;\
	$(CC) -MM $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$
