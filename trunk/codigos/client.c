#include "biblioteca.c"

void DieWithError(char *errorMessage);  // External error handling function

int main(int argc, char *argv[])
{
    int Sock; // descritor de Socket
    struct sockaddr_in servAddr; // Endereço destino
    struct sockaddr_in fromAddr; // Endereço origem

    struct fileHeader arqProp, *teste;
    struct datagramHeader dataToServer;
    struct datagramHeader *dataFromServer = (struct datagramHeader*)malloc(sizeof(struct datagramHeader));

    char *servIP;                    // Endereço IP do servidor
    char *nomeArquivo;                // String com o nome do arquivo
    char dados[TAMDADOSMAX];

	int lastPos = 0;

    if ((argc < 3) || (argc > 4))    // Testa pelo número correto de argumentos
    {
        fprintf(stderr,"Uso: %s <IP do Servidor> <Nome do Arquivo> [<Porta do Servidor>]\nPorta padrão: %d\n", argv[0], PORTAPADRAO);
        exit(1);
    }

    servIP = argv[1];       // Primeiro argumento: Endereço IP jdo Servidor
    nomeArquivo = argv[2];  // Segundo argumento: Nome do Arquivo que será enviado


    configuraPorta(argv[3]);// Terceiro argumento: Testa se a porta do servidor está válida


    if((arqOrigem = fopen(nomeArquivo, "rb")) == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo original %s\n", arqProp.nome);
        exit(1);
    }

    fseek(arqOrigem, 0, SEEK_END); // posiciona ponteiro no final do arquivo
    arqProp.tamanho = ftell(arqOrigem);   // pega o valor corrente do ponteiro
    fseek(arqOrigem, 0, SEEK_SET); // posiciona de volta no início do arquivo

    nomeArquivo = nomeBaseArquivo(nomeArquivo);

    if( strlen(nomeArquivo) > NOMEARQUIVOMAX)
    {
        fprintf(stderr,"Nome de arquivo excede o tamanho máximo de %li!\n", NOMEARQUIVOMAX);
        exit(1);
    }


    sprintf(arqProp.nome, "%s", nomeArquivo);

    printf("Arquivo: %s\nTamanho: %li Bytes\n", arqProp.nome, arqProp.tamanho);

    // Cria um socket datagrama/UDP
    if ((Sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // Constrói a estrutura do endereço destino
    memset(&servAddr, 0, sizeof(servAddr));       // Zera a estrutura
    servAddr.sin_family = AF_INET;               // Internet addr family
    servAddr.sin_addr.s_addr = inet_addr(servIP);// Endereço de IP do servidor
    servAddr.sin_port   = htons(Porta);          // Porta do servidor

    // Limpa dados do datagrama
    memset(dados, 0, TAMDADOSMAX);

    dataToServer.flags = SYN;
    dataToServer.sequencia = 0;
    dataToServer.idRecebido = 0;
    dataToServer.janela = (TAMDADOSMAX*TAMJANELA < arqProp.tamanho) ? TAMJANELA : arqProp.tamanho/TAMDADOSMAX + 1;
    
    memset(Buffer, 0, sizeof(Buffer));
    memcpy(Buffer, &(dataToServer), sizeof(dataToServer));
    memcpy(Buffer+sizeof(dataToServer), &(arqProp), sizeof(arqProp));
    
    // Envia cabeçalho do arquivo para o servidor
    enviaPacote(Sock,servAddr, Buffer, sizeof(dataToServer)+sizeof(arqProp)-NOMEARQUIVOMAX+strlen(arqProp.nome));

    recebePacote(Sock,&fromAddr,servAddr, Buffer);
	dataFromServer = (struct datagramHeader*)Buffer;
	
    if((dataFromServer->flags & SYN) != SYN)
    {
        DieWithError("Conexão com o servidor recusada!");
    }
    
    printf("Conexão com o servidor estabelecida, enviando dados...\n");


    // Transfere conteudo do arquivo para servidor
    while(!feof(arqOrigem))
    {
        
        memset(BufferJanela, 0, sizeof(BufferJanela));
        fread(&BufferJanela, 1, (TAMDADOSMAX * dataToServer.janela), arqOrigem);

        Reenviar = 1;
        while(Reenviar)
        {
            for(ContJanela=0; ContJanela<dataToServer.janela; ContJanela++)
            {
                dataToServer.flags = ACK;
                dataToServer.sequencia++;

				memset(Buffer, 0, sizeof(Buffer));
				memcpy(Buffer, &(dataToServer), sizeof(dataToServer));
				memcpy(Buffer+sizeof(dataToServer), (BufferJanela+TAMDADOSMAX*ContJanela), TAMDADOSMAX);
				//printf("%s",Buffer+sizeof(dataToServer));
                // Envia conteudo do arquivo para o servidor
                enviaPacote(Sock,servAddr,Buffer,BUFFERMAX);
                
            }
			
            // Depois de enviar toda a janela, recebe uma resposta
            memset(Buffer, 0, sizeof(Buffer));
            recebePacote(Sock,&fromAddr,servAddr, Buffer);
            dataFromServer = (struct datagramHeader*)Buffer;
            Reenviar = ((dataFromServer->flags & ACK) != ACK) ? 1 : 0;
            if(Reenviar){
				dataToServer.sequencia -= dataToServer.janela;
				printf("Servidor solicitou reenvio da janela.\n");
			}
            //printf("Resposta do servidor se é para reenviar = %d\n",Reenviar);
        }
        
        // Ajusta tamanho da última janela
        dataToServer.janela = (TAMDADOSMAX*dataToServer.sequencia + dataToServer.janela*TAMDADOSMAX < arqProp.tamanho) ?
                               dataToServer.janela :
                               (arqProp.tamanho-TAMDADOSMAX*dataToServer.sequencia)/TAMDADOSMAX + 1;
        //printf("\n\n\nJanela:%d->%d\n\n\n\n", i, dataToServer.janela);
        printf("Enviando o arquivo '%s' para o servidor %s\n%li%c de %li Bytes\n", arqProp.nome, servIP, ((dataToServer.sequencia-1)*100)/(arqProp.tamanho/TAMDADOSMAX), '%', arqProp.tamanho);
    }

    fclose(arqOrigem);

    // Envia sinalizacao do fim da tranferencia do arquivo para o servidor
    dataToServer.flags = FIM;
    dataToServer.sequencia = 0;
    
	memset(Buffer, 0, sizeof(Buffer));
    memcpy(Buffer, &(dataToServer), sizeof(dataToServer));
    enviaPacote(Sock,servAddr,Buffer,sizeof(dataToServer));

    close(Sock);
	printf("Fim da transmissao.\n");
    return 0;
}
