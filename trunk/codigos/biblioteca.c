// comando rápido para compilação e execução do servidor
// gcc -fno-stack-protector -o server server.c && gcc -fno-stack-protector -o client client.c && ./server 10000

#include <stdio.h>      // para printf() e fprintf()
#include <sys/socket.h> // para socket() e bind()
#include <arpa/inet.h>  // para sockaddr_in e inet_ntoa()
#include <stdlib.h>     // para atoi() e exit()
#include <string.h>     // para memset()
#include <unistd.h>     // para close()
#include "DieWithError.c"

struct datagramHeader
{
    short flags;            // indicadores de conexão
    long int sequencia;		// identifica número de sequência do pacote
    long int idRecebido;    // Acknowlegement
    int janela;             // tamanho da janela
};

#define BUFFERMAX 1458
#define PORTAPADRAO 10000

#define TAMDADOSMAX (BUFFERMAX-sizeof(struct datagramHeader))
#define NOMEARQUIVOMAX (TAMDADOSMAX-16)
// não utilizar valores maiores que 50 pois do contrário o programa 
//fica instável devido ao consumo de memória
#define TAMJANELA 50 

// indicadores de conexão
#define SYN   0x01   //solicitação de conexão
#define FIM   0x02   //sinalização de fim de conexão
#define ACK   0x04   //sinal de confirmação de recebimento de pacotes
#define NACK  0x08   //sinal informando falha no recebimento de pacotes

//Estrutura de dados e que contém informações sobre 
//o arquivo a ser transferido
struct fileHeader
{
    long int tamanho;
    char nome[NOMEARQUIVOMAX];
};

unsigned short Porta;     					// Porta aberta de conexão
FILE *arqOrigem, *arqDestino;           	// Ponteiro para o arquivo
short Reenviar;             				// Indica se janela precisa ser reenviada
char Buffer[BUFFERMAX];						// Buffer temporário que guarda o campo dados do pacote
char BufferJanela[TAMDADOSMAX*TAMJANELA];   // Buffer que armazena a janela
int ContJanela;								// Contador para controlar numero de janelas enviadas

//Rotina auxiliar para consistir a porta informada na execução da aplicação
void configuraPorta(char *parametro)
{
    if (!parametro || !atoi(parametro))
    {
        printf("Atenção: Porta do servidor não definida, assumindo porta padrão '%d'.\n", PORTAPADRAO);
        Porta = PORTAPADRAO;
    }
    else
    {
        Porta = atoi(parametro);
    }
}

//Rotina que encapusula o envio de dados para consistir se quantidade enviada
//é igual a pretendida
void enviaPacote(int Sock, struct sockaddr_in Addr, char *buffer, int quantidade)
{
    if (sendto(Sock, buffer, quantidade, 0, (struct sockaddr *) &Addr, sizeof(Addr)) != quantidade)
        DieWithError("Error: sendto() sent a different number of bytes than expected");
}

//Rotina que encapusula o recebimento de dados para consistir 
//recebeu a quantidade de dados sinalizada no envio
int recebePacote(int Sock, struct sockaddr_in *fromAddr, struct sockaddr_in sourceAddr, char *buffer)
{
    int quantidade;
    struct sockaddr_in tempFromAddr;
    unsigned int fromSize = sizeof(tempFromAddr);
	//printf("Aguardando recebimento...\n");
    memset(buffer, 0, sizeof(buffer));

    // Recebe resposta
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

    return quantidade;

}
//Função auxiliar que retira informação caminho do arquivo
//Retorna somente o nome do arquivo com extensão
char *nomeBaseArquivo(char *caminho)
{
    char *base = strrchr(caminho, '/');
    return base ? base+1 : caminho;
}
