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
    int Sock; // descritor de Socket
    struct sockaddr_in servAddr; // Endereço destino
    struct sockaddr_in fromAddr; // Endereço origem

    struct fileHeader arqProp;
    struct datagramHeader dataToServer;
    struct datagramHeader *dataFromServer = (struct datagramHeader*)malloc(sizeof(struct datagramHeader));

    char *servIP;                    // Endereço IP do servidor
    char *nomeArquivo;                // String com o nome do arquivo


    if ((argc < 3) || (argc > 4))    // Testa pelo número correto de argumentos
    {
        fprintf(stderr,"Uso: %s <IP do Servidor> <Nome do Arquivo> [<Porta do Servidor>]\nPorta padrão: %d\n", argv[0], PORTAPADRAO);
        exit(1);
    }

    servIP = argv[1];       // Primeiro argumento: Endereço IP jdo Servidor
    nomeArquivo = argv[2];  // Segundo argumento: Nome do Arquivo que será enviado


    configuraPorta(argv[3]);// Terceiro argumento: Testa se a porta do servidor está válida


    if((Arquivo = fopen(nomeArquivo, "rb")) == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo original %s\n", arqProp.nome);
        exit(1);
    }

    fseek(Arquivo, 0, SEEK_END); // posiciona ponteiro no final do arquivo
    arqProp.tamanho = ftell(Arquivo);   // pega o valor corrente do ponteiro
    fseek(Arquivo, 0, SEEK_SET); // posiciona de volta no início do arquivo

    nomeArquivo = nomeBaseArquivo(nomeArquivo);

    if( strlen(nomeArquivo) > NOMEARQUIVOMAX)
    {
        fprintf(stderr,"Nome de arquivo excede o tamanho máximo de %d!\n", NOMEARQUIVOMAX);
        exit(1);
    }


    sprintf(arqProp.nome, "%s", nomeArquivo);

    printf("Abriu arquivo: %s\nTamanho: %li\n", arqProp.nome, arqProp.tamanho);

    // Cria um socket datagrama/UDP
    if ((Sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // Constrói a estrutura do endereço destino
    memset(&servAddr, 0, sizeof(servAddr));       // Zera a estrutura
    servAddr.sin_family = AF_INET;               // Internet addr family
    servAddr.sin_addr.s_addr = inet_addr(servIP);// Endereço de IP do servidor
    servAddr.sin_port   = htons(Porta);          // Porta do servidor

    // Limpa dados do datagrama
    memset(dataToServer.dados, 0, TAMDADOSMAX);

    dataToServer.flags = SYN;
    dataToServer.sequencia = 0;
    dataToServer.idRecebido = 0;
    dataToServer.janela = (TAMDADOSMAX*TAMJANELA < arqProp.tamanho) ? TAMJANELA : arqProp.tamanho/TAMDADOSMAX + 1;
    memcpy(dataToServer.dados, &(arqProp), sizeof(struct fileHeader));

    // Envia cabeçalho do arquivo para o servidor
    enviaPacote(Sock,servAddr,dataToServer);

    dataFromServer = recebePacote(Sock,&fromAddr,1);

    if((dataFromServer->flags & SYN) != SYN)
    {
        DieWithError("Conexão recusada!");
    }
    else
    {
        printf("Inicialização OK!\n");
    }


    // Transfere conteudo do arquivo para servidor
    while(!feof(Arquivo))
    {
        memset(BufferJanela, 0, sizeof(BufferJanela));
        fread(&BufferJanela, (TAMDADOSMAX * dataToServer.janela), 1, Arquivo);

        Reenviar = 1;
        while(Reenviar)
        {
            for(ContJanela=0; ContJanela<dataToServer.janela; ContJanela++)
            {
                dataToServer.flags = ACK;
                dataToServer.sequencia++;
                memcpy(&(dataToServer.dados), (BufferJanela+(TAMDADOSMAX*ContJanela)), TAMDADOSMAX);

                // Envia conteudo do arquivo para o servidor
                printf("Conteúdo do arquivo %s\n",dataToServer.dados);
                enviaPacote(Sock,servAddr,dataToServer);
            }

            // Depois de enviar toda a janela, recebe uma resposta
            dataFromServer = recebePacote(Sock,&fromAddr,1);
            Reenviar = ((dataFromServer->flags & ACK) != ACK) ? 1 : 0;
        }

        // Ajusta tamanho da última janela
        dataToServer.janela = (TAMDADOSMAX*dataToServer.sequencia + dataToServer.janela*TAMDADOSMAX < arqProp.tamanho) ?
                               dataToServer.janela :
                               (arqProp.tamanho-TAMDADOSMAX*dataToServer.sequencia)/TAMDADOSMAX + 1;
    }

    fclose(Arquivo);

    /* Envia sinalizacao do fim da tranferencia do arquivo para o servidor */
    dataToServer.flags = FIM;
    dataToServer.sequencia = 0;

    enviaPacote(Sock,servAddr,dataToServer);

    close(Sock);

    return 0;
}
