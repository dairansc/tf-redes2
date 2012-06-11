#define BUFFERMAX 1500
#define PORTAPADRAO 10000

#define TAMDADOSMAX BUFFERMAX-sizeof(short)-sizeof(long int)-sizeof(long int)-sizeof(int)
#define NOMEARQUIVOMAX TAMDADOSMAX-sizeof(long int)
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

struct datagramHeader
{
    short flags;            // indicadores de conexão
    long int sequencia;
    long int idRecebido;    // Acknowlegement
    int janela;             // tamanho da janela
    char dados[TAMDADOSMAX];
};


unsigned short Porta;     // Porta aberta de conexão
FILE *Arquivo;              // Ponteiro para o arquivo
short Reenviar;             // Indica se janela precisa ser reenviada
char BufferJanela[TAMDADOSMAX*TAMJANELA];
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

void enviaPacote(int Sock, struct sockaddr_in Addr, struct datagramHeader pacote)
{
    char buffer[BUFFERMAX];

    printf("%d\n\n", pacote.janela);

    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &(pacote), sizeof(pacote));

    printf("envia pacotes.\n");

    if (sendto(Sock, buffer, BUFFERMAX, 0, (struct sockaddr *) &Addr, sizeof(Addr)) != BUFFERMAX)
        DieWithError("Error: sendto() sent a different number of bytes than expected");

    printf("enviou pacotes sem erro.\n");
}

struct datagramHeader *recebePacote(int Sock, struct sockaddr_in *Addr, short verificarOrigem)
{
    char buffer[BUFFERMAX];
    unsigned int fromSize = sizeof(Addr);
    struct datagramHeader* recPacote = (struct datagramHeader*)malloc(sizeof(struct datagramHeader));

    memset(buffer, 0, sizeof(buffer));

    // Recebe resposta
    memset(buffer, 0, sizeof(buffer));
    if ((recvfrom(Sock, buffer, BUFFERMAX, 0, (struct sockaddr *) &Addr, &fromSize)) != BUFFERMAX)
        DieWithError("recvfrom() failed");
    
   // printf("recebePacote Handling client %s\n", inet_ntoa(Addr->sin_addr));
    
/*   if (verificarOrigem && ToAddr.sin_addr.s_addr != FromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }
*/
    printf("Recebe pacotes, antes memcpy.\n");
    memcpy(recPacote, buffer, sizeof(buffer));
    printf("Recebe pacotes, depois memcpy.\n");

    return recPacote;

}

char *nomeBaseArquivo(char *caminho)
{
    char *base = strrchr(caminho, '/');
    return base ? base+1 : caminho;
}
