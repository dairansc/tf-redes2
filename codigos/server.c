#include "biblioteca.c"

//Função externa que trata erros na comunicação por sock
void DieWithError(char *errorMessage);  // External error handling function

//Função que testa se já existe arquivo com o mesmo nome do 
//arquivo enviado pelo cliente.
//Retorna 1 (true) se arquivo existir e 0 (false) caso contrário.
int file_exists(const char *filename)
{
  FILE *arquivo;
  int tam = strlen(filename);

  if ((arquivo = fopen(filename, "rb")) != NULL)
  {
    //printf("file exist %s %d\n",filename,tam);
    fclose(arquivo);
    return 1;
  }

  return 0;
}

//Função que alterar nome do arquivo, em caso de teste em localhost.
void altera_nome_arquivo (char nome_antigo[BUFFERMAX], int id_nome, char *nome_novo)
{
    int tam = strlen(nome_antigo);

    //busca extensao do arquivo
    char *p = strrchr(nome_antigo, '.');
    *p++;
    //renomea arquivo para nome_antigo+id.extensao
    strncat(nome_novo,nome_antigo,tam-4);
    sprintf(nome_novo, "%s%d.", nome_novo, id_nome);
    strcat(nome_novo, p);
}

int main(int argc, char *argv[])
{
    int Sock; 					 // descritor de Socket
    struct sockaddr_in servAddr; // Endereço destino
    struct sockaddr_in clntAddr; // Endereço origem
    
    struct fileHeader *arqProp;
    struct datagramHeader dataToClient;
    struct datagramHeader *dataFromClient = (struct datagramHeader*)malloc(sizeof(struct datagramHeader));

    char nome_arquivo[BUFFERMAX];
    int id_nome , iniciaComunicacao = 0, janela;
    long int sequencia, tamanhoArquivo;
    char nome_teste[10] = "teste";
    char dados[TAMDADOSMAX];

	int i, qntRecebida;
	long int qntGravar;

    if (argc > 2)    // Testa pelo número correto de argumentos
    {
        fprintf(stderr,"Uso: %s [<Porta a ser aberta>]\nPorta padrão: %d\n", argv[0], PORTAPADRAO);
        exit(1);
    }
    
    // Testa se a porta do servidor está válida
    configuraPorta(argv[1]);
    // Cria socket para enviar/receber datagramas
    if ((Sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // Constrói a estrutura de endereço local
    memset(&servAddr, 0, sizeof(servAddr));   // Zero out structure
    servAddr.sin_family = AF_INET;                // Internet address family
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    servAddr.sin_port = htons(Porta);      // Local port

    // Bind to the local address
    if (bind(Sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    for (;;) // Garante que servidor ficará ativo a espera de mensagens do cliente
    {
        if (iniciaComunicacao == 0)
        {
            // Servidor fica bloqueado até que receba a mensagem do cliente
            printf("Aguardando comunicação do cliente...\n");
            memset(Buffer, 0, sizeof(Buffer));
            recebePacote(Sock,&clntAddr,servAddr, Buffer);

			dataFromClient = (struct datagramHeader*)Buffer;
			
			if((dataFromClient->flags & SYN) != SYN)
			{
				printf("Informação recebida do ip %s, mas não é um pacote de sincronização\n", inet_ntoa(clntAddr.sin_addr));
			}
			else
			{
				//Recebe campos do cabeçalho para criar arquivo na máquina serividora
				janela = dataFromClient->janela;
				sequencia = dataFromClient->sequencia;
				arqProp = (struct fileHeader *)(Buffer+sizeof(struct datagramHeader));
				tamanhoArquivo = arqProp->tamanho;
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
				dataToClient.janela = janela;

				if((arqDestino = fopen(nome_arquivo,"wb")) == NULL)
				{
					dataToClient.flags = FIM;

					printf("Erro ao abrir o arquivo cópia %s\n", nome_arquivo);
					iniciaComunicacao = 0;
				}
				else
				{
					dataToClient.flags = SYN;
					// Envia um datagrama de volta para o cliente confirmando inicio da comunicacao
					printf("Conexão com o cliente estabelecida, aguardando transmissão de dados...\n");
					iniciaComunicacao = 1;
				}
				memset(Buffer, 0, sizeof(Buffer));
				memcpy(Buffer, &(dataToClient), sizeof(dataToClient));
				enviaPacote(Sock,clntAddr,Buffer,sizeof(dataToClient));
			}
        }
        else if(iniciaComunicacao == 1)
        {
            //Permanece nesse estado até que transmissão seja finalizada pelo cliente
            Reenviar = 1;
            while(Reenviar)
            {
				
                memset(BufferJanela, 0, sizeof(BufferJanela));
                Reenviar = 0;
                for(ContJanela=0; ContJanela<janela; ContJanela++)
                {

                    memset(Buffer, 0, sizeof(Buffer));
                    recebePacote(Sock,&clntAddr,servAddr, Buffer);
                    dataFromClient = (struct datagramHeader*)Buffer;
                    if((dataFromClient->flags & FIM) == FIM)
                    {
                        fclose(arqDestino);
                        iniciaComunicacao = 0;
                        Reenviar = 0;
                        printf("Fim da transmissao.\n");
                        exit(1);
                        
                    }
                    else if(!Reenviar) //Recebeu o pacote da sequencia da janela
                    {
                        memcpy((BufferJanela+(TAMDADOSMAX*ContJanela)), Buffer+sizeof(struct datagramHeader), TAMDADOSMAX);
                        
                        Reenviar = (sequencia != dataFromClient->sequencia - 1) ? 1 : 0;
                    }
                    sequencia = dataFromClient->sequencia;
                    janela = dataFromClient->janela;
                }
                
                //Após receber a janela, verifica se ela veio completa 
                if(!Reenviar) 
                {
                    qntGravar = (sequencia*TAMDADOSMAX > tamanhoArquivo) ? (janela-1)*TAMDADOSMAX+tamanhoArquivo-((sequencia-1)*TAMDADOSMAX) : TAMDADOSMAX*janela;
                    fwrite(&BufferJanela, 1, qntGravar, arqDestino);

                    dataToClient.flags = ACK;
                    
                }
                else //Janela não foi recebida completamente, sinaliza cliente para reenvio da janela
                {
                    dataToClient.flags = NACK;
                    printf("Pacotes perdidos, solicitando reenvio da janela...\n");
                    
                }
                memset(Buffer, 0, sizeof(Buffer));
				memcpy(Buffer, &(dataToClient), sizeof(dataToClient));
				enviaPacote(Sock,clntAddr,Buffer,sizeof(dataToClient));
				printf("Recebendo o arquivo '%s' do cliente %s\n%li%c de %li Bytes\n", nome_arquivo, inet_ntoa(clntAddr.sin_addr), ((sequencia-1)*100)/(tamanhoArquivo/TAMDADOSMAX), '%', tamanhoArquivo);
            }
        }
    }
}
