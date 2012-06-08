#define BUFFERMAX 1500
#define PORTAPADRAO 10000
#define NOMEARQUIVOMAX BUFFERMAX-sizeof(long int)-sizeof(int)
#define TAMDADOSMAX BUFFERMAX-sizeof(struct datagramHeader)
#define TAMJANELA 10

// indicadores de conexão
#define SYN   0x01
#define FIM   0x02
#define ACK   0x04
#define NACK  0x08

struct fileHeader
{
    long int tamanho;
    int tamanhoNome;
    char nome[NOMEARQUIVOMAX];
};

struct datagramHeader
{
    short flags;            // indicadores de conexão
    long int sequencia;
    long int idRecebido;    // Acknowlegement
    int janela;             // tamanho da janela
};




char *nomeBaseArquivo(char *caminho)
{
    char *base = strrchr(caminho, '/');
    return base ? base+1 : caminho;
}
