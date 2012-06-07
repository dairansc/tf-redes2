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
	struct fileHeader arqProp;
	struct fileHeader *teste;
	
	unsigned short porta;     /* Echo server port */
	unsigned int fromSize;           /* In-out of address size for recvfrom() */
	char *servIP;                    /* IP address of server */
	char *nomeArquivo;                /* String to send to echo server */
	char echoBuffer[BUFFERMAX+1];      /* Buffer for receiving echoed string */
	int respStringLen;               /* Length of received response */
	FILE *arquivo;
	char buffer[BUFFERMAX];
	
	if ((argc < 3) || (argc > 4))    // Testa pelo número correto de argumentos
	{
			fprintf(stderr,"Uso: %s <IP do Servidor> <Nome do Arquivo> [<Porta do Servidor>]\nPorta padrão: %d\n", argv[0], PORTAPADRAO);
			exit(1);
	}
	
	servIP = argv[1];       // Primeiro argumento: Endereço IP do Servidor
	nomeArquivo = argv[2];  // Segundo argumento: Nome do Arquivo que será enviado
	
	
	//printf("%s", arqProp.nome);
	if (!argv[3] || !atoi(argv[3])) // Terceiro argumento: Testa se a porta do servidor está válida
	{
	    printf("Atenção: Porta não definida, assumindo porta padrão '%d' para servidor '%s'.\n", PORTAPADRAO, servIP);
	    porta = PORTAPADRAO;
	}
	else
	{
	    porta = atoi(argv[3]);
	}
	
	
	if((arquivo = fopen(nomeArquivo, "rb")) == NULL)
	{
		fprintf(stderr, "Erro ao abrir o arquivo original %s\n", arqProp.nome);
		exit(1);
	}             

    fseek(arquivo, 0, SEEK_END); // posiciona ponteiro no final do arquivo
    arqProp.tamanho = ftell(arquivo);   // pega o valor corrente do ponteiro
    fseek(arquivo, 0, SEEK_SET); // posiciona de volta no início do arquivo
    
    nomeArquivo = nomeBaseArquivo(nomeArquivo);
    arqProp.tamanhoNome = strlen(nomeArquivo);
    
    if( arqProp.tamanhoNome > NOMEARQUIVOMAX)
    {
        fprintf(stderr,"Nome de arquivo excede o tamanho máximo!\n");
        exit(1);
    }
    
    
    sprintf(arqProp.nome, "%s", nomeArquivo);

    printf("Abriu arquivo: %s\nTamanho: %li\n", arqProp.nome, arqProp.tamanho);

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
    
    memcpy(buffer, &(arqProp), sizeof(struct fileHeader));
    
    teste = (struct fileHeader *)buffer;

    printf("%s\n",teste->nome);
	
	
    // Envia cabeçalho do arquivo para o servidor
    if (sendto(sock, buffer, BUFFERMAX, 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != arqProp.tamanhoNome)
      DieWithError("sendto() sent a different number of bytes than expected");
	
	exit(1);
	/* Recv a response */
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, BUFFERMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != arqProp.tamanhoNome)
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
		arqProp.tamanhoNome = sizeof(buffer)*1;
		
		/* Envia conteudo do arquivo para o servidor */
		printf("%s",buffer);
		if (sendto(sock, &buffer, arqProp.tamanhoNome, 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != arqProp.tamanhoNome)
			DieWithError("sendto() sent a different number of bytes than expected");
	
		/* Recv a response */
		fromSize = sizeof(fromAddr);
		if ((respStringLen = recvfrom(sock, echoBuffer, BUFFERMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != arqProp.tamanhoNome)
			DieWithError("recvfrom() failed");
	
		if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
		{
			fprintf(stderr,"Error: received a packet from unknown source.\n");
			exit(1);
		}
		
	}
	
	fclose(arquivo);
	
	/* Envia sinalizacao do fim da tranferencia do arquivo para o servidor */
	strcpy(arqProp.nome,"fim");
	arqProp.tamanhoNome = strlen(arqProp.nome);
	if (sendto(sock, arqProp.nome, arqProp.tamanhoNome, 0, (struct sockaddr *)
					 &servAddr, sizeof(servAddr)) != arqProp.tamanhoNome)
		DieWithError("sendto() sent a different number of bytes than expected");
	
	/* Recv a response */
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, BUFFERMAX, 0, 
		 (struct sockaddr *) &fromAddr, &fromSize)) != arqProp.tamanhoNome)
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
