#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "DieWithError.c"
#include "biblioteca.c"

void DieWithError(char *errorMessage);  /* External error handling function */

int main(int argc, char *argv[])
{
	int sock;                        /* Socket descriptor */
	struct sockaddr_in servAddr; /* Echo server address */
	struct sockaddr_in fromAddr;     /* Source address of echo */
	unsigned short porta;     /* Echo server port */
	unsigned int fromSize;           /* In-out of address size for recvfrom() */
	char *servIP;                    /* IP address of server */
	char *nomeArquivo;                /* String to send to echo server */
	//int porta;
	char echoBuffer[BUFFERMAX+1];      /* Buffer for receiving echoed string */
	int nomeArquivoLen;               /* Length of string to echo */
	int respStringLen;               /* Length of received response */
	unsigned long fileSize;
	FILE *arquivo;
	char buffer[BUFFERMAX];
	
	if ((argc < 3) || (argc > 4))    // Testa pelo número correto de argumentos
	{
			fprintf(stderr,"Uso: %s <IP do Servidor> <Nome do Arquivo> [<Porta do Servidor>]\nPorta padrão: %d\n", argv[0], PORTAPADRAO);
			exit(1);
	}
	
	servIP = argv[1];       // Primeiro argumento: Endereço IP do Servidor
	nomeArquivo = argv[2];  // Segundo argumento: Nome do Arquivo que será enviado
	
	
	if (!argv[3] || !atoi(argv[3])) // Terceiro argumento: Testa se a porta do servidor está válida
	{
	    printf("Atenção: Porta não definida, assumindo porta padrão '%d' para servidor '%s'.\n", PORTAPADRAO, servIP);
	    porta = PORTAPADRAO;
	}
	else
	{
	    porta = atoi(argv[3]);
	}
	
	//printf("Arquivo: %s\nPorta: %d\n", nomeArquivo, porta);
	
	//if ((nomeArquivoLen = strlen(nomeArquivo)) > BUFFERMAX)  /* Check input length */
	//		DieWithError("Nome de arquivo excede o tamanho máximo!\n");
	
	if((arquivo = fopen(nomeArquivo, "rb")) == NULL)
	{
		fprintf(stderr, "Erro ao abrir o arquivo original %s\n", nomeArquivo);
		exit(1);
	}             

    fseek(arquivo, 0, SEEK_END); // posiciona ponteiro no final do arquivo
    fileSize = ftell(arquivo);   // pega o valor corrente do ponteiro
    fseek(arquivo, 0, SEEK_SET); // posiciona de volta no início do arquivo
    
    nomeArquivo = nomeBaseArquivo(nomeArquivo);
    
//    exit(1);
//	if (argc == 4)
//			porta = atoi(argv[3]);  /* Use given port, if any */
//	else
//			porta = 7;  /* 7 is the well-known port for the echo service */

    nomeArquivoLen = strlen(nomeArquivo);

    if( (sizeof(unsigned long)+sizeof(int)+nomeArquivoLen) > BUFFERMAX)
    {
        fprintf(stderr,"Nome de arquivo excede o tamanho máximo!\n");
        exit(1);
    }

    printf("Abriu arquivo: %s\nTamanho: %d\n", nomeArquivo, fileSize);

	// Cria um socket datagrama/UDP
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");
	
	
	// Constrói a estrutura do endereço do servidor
	memset(&servAddr, 0, sizeof(servAddr));     // Zera a estrutura
	servAddr.sin_family = AF_INET;               // Internet addr family
	servAddr.sin_addr.s_addr = inet_addr(servIP);// Endereço de IP do servidor
	servAddr.sin_port   = htons(porta);          // Porta do servidor
	
	// Limpa dados do buffer
    memset(buffer, 0, sizeof(buffer));
    
    
    //memcpy(buffer, fileSize, sizeof(unsigned long));
    //memcpy(buffer+sizeof(unsigned long), (char)nomeArquivoLen, sizeof(int));
    //memcpy(buffer+sizeof(unsigned long)+sizeof(int), nomeArquivo, nomeArquivoLen);
    
    //memcpy(buffer, fileSize, sizeof(unsigned long));
    memcpy(buffer+sizeof(unsigned long)+1, nomeArquivo, nomeArquivoLen);
    char *nome;

    printf("%s\n",buffer+sizeof(unsigned long)+1);
	exit(1);
    // Envia cabeçalho do arquivo para o servidor
    if (sendto(sock, nomeArquivo, nomeArquivoLen, 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != nomeArquivoLen)
      DieWithError("sendto() sent a different number of bytes than expected");
	
	/* Recv a response */
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, BUFFERMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != nomeArquivoLen)
		DieWithError("recvfrom() failed");
	
	if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
			fprintf(stderr,"Error: received a packet from unknown source.\n");
			exit(1);
	}
	
	/* Transfere conteudo do arquivo para servidor */
	while(!feof(arquivo)) 
	{
		fread(&buffer, sizeof(buffer),1,arquivo);
		nomeArquivoLen = sizeof(buffer)*1;
		
		/* Envia conteudo do arquivo para o servidor */
		printf("%s",buffer);
		if (sendto(sock, &buffer, nomeArquivoLen, 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != nomeArquivoLen)
			DieWithError("sendto() sent a different number of bytes than expected");
	
		/* Recv a response */
		fromSize = sizeof(fromAddr);
		if ((respStringLen = recvfrom(sock, echoBuffer, BUFFERMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != nomeArquivoLen)
			DieWithError("recvfrom() failed");
	
		if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
		{
			fprintf(stderr,"Error: received a packet from unknown source.\n");
			exit(1);
		}
		
	}
	
	fclose(arquivo);
	
	/* Envia sinalizacao do fim da tranferencia do arquivo para o servidor */
	nomeArquivo = "fim";
	nomeArquivoLen = strlen(nomeArquivo);
	if (sendto(sock, nomeArquivo, nomeArquivoLen, 0, (struct sockaddr *)
					 &servAddr, sizeof(servAddr)) != nomeArquivoLen)
		DieWithError("sendto() sent a different number of bytes than expected");
	
	/* Recv a response */
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, BUFFERMAX, 0, 
		 (struct sockaddr *) &fromAddr, &fromSize)) != nomeArquivoLen)
		DieWithError("recvfrom() failed");
	
	if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
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
