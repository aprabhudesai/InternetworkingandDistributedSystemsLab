/*
 * Client.cpp
 * File Sender
 *
 * Code written by Abhishek Prabhudesai
 * prabhude@usc.edu
 * designed by Aniket and Abhishek
 */

#include "server.h"

extern struct message* stream_to_message(char *,int);
extern char* message_to_stream(struct message);
extern void sigUSR1Handler(int);
void sigUSR2Handler(int);
extern void error(const char *);
extern void logger_system(struct message *, int);

extern FILE *log_fp;
extern pthread_mutex_t logger_file_lock;

time_t start_time,end_time;

void handle_tcp_control_connection(){

	server_tcp_sock_len = sizeof(int);
	//Setup TCP control connection
	server_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_tcp_socket < 0)
		error("ERROR opening socket");
	bzero((char *) &server_tcp_addr, sizeof(server_tcp_addr));
	server_tcp_addr.sin_family = AF_INET;
	server_tcp_addr.sin_addr.s_addr = INADDR_ANY;
	server_tcp_addr.sin_port = htons(server_tcp_port);
	if((setsockopt(server_tcp_socket,SOL_SOCKET,SO_REUSEADDR,&server_tcp_sock_opt,server_tcp_sock_len)) == -1){
		printf("Error in setting socket opt");
		exit(0);
	}
	if (bind(server_tcp_socket, (struct sockaddr *) &server_tcp_addr,sizeof(server_tcp_addr)) < 0)
		error("ERROR on binding");
	listen(server_tcp_socket,5);
	client_tcp_len = sizeof(client_tcp_addr);
	server_tcp_control_socket = accept(server_tcp_socket,(struct sockaddr *) &client_tcp_addr, &client_tcp_len);
	close(server_tcp_socket);
	return;
}

void create_and_send_ack_message(int seq_num){

	int num_of_bytes;
	char * sender_buffer;
	//Create the message
	struct message msg_to_send;
	msg_to_send.type = ACK;
	msg_to_send.seq_num = seq_num;
	msg_to_send.data = NULL;
	msg_to_send.length = 5;
	sender_buffer = message_to_stream(msg_to_send);
	int n = write(server_tcp_control_socket,sender_buffer,msg_to_send.length);
	free(sender_buffer);
}

void sendFinalAcksAndExit(){
	
	if(!sender_thread_queue.empty()){
		char * ack_array = NULL;
		uint64_t seq_num_for_ack;
		ack_array = (char *)malloc(200000);
		memset(ack_array,'\0',200000);
		seq_num_for_ack = sender_thread_queue.front();
		sender_thread_queue.pop();
		sprintf(ack_array,"lu",seq_num_for_ack);
		while(! sender_thread_queue.empty()){
			seq_num_for_ack = sender_thread_queue.front();
			sender_thread_queue.pop();
			sprintf(ack_array,"%s,%lu",ack_array,seq_num_for_ack);
		}	
		char * sender_buffer;
		//Create the message
		struct message msg_to_send;
		msg_to_send.type = ACK;
		msg_to_send.seq_num = 0;
		msg_to_send.data = ack_array;
		msg_to_send.length = 1 + strlen(ack_array);
		sender_buffer = message_to_stream(msg_to_send);

		int i = 0;
		while(i < 5){
			int n  = sendto(server_udp_socket,sender_buffer,msg_to_send.length,0,(struct sockaddr *) &client_addr,client_len);
			i++;
		}
		printf("\nLast ACK sent\n");
		if(sender_buffer != NULL)
			free(sender_buffer);
	}	
	pthread_exit(NULL);
}

