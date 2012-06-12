#include "biblioteca.c"

void DieWithError(char *errorMessage);  /* External error handling function */

/*Se o arquivo existir o valor 1 (true) será retornado. Caso
  contrário a função retornará 0 (false). */
int file_exists(const char *filename)
{
  FILE *arquivo;
  int tam = strlen(filename);

  if ((arquivo = fopen(filename, "rb")) != NULL)
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

    char nome_arquivo[BUFFERMAX];
    int id_nome , iniciaComunicacao = 0, janela;
    long int sequencia;
    char nome_teste[10] = "teste";

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
            dataFromClient = recebePacote(Sock,&clntAddr,servAddr,0);
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

            if((arqDestino = fopen(nome_arquivo,"wb")) == NULL)
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
                /* Envia um datagrama de volta para o cliente confirmando inicio da comunicacao*/
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
                    dataFromClient = recebePacote(Sock,&clntAddr,servAddr,1);
                    if((dataFromClient->flags & FIM) == FIM)
                    {
                        fclose(arqDestino);
                        iniciaComunicacao = 0;
                        printf("Fim da transmissao.\n");
                    }
                    else if(!Reenviar)
                    {
                        memcpy((BufferJanela+(TAMDADOSMAX*ContJanela)), dataFromClient->dados, TAMDADOSMAX);
                        
                        Reenviar = (sequencia != dataFromClient->sequencia - 1) ? 1 : 0;
                        printf("Verificando sequencia atual %li com sequencia recebida %li. Tamanho do Buffer de Janela: %d\n",sequencia,dataFromClient->sequencia-1, TAMDADOSMAX*ContJanela);
                    }
                    sequencia = dataFromClient->sequencia;
                }
				printf("%s\n\n", BufferJanela);
                if(!Reenviar)
                {
                    janela = dataFromClient->janela;
                    printf("Realiza transferência para o arquivo\n");
                   // fwrite(&BufferJanela, (TAMDADOSMAX * janela), 1, arqDestino);
                   // fwrite(&BufferJanela, sizeof(BufferJanela), 1, arqDestino);
                    
                    /*
                    memset(TempBufferJanela, 0, sizeof(TempBufferJanela));
                   
                    for(ContJanela=0; ContJanela<janela; ContJanela++) {
                        memcpy(TempBufferJanela, (BufferJanela+(TAMDADOSMAX*ContJanela)), TAMDADOSMAX);
                        printf("Copiou %d.\n",sizeof(TempBufferJanela));
                        fwrite(&TempBufferJanela, sizeof(TempBufferJanela), 1, arqDestino);
                        printf("Gravou no arquivo %s.\n",TempBufferJanela);
                    }
                    */ 

                    //for(ContJanela=0; ContJanela<(TAMDADOSMAX*TAMJANELA); ContJanela++) {
                    //    printf("Copiou %d.\n",sizeof(TempBufferJanela));
                        fwrite(&BufferJanela, 1, sizeof(BufferJanela), arqDestino);
                    //    printf("Gravou no arquivo %s.\n",TempBufferJanela);
                    //}
                     
                    //printf("Desliza janela para %d.\n",janela);
                }
                else
                {
                    dataToClient.flags = NACK;
                    printf("Envio de NACK.\n",janela);
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
