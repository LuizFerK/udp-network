#include <semaphore.h>
#include <stdio.h>
#include "receiver.h"
#include "packet_handler.h"

#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<unistd.h> //close()
#include<arpa/inet.h>
#include<sys/socket.h>

#define LOG_PREFIX "[Receiver]"
#define BUFLEN 512

static void die(char *s) {
  perror(s);
  exit(1);
}

void* receiver(void* arg) {
  Config* config = (Config*)arg;

  struct sockaddr_in si_me, si_other;
     
  int s, recv_len;
  socklen_t slen = sizeof(si_other);
  char buf[BUFLEN];
    
  //create a UDP socket
  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("socket");
  }
    
  // zero out the structure
  memset((char *) &si_me, 0, sizeof(si_me));
    
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(config->router.port);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
  //bind socket to port
  if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
    die("bind");
  }
    
  //keep listening for data
  while (1) {
    printf("%s Waiting for messages...\n", LOG_PREFIX);
    fflush(stdout);
    //receive a reply and print it
    //clear the buffer by filling null, it might have previously received data
    memset(buf,'\0', BUFLEN);

    //try to receive some data, this is a blocking call
    if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
      die("recvfrom()");
    }
      
    //print details of the client/peer and the data received
    printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
    printf("Data: %s\n" , buf);
      
    //now reply the client with the same data
    // if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
    //   die("sendto()");
    // }
  }

  close(s);

  return NULL;
}