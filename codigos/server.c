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
    int Sock; // descritor de Socket
    struct sockaddr_in servAddr; // Endereço destino
    struct sockaddr_in clntAddr; // Endereço origem
    
    struct fileHeader *arqProp;
    struct datagramHeader dataToClient;
    struct datagramHeader *dataFromClient = (struct datagramHeader*)malloc(sizeof(struct datagramHeader));

    char nome_arquivo[BUFFERMAX];        /* Buffer for echo string */
    int id_nome , iniciaComunicacao = 0, janela;
    long int sequencia;

    if (argc > 2)    // Testa pelo número correto de argumentos
    {
        fprintf(stderr,"Uso: %s [<Porta a ser aberta>]\nPorta padrão: %d\n", argv[0], PORTAPADRAO);
        exit(1);
    }

    printf("porta %d\n",Porta);
    configuraPorta(argv[1]);// Testa se a porta do servidor está válida
    printf("porta %d\n",Porta);
    // Cria socket para enviar/receber datagramas
    if ((Sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // Constrói a estrutura de endereço local
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(Porta);      /* Local port */

    /* Bind to the local address */
    if (bind(Sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    for (;;) /* Run forever */
    {
        if (iniciaComunicacao == 0)
        {
            /* Bloqueado até que receba a mensagem do cliente */

            printf("Antes de chamar o Recebe pacotes iniciaComunicacao == 0.\n");
            printf("antes Handling client %s\n", inet_ntoa(clntAddr.sin_addr));
            dataFromClient = recebePacote(Sock,&clntAddr,0);
            printf("Depois de chamar o Recebe pacotes iniciaComunicacao == 0.\n");
            janela = dataFromClient->janela;
            sequencia = dataFromClient->sequencia;
            arqProp = (struct fileHeader *)dataFromClient->dados;

            printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));
            printf("Received: %s\n", arqProp->nome);


            strcpy(nome_arquivo, arqProp->nome);
            id_nome = 2;

            while (file_exists(nome_arquivo) == 1) {
                memset(&nome_arquivo, 0, sizeof(nome_arquivo));
                altera_nome_arquivo(arqProp->nome,id_nome,nome_arquivo);
                id_nome++;
            }

            dataToClient.flags = 0;
            dataToClient.sequencia = 0;
            dataToClient.idRecebido = 0;
            dataToClient.janela = TAMJANELA;
            memset(dataToClient.dados, 0, TAMDADOSMAX);

            if((Arquivo = fopen(nome_arquivo,"wb")) == NULL)
            {
                dataToClient.flags = FIM;
                strcpy(dataToClient.dados, "Erro ao abrir o arquivo");

                printf("%s cópia %s\n", dataToClient.dados, nome_arquivo);

                /* Envia um datagrama de volta para o cliente informando erro*/
                enviaPacote(Sock,clntAddr,dataToClient);
                //exit(1);
            }
            else
            {
                dataToClient.flags = SYN;

                enviaPacote(Sock,clntAddr,dataToClient);

                iniciaComunicacao = 1;
            }
        }
        else if(iniciaComunicacao == 1)
        {

            Reenviar = 1;
            while(Reenviar)
            {
                memset(BufferJanela, 0, sizeof(BufferJanela));
                Reenviar = 0;
                for(ContJanela=0; ContJanela<janela; ContJanela++)
                {
                    //dataToServer.flags = ACK;
                    //dataToServer.sequencia++;
                    printf("Antes de chamar o Recebe pacotes iniciaComunicacao == 1.\n");
                    dataFromClient = recebePacote(Sock,&clntAddr,1);
                    printf("Depois de chamar o Recebe pacotes iniciaComunicacao == 1.\n");
                    if((dataFromClient->flags & FIM) == FIM)
                    {
                        fclose(Arquivo);
                        iniciaComunicacao = 0;
                    }
                    else if(!Reenviar)
                    {
                        memcpy((BufferJanela+(TAMDADOSMAX*ContJanela)), dataFromClient->dados, TAMDADOSMAX);

                        Reenviar = (sequencia != dataFromClient->sequencia - 1) ? 1 : 0;
                    }
                    sequencia = dataFromClient->sequencia;
                }

                if(!Reenviar)
                {
                    janela = dataFromClient->janela;
                    fwrite(&BufferJanela, (TAMDADOSMAX * janela), 1, Arquivo);
                }
                else
                {
                    dataToClient.flags = NACK;
                    enviaPacote(Sock,clntAddr,dataToClient);
                }
            }
        }
        else
        {
            exit(1);
        }
    }
    /* NOT REACHED */
}
