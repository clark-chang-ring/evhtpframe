GCC = g++
INCLUDE = -I../util -I../common -I/apps/local/include/
CFLAG = -c -O3 -Wall -g
LIB= -L/apps/local/lib/ -L /usr/lib/sasl2/ 
LIBS=-levent -levhtp -levent_openssl -ljson -lcurl -lhiredis -lsasl2
OBJS = Daemon.o ServerFrame.o hist_size.o  ../util/my_func.o ../util/my_redis.o
hist.sized: $(OBJS)
	$(GCC) -o $@   $(OBJS) $(INCLUDE) $(LIB) $(LIBS) -g
	-rm $(OBJS) >/dev/null 2>&1
.cpp.o: 
	$(GCC) -o $@  $(CFLAG) $< $(INCLUDE) $(LIB)
.PHONY:clean
clean:
	-rm userd $(OBJS) >/dev/null 2>&1
