/* Example code for starting
 * 2 threads and passing messages  
 * of different data types between
 * them over a single message queue.
 *
 * All code provided is as is 
 * and not completely tested
 *
 * Author: Aadil Rizvi
 * Date: 14/1/2016
 *
 * Website: arembedded.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <util/util.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>

#define MY_MQ_NAME "/my_mq"
#define DATA_STR_LEN 30

pthread_t thread1;
pthread_t thread2;

static struct mq_attr my_mq_attr;
static mqd_t my_mq;

enum {
    DATA_TYPE_U32,
    DATA_TYPE_I32,
    DATA_TYPE_U16,
    DATA_TYPE_I16,
    DATA_TYPE_U8,
    DATA_TYPE_F32,
    DATA_TYPE_STR
};

typedef union my_data_s {
    unsigned int data_u32;
    int data_i32;
    unsigned short int data_u16;
    short int data_i16;
    unsigned char data_u8;
    float data_f32;
    unsigned char array[DATA_STR_LEN];
} my_data_t;

typedef struct my_mq_msg_s {
    int type;
    my_data_t data;
} my_mq_msg_t;

void thread1_main(void);
void thread2_main(void);

void sig_handler(int signum) {
    if (signum != SIGINT) {
        printf("Received invalid signum = %d in sig_handler()\n", signum);
        ASSERT(signum == SIGINT);
    }

    printf("Received SIGINT. Exiting Application\n");

    pthread_cancel(thread1);
    pthread_cancel(thread2);

    mq_close(my_mq);
    mq_unlink(MY_MQ_NAME);

    exit(0);
}

int main(void) {
    pthread_attr_t attr;
    int status;
 
    signal(SIGINT, sig_handler);

    my_mq_attr.mq_maxmsg = 10;
    my_mq_attr.mq_msgsize = sizeof(my_mq_msg_t);

    my_mq = mq_open(MY_MQ_NAME, \
                    O_CREAT | O_RDWR | O_NONBLOCK, \
                    0666, \
                    &my_mq_attr);

    ASSERT(my_mq != -1);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024*1024);
   
    printf("Creating thread1\n");
    status = pthread_create(&thread1, &attr, (void*)&thread1_main, NULL);
    if (status != 0) {
        printf("Failed to create thread1 with status = %d\n", status);
        ASSERT(status == 0);
    }    

    printf("Creating thread2\n");
    status = pthread_create(&thread2, &attr, (void*)&thread2_main, NULL);
    if (status != 0) {
        printf("Failed to create thread2 with status = %d\n", status);
        ASSERT(status == 0);
    }    

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    sig_handler(SIGINT);
    
    return 0;
}

void thread1_main(void) {
    unsigned int exec_period_usecs;
    int status;
    my_mq_msg_t send_msg;
    int cnt=0;

    exec_period_usecs = 1000000; /*in micro-seconds*/

    printf("Thread 1 started. Execution period = %d uSecs\n",\
                                           exec_period_usecs);
    while(1) {
        switch (cnt) {

            case 0:
                /* Send a U32 type value on the MQ */
                send_msg.type = DATA_TYPE_U32;
                send_msg.data.data_u32 = 99999999;
                break;
            case 1:
                /* Send an I32 type value on the MQ */
                send_msg.type = DATA_TYPE_I32;
                send_msg.data.data_i32 = -1324287;
                break;
            case 2:
                /* Send a U16 type value on the MQ */
                send_msg.type = DATA_TYPE_U16;
                send_msg.data.data_u16 = 100;
                break;
            case 3:
                /* Send a I16 type value on the MQ */
                send_msg.type = DATA_TYPE_I16;
                send_msg.data.data_i16 = -45;
                break;
            case 4:
                /* Send a U8 type value on the MQ */
                send_msg.type = DATA_TYPE_U8;
                send_msg.data.data_u8 = 5;
                break;
            case 5:
                /* Send a F32 type value on the MQ */
                send_msg.type = DATA_TYPE_F32;
                send_msg.data.data_f32 = 3.1415;
                break;
            case 6:
                /* Send a STR type value on the MQ */
                send_msg.type = DATA_TYPE_STR;
                snprintf(send_msg.data.array,\
                         DATA_STR_LEN,\
                         "This is a test str\n");
                break;
            default:
                printf("Invalid counter value in sending thread\n");
                break;
        }

        status = mq_send(my_mq, (const char*)&send_msg, sizeof(send_msg), 1);
        ASSERT(status != -1);

        cnt += 1;

        if (cnt > 6) {
           cnt = 0;
        }
        usleep(exec_period_usecs);
    }
}


void thread2_main(void) {
    unsigned int exec_period_usecs;
    int status;
    my_mq_msg_t recv_msg;

    exec_period_usecs = 10000; /*in micro-seconds*/

    printf("Thread 2 started. Execution period = %d uSecs\n",\
                                           exec_period_usecs);

    while(1) {
        status = mq_receive(my_mq, (char*)&recv_msg, \
                            sizeof(recv_msg), NULL);

        if (status == sizeof(recv_msg)) {

            switch(recv_msg.type) {
                case DATA_TYPE_U32:
                    printf("Received data type:   U32,   value = %u\n",\
                            recv_msg.data.data_u32); 
                    break;         
                case DATA_TYPE_I32:
                    printf("Received data type:   I32,   value = %d\n",\
                            recv_msg.data.data_i32); 
                    break;         
                case DATA_TYPE_U16:
                    printf("Received data type:   U16,   value = %hu\n",\
                            recv_msg.data.data_u16); 
                    break;         
                case DATA_TYPE_I16:
                    printf("Received data type:   I16,   value = %hd\n",\
                            recv_msg.data.data_i16); 
                    break;         
                case DATA_TYPE_U8:
                    printf("Received data type:   U8,    value = %hhu\n",\
                            recv_msg.data.data_u8); 
                    break;         
                case DATA_TYPE_F32:
                    printf("Received data type:   F32,   value = %f\n",\
                            recv_msg.data.data_f32); 
                    break;         
                case DATA_TYPE_STR:
                    printf("Received data type:   STR,   value = %s\n",\
                            recv_msg.data.array); 
                    break;         
                default:
                    printf("Received invalid data type\n");
                    ASSERT(0);
                    break;         
            }
        }
 
        usleep(exec_period_usecs);
    }
}

