
WORKDIR=.
VPATH = ./src

CC=gcc
CFLGS= -Wall -g -I$(WORKDIR)/inc/ -I/usr/local/include/hiredis/
LIBFLAG = -L$(HOME)/lib

LIBS=-lhiredis

BIN = demo echo test_main data


all:$(BIN)
#keymngclientop.o myipc_shm.o keymng_shmop.o
#keymngclient:keymngclient.o  keymnglog.o 
	#$(CC) $(LIBFLAG) $^ -o $@ 

#keymngserver:keymngserver.o    keymngserverop.o keymnglog.o myipc_shm.o  keymng_shmop.o #key#mng_dbop.o 
#	$(CC) $(LIBFLAG) $^ -o $@ -lpthread -litcastsocket -lmessagereal  -lclntsh  -licdbapi
 
#testdbapi:testdbapi.o  
#	$(CC) $(LIBFLAG) $^ -o $@ -lpthread  -lclntsh  -licdbapi
		
demo:demo.o
	$(CC) $(LIBFLAG) -lfcgi $^ -o $@ 

echo:echo.o keymnglog.o make_log.o echo_op.o redis_op.o 
	$(CC) $(LIBFLAG) -lfcgi $^ -o $@ $(LIBS)

data:data.o data_op.o  make_log.o redis_op.o util_cgi.o cJSON.o
	$(CC) $(LIBFLAG) -lfcgi -lm $^ -o $@ $(LIBS)
	
test_main:test_main.o make_log.o redis_op.o
	$(CC) $(LIBFLAG) -lhiredis $^ -o $@ $(LIBS)

%.o:%.c
	$(CC) $(CFLGS) -c $< -o $@	

.PHONY:clean all Clean

clean:
	-rm -f *.o 
Clean:
	-rm -f *.o $(BIN)
	
	
	




