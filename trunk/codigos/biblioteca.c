// comando rápido para compilação e execução do servidor
// gcc -fno-stack-protector -o server server.c && gcc -fno-stack-protector -o client client.c && ./server 10000

#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket() and bind()
#include <arpa/inet.h>  // for sockaddr_in and inet_ntoa()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include "DieWithError.c"

struct datagramHeader
{
    short flags;            // indicadores de conexão
    long int sequencia;
    long int idRecebido;    // Acknowlegement
    int janela;             // tamanho da janela
    //char dados[TAMDADOSMAX];
};

#define BUFFERMAX 1458
#define PORTAPADRAO 10000

#define TAMDADOSMAX (BUFFERMAX-sizeof(struct datagramHeader))
#define NOMEARQUIVOMAX (TAMDADOSMAX-16)
//#define NOMEARQUIVOMAX (TAMDADOSMAX-sizeof(long int))
#define TAMJANELA 10

// indicadores de conexão
#define SYN   0x01
#define FIM   0x02
#define ACK   0x04
#define NACK  0x08

struct fileHeader
{
    long int tamanho;
    char nome[NOMEARQUIVOMAX];
};

unsigned short Porta;     // Porta aberta de conexão
FILE *arqOrigem, *arqDestino;              // Ponteiro para o arquivo
short Reenviar;             // Indica se janela precisa ser reenviada
char Buffer[BUFFERMAX];
char BufferJanela[TAMDADOSMAX*TAMJANELA];
char TempBufferJanela[TAMDADOSMAX];
int ContJanela;

void configuraPorta(char *parametro)
{
    if (!parametro || !atoi(parametro))
    {
        printf("Atenção: Porta não definida, assumindo porta padrão '%d'.\n", PORTAPADRAO);
        Porta = PORTAPADRAO;
    }
    else
    {
        Porta = atoi(parametro);
    }
}

void enviaPacote(int Sock, struct sockaddr_in Addr, char *buffer, int quantidade)
{
    //char buffer[BUFFERMAX];

    //printf("%d\n\n", pacote.janela);

    //memset(buffer, 0, sizeof(buffer));
    //memcpy(buffer, &(pacote), sizeof(pacote));

    printf("envia pacotes.\n");

    if (sendto(Sock, buffer, quantidade, 0, (struct sockaddr *) &Addr, sizeof(Addr)) != quantidade)
        DieWithError("Error: sendto() sent a different number of bytes than expected");

    printf("enviou pacotes sem erro.\n");
}

int recebePacote(int Sock, struct sockaddr_in *fromAddr, struct sockaddr_in sourceAddr, short verificarOrigem, char *buffer)
{
    //char buffer[BUFFERMAX];
    int quantidade;
    struct sockaddr_in tempFromAddr;
    unsigned int fromSize = sizeof(tempFromAddr);
    
    //struct datagramHeader* recPacote = (struct datagramHeader*)malloc(sizeof(struct datagramHeader));

    memset(buffer, 0, sizeof(buffer));

    // Recebe resposta
    //memset(buffer, 0, sizeof(buffer));
    if ((quantidade = recvfrom(Sock, buffer, BUFFERMAX, 0, (struct sockaddr *) &tempFromAddr, &fromSize)) > BUFFERMAX)
        DieWithError("recvfrom() failed");
    
    *fromAddr = tempFromAddr;
/*    
    if (verificarOrigem && sourceAddr.sin_addr.s_addr != tempFromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }
*/
    //memcpy(recPacote, buffer, sizeof(buffer));

    return quantidade;

}

char *nomeBaseArquivo(char *caminho)
{
    char *base = strrchr(caminho, '/');
    return base ? base+1 : caminho;
}