void senderThread(){

	/*
	 * 	1.	create a ACK message to send to the client
	 * 	2.	Use the client_addr struct and send the message to the client
	 */

	char * ack_array = NULL;
	server_actUSR1.sa_handler = sigUSR1Handler;
	sigaction(SIGUSR1, &server_actUSR1, NULL);
	pthread_sigmask(SIG_BLOCK, &server_signalSetUSR1, NULL);

	struct timeval tvtime;

	uint64_t seq_num_for_ack;
	int num_of_acks = 0, i = 0;

	while(1){

		tvtime.tv_sec = 0;
		tvtime.tv_usec = 10000;

		select(0,NULL,NULL,NULL,&tvtime);
		if(time_to_exit == 1){

			fprintf(log_fp,"Sender Exiting\n");
			fflush(log_fp);
			pthread_kill(packet_receiver_thread,SIGINT);
			pthread_exit(NULL);
		}
		//wait for signal from receiver thread
		pthread_mutex_lock(&sender_thread_queue_Lock);
		if(sender_thread_queue.empty()){
			pthread_mutex_unlock(&sender_thread_queue_Lock);
			continue;
		}
		else{
			ack_array = (char *)malloc(20000);
			memset(ack_array,'\0',20000);
			seq_num_for_ack = sender_thread_queue.front();
			sender_thread_queue.pop();
			sprintf(ack_array,"%lu",seq_num_for_ack);
			while(! sender_thread_queue.empty()){
				seq_num_for_ack = sender_thread_queue.front();
				sender_thread_queue.pop();
				sprintf(ack_array,"%s,%lu",ack_array,seq_num_for_ack);
			}
			char * sender_buffer;
			//Create the message
			struct message msg_to_send;
			msg_to_send.identity = ID;
			msg_to_send.type = ACK;
			msg_to_send.seq_num = 0;
			msg_to_send.data = ack_array;
			msg_to_send.length = 2 + strlen(ack_array);
			sender_buffer = message_to_stream(msg_to_send);

			int n  = sendto(server_udp_socket,sender_buffer,msg_to_send.length,0,(struct sockaddr *) &client_addr,client_len);
			if(sender_buffer != NULL)
				free(sender_buffer);
			if(msg_to_send.data != NULL)
				free(msg_to_send.data);
		}
		pthread_mutex_unlock(&sender_thread_queue_Lock);
	}
	pthread_exit(NULL);
}

void fileWriterThread(){

	server_actUSR1.sa_handler = sigUSR1Handler;
	sigaction(SIGUSR1, &server_actUSR1, NULL);
	pthread_sigmask(SIG_BLOCK, &server_signalSetUSR1, NULL);
	recvd_fptr = fopen("data1G.bin","w");
	uint64_t next_seq_no_to_write = 1;
	uint64_t total_bytes_received = 0;
	uint64_t offset_in_file = 0;
	int flag = 0;
	while(1){

		struct message * msg = NULL;
		pthread_mutex_lock(&file_writer_thread_Lock);
		if(file_writer_queue.empty()){
			pthread_cond_wait(&file_writer_thread_CV,&file_writer_thread_Lock);
		}

		msg = file_writer_queue.front();
		file_writer_queue.pop();
		pthread_mutex_unlock(&file_writer_thread_Lock);

		if(msg != NULL && msg->data != NULL){

			if(flag == 0){
				start_time = time(NULL);
				flag = 1;
			}

			//Calculate the offset from sequence number
			offset_in_file = (msg->seq_num - 1) * PAYLOAD_SIZE;
			fseek(recvd_fptr, offset_in_file, SEEK_SET);
			size_t bytes_written = fwrite(msg->data,1,(msg->length - 6),recvd_fptr);
			fflush(recvd_fptr);

			free(msg->data);
			free(msg);

			total_bytes_received = total_bytes_received + bytes_written;

			if(total_bytes_received >= File_Size){

				end_time = time(NULL);
				uint32_t time_diff = end_time - start_time;
				fprintf(log_fp,"%d\n",time_diff);
				fflush(log_fp);
				fclose(log_fp);
				fclose(recvd_fptr);
				pthread_mutex_unlock(&file_writer_thread_Lock);

				break;
			}
		}
	}
	pthread_exit(NULL);
}

