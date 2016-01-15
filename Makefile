INC_DIR=/home/arembedded/unity_mq_example

u_mq: unity_mq_example.c $(INC_DIR)/util/util.c
	gcc unity_mq_example.c $(INC_DIR)/util/util.c -I$(INC_DIR) -lpthread -lrt -o unity_mq_example.o

all: u_mq

clean:
	rm -rf *.o	
