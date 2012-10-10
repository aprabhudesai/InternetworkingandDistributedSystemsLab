/*
 * Client.cpp
 * File Sender
 *
 * Code written by Abhishek Prabhudesai
 * prabhude@usc.edu
 * designed by Aniket and Abhishek
 */

#include "common.h"

struct timeval tvtime;

struct recvdMsg{
	char *recvd_msg;
	uint32_t num_of_bytes;
};
//File Pointer
FILE * recvd_fptr;
uint64_t File_Size;

//Exit notification
int time_to_exit = 0;

//Socket variables
struct hostent *server_info;
int server_udp_socket;
socklen_t server_udp_sock_len;
struct sockaddr_in server_addr, client_addr;
socklen_t client_len,client_udp_sock_len;
uint32_t server_port;
bool server_sock_opt;

//TCP
pthread_t tcp_control_thread;
int server_tcp_socket;
struct sockaddr_in server_tcp_addr, client_tcp_addr;
socklen_t server_tcp_sock_len, client_tcp_len;
bool server_tcp_sock_opt;
uint32_t server_tcp_port;
int server_tcp_control_socket;

//send thread window variables
uint32_t swnd[WINDOW_SIZE];
uint32_t swnd_size = WINDOW_SIZE;
uint32_t swnd_current_size;

//list of messages acked
uint32_t ack_msgs_vector[50];

//locks & condition variables
pthread_mutex_t swnd_Lock = PTHREAD_MUTEX_INITIALIZER;

//server threads
pthread_t packet_receiver_thread;
pthread_t worker_thread;
pthread_t sender_thread;
pthread_t receiver_thread;
pthread_t timer_thread;

//sender thread queue
std::queue<uint64_t> sender_thread_queue;
pthread_mutex_t sender_thread_queue_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sender_thread_queue_CV = PTHREAD_COND_INITIALIZER;

//receiver thread queue
std::queue<recvdMsg *> receiver_thread_queue;
pthread_mutex_t receiver_thread_queue_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t receiver_thread_queue_CV = PTHREAD_COND_INITIALIZER;

//signal handling variables
sigset_t server_signalSetUSR1;
struct sigaction server_actUSR1;

sigset_t server_signalSetUSR2;
struct sigaction server_actUSR2;

//File writer thread for I/O operations
pthread_t file_writer_thread;
std::queue<message*> file_writer_queue;
pthread_mutex_t file_writer_thread_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t file_writer_thread_CV = PTHREAD_COND_INITIALIZER;

//Map to store the already acked msgs
typedef std::map<uint64_t,uint32_t> SEQUENCENUMBERMAP;
SEQUENCENUMBERMAP acked_msgs_map;
SEQUENCENUMBERMAP::iterator acked_msgs_map_iter;

typedef std::map<uint64_t,message *> CACHEMAP;
CACHEMAP file_writer_cache;
CACHEMAP::iterator file_writer_cache_iter;

