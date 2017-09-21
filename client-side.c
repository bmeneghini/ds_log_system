/*
* TCP LogSystem 
* Created by Bernardo Meneghini, Lucas Rocha and Thiago Alexandre in 09/21/2017.
* This program sends local logs from a client to the server based of some given expression.
*/

#include "supportl.h"

char buffer[BUFFER_SIZE];
char *ip_address;
char *filename;
char *expression;

int connectSocket(char *ip_address){
    int sockfd; // socket file descriptor
    struct hostent *server;
    struct sockaddr_in serv_addr;
    struct ifreq ifr;
    char msg[25];

    // Opening socket to start connection:
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    // resolving host:
    server = gethostbyname(ip_address);
    if (server == NULL)
        error("ERROR, no such host\n");
    bzero((char *) &serv_addr, sizeof(serv_addr)); // cleans serv_addr

    // configuring and connecting socket:
    serv_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr,
           (char *)&serv_addr.sin_addr.s_addr,
           server->h_length);
    serv_addr.sin_port = htons(PORT_NUMBER);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    strcpy(buffer,"Testing connection...");
    sendMessage(sockfd, buffer);
    receiveMessage(sockfd, msg);
	
    return sockfd;
}

void parse_options(int argc, char *argv[])
{
    if (argc < 1) {
       fprintf(stderr,"Usage %s -i ip_address\n", argv[0]);
       puts("Optional flags:\n-f file_name\n-e expression\n");
       exit(0);
    }
    puts("----- initial information -----");
    for(int i = 1; i < argc; ++i) {
        if(strcmp("-i",argv[i]) == 0) {
            printf("ip: %s\n",argv[++i]);
            ip_address = argv[i];
        }
        if(strcmp("-f",argv[i]) == 0) {
            printf("filename: %s\n", argv[++i]);
            filename = argv[i];
        }
        if(strcmp("-e",argv[i]) == 0) {
            printf("expression: %s\n", argv[++i]);
            expression = argv[i];
        }
    }
    puts("-------------------------------");
}


int main(int argc, char *argv[])
{
	long SIZE;
    int sockfd,  n;
    struct Frame frame = {};

    parse_options(argc, argv);

    sockfd = connectSocket(ip_address);

    // Connection stabilished. Sending message requesting frame size:
    strcpy(buffer,"Requesting frame size...");
    sendMessage(sockfd, buffer);

    // Reading message from server:
    receiveMessage(sockfd, buffer);
    printf("Message size: %s\n", buffer);
    SIZE = strtol (buffer,NULL,10);

    // with socket size negotiated, send filename:
    strcpy(buffer,filename);
    createFrame(&frame, buffer);
    sendFrame(&frame, sockfd, frameSize(&frame));

    // And receiving confirmation:
    receiveFrame(&frame, sockfd);
    strcpy(buffer, frame.data);
    printf("Return from server: %s (%d bytes)\n", buffer, (int) frameSize(&frame));

	// Creating the file
	char comand[] = "cat /var/log/*.log | grep ";
	strcat(comand, expression);
	strcat(comand, " > ");
    strcat(comand, filename);
	system(comand);

	// Opening message file to read bytes and send them:
    FILE* msgFile;
    msgFile = fopen(filename,"rb");
    int msgFd = fileno(msgFile);
    if( !msgFile | msgFd < 0)
    	error("File doesn't exist");

    // Finally, Sending files:
    int i = 0;
    int c;
    bzero(buffer,BUFFER_SIZE);
    size_t nbytes = fread(buffer, sizeof(char), MAX_DATA_SIZE-1, msgFile);
    while (nbytes > 0){
        createFrame(&frame, buffer);
        printf("\nsending message of %li bytes to server...\n", strlen(frame.data));
        sendFrame(&frame, sockfd, frameSize(&frame));
        bzero(buffer,BUFFER_SIZE);
        nbytes = fread(buffer, sizeof(char), MAX_DATA_SIZE, msgFile);
    }
    printf("File sent.\n");
    fclose(msgFile);
    close(sockfd);
    return 0;
}












