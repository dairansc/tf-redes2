#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "DieWithError.c"
#include "biblioteca.c"

void DieWithError(char *errorMessage);  /* External error handling function */

/*Se o arquivo existir o valor 1 (true) será retornado. Caso
  contrário a função retornará 0 (false). */
int file_exists(const char *filename)
{
  FILE *arquivo;
  int tam = strlen(filename);

  if ((arquivo = fopen(filename, "r")) != NULL)
  {
	printf("file exist %s %d\n",filename,tam);
    fclose(arquivo);
    return 1;
  }
  return 0;
}

/*Alterar nome do arquivo, em caso de teste em localhost. */
void altera_nome_arquivo (char nome_antigo[BUFFERMAX], int id_nome, char *nome_novo)
{
	int tam = strlen(nome_antigo);
	
	//busca extensao do arquivo
	char *p = strrchr(nome_antigo, '.');
	*p++;

	strncat(nome_novo,nome_antigo,tam-4);
	printf("Novo nome de arquivo %s\n",nome_novo);
    sprintf(nome_novo, "%s%d.", nome_novo, id_nome);
	strcat(nome_novo, p);
	printf("Novo nome de arquivo %s\n",nome_novo);
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    char echoBuffer[BUFFERMAX], nome_arquivo[BUFFERMAX];        /* Buffer for echo string */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    FILE *arq;
    int id_nome , iniciaComunicacao = 0;
    char buffer;

    if (argc != 2)         /* Test for correct number of parameters */
    {
  	    printf("Atenção: Porta não definida, assumindo porta padrão '%d' para servidor.\n", PORTAPADRAO);
	    echoServPort = PORTAPADRAO;
    }
    else 
		echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
  
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
		memset(&echoBuffer, 0, sizeof(echoBuffer));

        if (iniciaComunicacao == 0) 
		{
	        /* Bloqueado até que receba a mensagem do cliente */
	        if ((recvMsgSize = recvfrom(sock, echoBuffer, BUFFERMAX, 0,
	            (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
	            DieWithError("recvfrom() failed");

	        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
	        printf("Received: %s\n", echoBuffer);
            
            strcpy(nome_arquivo, echoBuffer);
            id_nome = 2;
            
            while (file_exists(nome_arquivo) == 1) {
				memset(&nome_arquivo, 0, sizeof(nome_arquivo));
				altera_nome_arquivo(echoBuffer,id_nome,nome_arquivo);
				id_nome++;
            }

			if((arq = fopen(nome_arquivo,"wb")) == NULL)
			{
				printf("Erro ao abrir o arquivo cópia %s\n",echoBuffer);
				strcpy(echoBuffer, "Erro ao abrir o arquivo cópia\n");
		        /* Envia um datagrama de volta para o cliente informando erro*/
		        if (sendto(sock, echoBuffer, recvMsgSize, 0, 
		             (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
		            DieWithError("sendto() sent a different number of bytes than expected");
			}
		  
	        /* Envia um datagrama de volta para o cliente em caso de erro*/
	        if (sendto(sock, echoBuffer, recvMsgSize, 0, 
	             (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
	            DieWithError("sendto() sent a different number of bytes than expected");

	        iniciaComunicacao = 1;
		}
		else
		{
	        /* Bloqueado para receber conteudo do arquivo */
	        if ((recvMsgSize = recvfrom(sock, &buffer, BUFFERMAX, 0,
	            (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
	            DieWithError("recvfrom() failed");
		
            if (strcmp(&buffer,"fim") == 0)
            {
              fclose(arq);
   		        iniciaComunicacao = 0;
   		        strcpy(echoBuffer, nome_arquivo);
            }
            else
              fwrite(&buffer,sizeof(buffer),4, arq);   
		    
	        /* Envia confirmação para cliente */
	        if (sendto(sock, echoBuffer, recvMsgSize, 0, 
	             (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
	            DieWithError("sendto() sent a different number of bytes than expected");
		}
	}
    /* NOT REACHED */
}
