/*

Compile with: gcc -g -c server.c -o server.o

Create executable with: gcc -g server.o -o server.exe -lpthread

Then run server from the terminal with: ./server.exe 

clear ; gcc -g -c server.c -o server.o ; gcc -g server.o -o server.exe -lpthread ; ./server.exe

*/


#include <stdio.h>
#include <string.h>	
#include <stdlib.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>	
#include <pthread.h>

void *client_connection_establish(void *);
void *client_message_echo(void *);
void user_has_joined(void *);

char *message, client_message[2000], client_name[50];
int msgRcvd = 0;
int clients_nr = 0;
char *connected_clients[5];
int  connected_clients_sockets[5] = {0,0,0,0,0};


int main(int argc , char *argv[])
{
   int socket_desc , new_socket , c , *new_sock;
   struct sockaddr_in server , client;
   
   //Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
     
    /* 
    Enter the server's IP address and port number 
    We will connect to the server to local host:
    IP Address: 127.0.0.1
    Port number: 8888
    */
    server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );

    //Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
	   puts("bind failed");
	   return 1;
	}
	puts("bind done");

	listen(socket_desc , 5); //The server will accept a maximum of 5 clients in the chat room at any given time

    //Accept and incoming connection
	puts("Processing incomming connection requests(max 5)...");
	c = sizeof(struct sockaddr_in);

    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted - Client will now choose a username");  //client used connect() to establish a connection to the server

        /*
        Confirm to the client that he has managed to connect to the chat room
        the waiting for the client to choose of a username will be done in the client-serving worker thread to avoid blocking the server here
		*/
        //message = "Welcome to the chat room! Please choose a username!"; 
		//write(new_socket , message , strlen(message));

        pthread_t client_connection_thread, client_echo_thread; 
        new_sock = malloc(1);
        *new_sock = new_socket;
  
        // Create thread that handles connections with the clients - sending welcome message and telling them to choose username
        pthread_create(&client_connection_thread, NULL,  client_connection_establish, (void*) new_sock); 
	    //pthread_create(&client_echo_thread,       NULL,  client_message_echo,   (void*) new_sock);

        //Now join the thread , so that we dont terminate before each communication thread has finished its work
		pthread_join(client_connection_thread , NULL);
       
    }
    
    //We do not go into the while(1) which represents the server confirming a connection with the client if accept() API goes wrong
    if (new_socket<0)
	{
		perror("accept failed");
		return 1;
	}

    return 0;

}

/* This function handles the communication with EACH client in a different 
   concurrent client_connection_thread
*/
void *client_connection_establish(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    pthread_t client_echo_thread; 
    
    
    user_has_joined(socket_desc); // notify everyone in the chat room that a new client has joined
    pthread_create(&client_echo_thread,       NULL,  client_message_echo,   (void*) socket_desc);
    //pthread_join(client_echo_thread, NULL);
	
	
	return 0;
}

void *client_message_echo(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int i;
    char client_reply[2000];
	
    while(1)
    {
     bzero(client_reply, sizeof(client_reply));
     recv(sock, client_reply , 2000 , 0);
     // Send the text to all other connected clients
     for(i=0; i<clients_nr; i++)
     {
      if(connected_clients_sockets[i] != sock)
      {
       write(connected_clients_sockets[i] , client_reply , strlen(client_reply)); 
      }     
     }
    }
    //If client closes connection - free socket and update the arrays
	//free(socket_desc);
}


void user_has_joined(void *socket_desc)
{
  //Get the socket descriptor and cast it to appropiate data type - int in this case
  int sock = *(int*)socket_desc;
  int i;
	
  //Send some messages to the client
  message = "Please chose a username:\n";
  write(sock , message , strlen(message));
  
  //Get the client name back so we can notify the other clients that someone has joined
  recv(sock , client_name , 100 , 0); 

  //update the array of clients names and socket descriptors(so we can interact with them)
  connected_clients[clients_nr] = client_name;
  connected_clients_sockets[clients_nr] = sock;
  
  char str1[50];
  strcpy(str1, connected_clients[clients_nr]);
  char str2[] = " has joined!";
  strcat(str1, str2);

  //A new client has joined the chat room - increment the number of connected clients
  clients_nr++;

  //Notify all the connected clients that a new client has joined the chat room
  for(i=0; i<clients_nr; i++)
  {
     if(connected_clients_sockets[i] != sock)
     {
       write(connected_clients_sockets[i] , str1 , strlen(str1)); 
     }
  }

  
}