void receiverThread(){

	server_actUSR1.sa_handler = sigUSR1Handler;
	sigaction(SIGUSR1, &server_actUSR1, NULL);
	pthread_sigmask(SIG_BLOCK, &server_signalSetUSR1, NULL);

	while(1){
        
		recvdMsg *msg = NULL;
		message * recvd_msg = NULL;
		usleep(50);
		if(receiver_thread_queue.empty()){
			continue;
		}
		msg = receiver_thread_queue.front();
		receiver_thread_queue.pop();
		recvd_msg = stream_to_message(msg->recvd_msg, msg->num_of_bytes);
		if(msg->recvd_msg != NULL)
			free(msg->recvd_msg);
		if(msg != NULL){
			free(msg);
			msg = NULL;
		}
		if(recvd_msg->identity == ID){

			switch(recvd_msg->type){

				case DATA:{

					uint64_t seq_no;
					seq_no = recvd_msg->seq_num;

					pthread_mutex_lock(&sender_thread_queue_Lock);
					sender_thread_queue.push(seq_no);
					pthread_mutex_unlock(&sender_thread_queue_Lock);

					acked_msgs_map_iter = acked_msgs_map.find(seq_no);
					if(acked_msgs_map_iter == acked_msgs_map.end()){

						//packet is received for the first time, Need to write it
						acked_msgs_map.insert(SEQUENCENUMBERMAP::value_type(seq_no,1));
						pthread_mutex_lock(&file_writer_thread_Lock);
						file_writer_queue.push(recvd_msg);

						pthread_cond_signal(&file_writer_thread_CV);
						pthread_mutex_unlock(&file_writer_thread_Lock);
					}
					else{
						free(recvd_msg);
					}
					break;
				}
				case NOTIFY:{
					//signal all the threads to exit
					free(recvd_msg);
					time_to_exit = 1;
					shutdown(server_udp_socket,SHUT_RDWR);
					close(server_udp_socket);
					pthread_exit(NULL);
					break;
				}
				default:{
					printf("\nInvalid message type\n");
					break;
				}
			}
		}
		else{
			free(recvd_msg->data);
			free(recvd_msg);
		}
	}
	pthread_exit(NULL);
}

void initialize_queues(){

	while(!sender_thread_queue.empty()){
		sender_thread_queue.pop();
	}

	while(!receiver_thread_queue.empty()){
		receiver_thread_queue.pop();
	}

	while(!file_writer_queue.empty()){
		file_writer_queue.pop();
	}

	for(file_writer_cache_iter = file_writer_cache.begin(); file_writer_cache_iter != file_writer_cache.end(); file_writer_cache_iter++){
		file_writer_cache.erase(file_writer_cache_iter->first);
	}
}

void packetReceiverThread(){

	int num_of_bytes = 0;
	uint64_t recv_sock_buffer_size = 1000000000;	//1024x1024x1024
	uint64_t total_bytes_recvd = 0;
	server_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	server_udp_sock_len = sizeof(int);

	if (server_udp_socket < 0)
		error("ERROR opening socket");
	bzero((char *) &server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(server_port);

	if((setsockopt(server_udp_socket,SOL_SOCKET,SO_REUSEADDR,&server_sock_opt,server_udp_sock_len)) == -1){
		printf("Error in setting socket opt");
	}
	int status = setsockopt(server_udp_socket,SOL_SOCKET,SO_RCVBUF,&recv_sock_buffer_size, sizeof(uint64_t));
	if(status < 0)
		error("\nError setting socket buffer size");

	if (bind(server_udp_socket, (struct sockaddr *) &server_addr,sizeof(server_addr)) < 0)
		error("ERROR on binding");
	client_len = sizeof(client_addr);
	bzero((char *) &client_addr, sizeof(client_addr));
	while(1){
		char * server_buffer = NULL;
		server_buffer = (char *)malloc(1500);
		memset(server_buffer, '\0', 1500);

		num_of_bytes = recvfrom(server_udp_socket,server_buffer,1500,0,(struct sockaddr *)&client_addr,&client_len);
		if(time_to_exit == 1){
			pthread_exit(NULL);
			break;
		}
		if(num_of_bytes > 6){
			recvdMsg *msg = (struct recvdMsg *) malloc(sizeof(struct recvdMsg));
			msg->num_of_bytes = num_of_bytes;
			msg->recvd_msg = server_buffer;
			receiver_thread_queue.push(msg);
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	
	log_fp = fopen("time_elapsed_file_receiver.log","w+");
	if(log_fp == NULL)
		printf("Could not open log file\n");

	server_port = atoi(argv[1]);
	File_Size = atoi(argv[2]);
        //File_Size = 1048576000;
	initialize_queues();
	//create sender and receiver threads
	(void) pthread_create(&packet_receiver_thread,NULL,(void* (*)(void*))packetReceiverThread,NULL);
	(void) pthread_create(&receiver_thread,NULL,(void* (*)(void*))receiverThread,NULL);
	(void) pthread_create(&sender_thread,NULL,(void* (*)(void*))senderThread,NULL);
	(void) pthread_create(&file_writer_thread,NULL,(void* (*)(void*))fileWriterThread,NULL);

	pthread_join(packet_receiver_thread,NULL);
	pthread_join(receiver_thread,NULL);
	pthread_join(sender_thread,NULL);
	pthread_join(file_writer_thread,NULL);

	return 0;
}
