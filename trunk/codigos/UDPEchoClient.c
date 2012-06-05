#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "DieWithError.c"

#define ECHOMAX 255     /* Longest string to echo */

void DieWithError(char *errorMessage);  /* External error handling function */

int main(int argc, char *argv[])
{
	int sock;                        /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
	struct sockaddr_in fromAddr;     /* Source address of echo */
	unsigned short echoServPort;     /* Echo server port */
	unsigned int fromSize;           /* In-out of address size for recvfrom() */
	char *servIP;                    /* IP address of server */
	char *echoString;                /* String to send to echo server */
	char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
	int echoStringLen;               /* Length of string to echo */
	int respStringLen;               /* Length of received response */
	FILE *arq;
	char buffer;
	
	if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
	{
			fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
			exit(1);
	}
	
	servIP = argv[1];           /* First arg: server IP address (dotted quad) */
	echoString = argv[2];    /* Second arg: string to echo */
	
	printf("Arquivo %s\n",echoString);
	if ((echoStringLen = strlen(echoString)) > ECHOMAX)  /* Check input length */
			DieWithError("Nome de arquivo excede o tamanho máximo!\n");
	
	if((arq = fopen(echoString,"rb")) == NULL)
	{
		printf("Erro ao abrir o arquivo original %s\n",echoString);
		exit(1);
	}             
	
	printf("Abriu arquivo %s\n",echoString);
	
	if (argc == 4)
			echoServPort = atoi(argv[3]);  /* Use given port, if any */
	else
			echoServPort = 7;  /* 7 is the well-known port for the echo service */
	
	/* Create a datagram/UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");
	
	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
	echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
	echoServAddr.sin_port   = htons(echoServPort);     /* Server port */
	
	/* Envia nome do arquivo para o servidor */
	if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
		DieWithError("sendto() sent a different number of bytes than expected");
	
	/* Recv a response */
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
		DieWithError("recvfrom() failed");
	
	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
			fprintf(stderr,"Error: received a packet from unknown source.\n");
			exit(1);
	}
	
	/* Transfere conteudo do arquivo para servidor */
	while(!feof(arq)) 
	{
		fread(&buffer, sizeof(buffer),4,arq);
		echoStringLen = sizeof(buffer)*4;
		
		/* Envia conteudo do arquivo para o servidor */
		if (sendto(sock, &buffer, echoStringLen, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
			DieWithError("sendto() sent a different number of bytes than expected");
	
		/* Recv a response */
		fromSize = sizeof(fromAddr);
		if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
			DieWithError("recvfrom() failed");
	
		if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
		{
			fprintf(stderr,"Error: received a packet from unknown source.\n");
			exit(1);
		}
		
	}
	
	fclose(arq);
	
	/* Envia sinalizacao do fim da tranferencia do arquivo para o servidor */
	echoString = "fim";
	echoStringLen = strlen(echoString);
	if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)
					 &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
		DieWithError("sendto() sent a different number of bytes than expected");
	
	/* Recv a response */
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, 
		 (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
		DieWithError("recvfrom() failed");
	
	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}
	
	/* null-terminate the received data */
	echoBuffer[respStringLen] = '\0';
	printf("Received: %s\n", echoBuffer);    /* Print the echoed arg */
	
	close(sock);
	exit(0);

}
