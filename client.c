/*

Compile with: gcc -g -c client.c -o client.o

Create executable with: gcc -g client.o -o client.exe

To connect with the server use: ./client.exe

clear ; gcc -g -c client.c -o client.o ; gcc -g client.o -o client.exe -lpthread ; ./client.exe
*/

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX 2000

int socket_desc, n;
struct sockaddr_in server;
char *message , server_reply[2000];
char buff[MAX]; //used to get the message from the command line
char my_name[300];
char my_name_aux[300];
char final_msg[2500];
char cursor_msg[300];

int display_name = 0;


void* receive_message(void*);
void* send_message(void*);

int main(int argc , char *argv[])
{
  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
	 printf("Could not create socket");
  }

  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons( 8888 );

  //Connect to the chat server
  if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
	 puts("connect error");
	 return 1;
  }

  //Receive confirmation of connection from the server
  if( recv(socket_desc, server_reply , 2000 , 0) < 0)
  {
	 puts("Confirmation not received - exiting"); // message from the server did not reach us
   return 1;
  }
  puts(server_reply);

  /* Client chooses his name here */
  bzero(buff, sizeof(buff));
  n = 0;
  while ((buff[n++] = getchar()) != '\n');
  write(socket_desc , buff , (n-1));
  memcpy(my_name, buff, n-1 ); 

  pthread_t receive_message_thread, send_message_thread;
  pthread_create(&receive_message_thread, NULL, receive_message, NULL);
  pthread_create(&send_message_thread, NULL, send_message, NULL);
  
  //pthread_join( receive_message_thread , NULL);
  //pthread_join( send_message_thread , NULL);

  /* Client sends and receive data from the server*/
  while(1)
  {
    if(display_name == 1)
    {
     strcpy(cursor_msg, my_name); 
     strncat(cursor_msg, ": ", 2);
     puts(cursor_msg);
     bzero(cursor_msg, sizeof(cursor_msg));
     display_name = 0;
    }

  }

  return 0;

}

/* This function executes concurrently and blocks on recv 
   Its job is to get messages from the server and display them 
*/
void* receive_message(void* arg)
{
  while(1)
  {
   bzero(server_reply, sizeof(server_reply));
   recv(socket_desc, server_reply , 2000 , 0);
   puts(server_reply);

   display_name = 1;
  }
}

/* This function executes concurrently and blocks on send
   Its job is to send messages to the server
*/
void* send_message(void* arg)
{
 while(1)
 {
  bzero(buff, sizeof(buff));
  bzero(my_name_aux, sizeof(my_name_aux));
  strcpy(my_name_aux, my_name); 
  
  strncat(my_name_aux, ": ", 2);
  n = 0;
  while ((buff[n++] = getchar()) != '\n');

  //
  strcat(my_name_aux, buff);
  strncat(my_name_aux, "\n", 2); 
  strcpy(final_msg, my_name_aux);
  //  

  write(socket_desc , final_msg , sizeof(final_msg));  

  display_name = 1;
 }
}
