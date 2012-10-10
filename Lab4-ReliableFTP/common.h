/*
 * Client.cpp
 * File Sender
 *
 * Code written by Abhishek Prabhudesai
 * prabhude@usc.edu
 * designed by Aniket and Abhishek
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include "limits.h"
#include <string>
#include <iostream>
#include <strings.h>
#include <string.h>
#include <map>
#include <vector>
#include <queue>
#include <errno.h>
#include <sched.h>

#define ERROR -1
#define WINDOW_SIZE 16500 //17200
#define PAYLOAD_SIZE 1450
#define ID 37

#define ACK 	0x01
#define DATA 	0x06
#define NOTIFY  0x03

#define SENT 0x01
#define RECV 0x02
#define RESENT 0x03

//message structure
struct message_info{

	timeval timestamp;
	uint64_t seq_num;
	uint64_t file_offset;
};

//data info

typedef struct data_info_t{
	uint32_t num_bytes;
	char *data;
}data_info;

/*
 * 	message structure used between sender and receiver
 * 	type:	0x01 - DATA
 * 			0x02 - ACK
 * 			0x03 - NOTIFY
 */
struct message{
	uint8_t identity;
	uint8_t type;
	uint32_t length;
	uint64_t seq_num;
	char *data;
};

int setup_udp_socket(int,struct hostent *);

extern FILE *log_fp;
extern pthread_mutex_t logger_file_lock;

using namespace std;
