#include <cstdio> 
#include <cstdlib>
#include <cstring>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>

static const int port = 5000; 
static const int packet_size = 512;
static const int count = 256;

static unsigned char data[packet_size * count];
static bool received[count];
static unsigned char buffer[packet_size + 2];
  
// Driver code 
int main() { 
    int sockfd; 
    sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        fprintf(stderr, "ERROR: Socket creation failed"); 
        exit(1); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 

	//bind socket to port
	if( bind(sockfd , (sockaddr*)&servaddr, sizeof(servaddr) ) == -1)
	{
		fprintf(stderr, "Cannot bind socket!\n");
        exit(1); 
	}

    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    unsigned short number = 0;

    while(number < count){

        size_t len;      

        size_t n = recv(sockfd, (void *)buffer, packet_size + 2,  
                MSG_WAITALL);
        if(n == -1)
            break;
	if(n < packet_size + 2)
		continue;
        number = *((unsigned short*) buffer);
        received[number] = true;
        for(int i = 0; i < packet_size; ++i){
            data[number * packet_size + i] = buffer[i + 2];
        } 
        
    }
//    fprintf(stderr,"%i\n", (int)number);
    close(sockfd);
//    return 0;

    for(int i = 0; i < count; ++i){
        if(!received[i]){
            fprintf(stderr, "%i\n", i);
        }
        write(1 , (const void*) (data + i * packet_size), (size_t)packet_size);
    } 
    return 0; 
} 

