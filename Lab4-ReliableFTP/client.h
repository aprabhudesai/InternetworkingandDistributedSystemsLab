/*
 * 	Client.cpp
 * 	Client Header Module
 *
 * 	Code written by Aniket Zamwar
 * 	zamwar@usc.edu
 * 	designed by Aniket and Abhishek
 */

#include "common.h"

//Command line parameters
float link_delay;
float link_speed;
float loss_rate;
char file_path[256];
FILE *fp;

//Socket variables
struct hostent *server_info = NULL;
int client_udp_socket;
socklen_t server_udp_sock_len, client_udp_sock_len;
struct sockaddr_in server_addr, client_addr;

//send thread queue variables

std::vector<uint64_t> udp_control_queue;
std::vector<uint64_t>::iterator udp_control_queue_itr;
int notify_status = 0;
pthread_mutex_t udp_control_queue_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t udp_control_queue_CV = PTHREAD_COND_INITIALIZER;

uint32_t cwnd[WINDOW_SIZE];

typedef std::map<uint32_t, struct message_info*> MSGMAPTYPE;
MSGMAPTYPE MsgMap;
MSGMAPTYPE::iterator MsgMapIter;

typedef std::map<uint32_t, data_info*> DATAMAPTYPE;
DATAMAPTYPE DataMap;
DATAMAPTYPE::iterator DataMapIterSend;
DATAMAPTYPE::iterator DataMapIterReSend;
DATAMAPTYPE::iterator DataMapIterRecv;

uint32_t data_map_size = 0, data_map_high_limit = 40000, data_map_low_limit = 30000;

uint32_t cwnd_size;
uint32_t cwnd_current_count;
uint64_t curr_seq_num = 0;

std::vector<uint64_t> queue_of_sent_seq;
std::vector<uint64_t>::iterator queue_of_sent_seq_itr;

pthread_mutex_t MsgMap_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t MsgMap_CV = PTHREAD_COND_INITIALIZER;

pthread_t timer_thread, sender_thread, receiver_thread, resender_thread, file_reader_thread, attacker_thread;
