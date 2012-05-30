// Caso ocorra erro na execução somente do servidor, instalar o programa portmap com o comando:
// sudo apt-get install portmap

// share_svc.c código do servidor da aplicação Share Center


#include "biblioteca.h"


// Estrutura principal de controle de usuários e arquivos compartilhados
struct tabelaUsuarios{
    string nome;
    string arquivos[QNT_ARQS_USER];
};

int qntConexoes = 0; // marcador de quantidade de conexões mantidas
tabelaUsuarios usuarios[NUM_MAX_CON]; // tabela principal com controle de usuários



// Retorna posição de usuário na tabela através do nome
int procuraUsuario(string nome){
    int i;
    for(i=0; i<NUM_MAX_CON; i++){
        if(usuarios[i].nome == nome){
            return i; // encontrado
        }
    }
    return -1;// não encontrado
}

// Retorna primeira posição livre encontrada na tabela de usuários
int posicaoLivreUser(){
    int i;
    for(i=0; i<NUM_MAX_CON; i++){
        if(usuarios[i].nome.empty()){
            return i;
        }
    }
    return -1; // não encontrado
}

// Retorna posição livre na tabela de arquivos dentro da tabela de usuários
int posicaoLivreArquivo(int idUser){
    int i;
    for(i=0; i<QNT_ARQS_USER; i++){
        if(usuarios[idUser].arquivos[i].empty()){
            return i;
        }
    }
    return -1; // não encontrado
}

// Mostra organização da tabela de usuários e seus respectivos arquivos compartilhados
void debug(){
    int i,j;
    cout << " " << endl;
    cout << "------Espaço Livre: " << (int)(TAM_MAX_REPOSITORIO - tamanhoDiretorio((string)DIR_REP) ) << " ---Usuários:" << endl;
    for(i=0; i<NUM_MAX_CON; i++){ // percorre toda tabela de usuários
        if(!usuarios[i].nome.empty()){ // se não está vazio
            cout << usuarios[i].nome << " -> ["; // mostra nome do usuário
            for(j=0; j<QNT_ARQS_USER; j++){ // percorre toda tabela de arquivos do usuário
                if(!usuarios[i].arquivos[j].empty()){ // se não está vazio
                    cout << usuarios[i].arquivos[j] << "; "; // mostra nome do arquivo
                }
            }
            cout << "]";
        }
        cout << endl;
    }
    cout << " " << endl;
    cout << "------------------------------------------------------------------------------------------------------------------" << endl; 
}


int *conectuser_args_1_svc(char **userParam, struct svc_req *clnt) {
    static int result; // resultado da execução dessa função
    int i,j; // variáveis de controle
    string usuario = (string)*userParam; // conversão da string do stub para string do C++
    
    if(qntConexoes == 0){
        // Cria o repositório, só é realizada na primeira tentativa de conexão de um usuário
        if(mkdir(DIR_REP, 0777) < 0){ // se ocorrer um erro na criação
            if(limpaDiretorio((string)DIR_REP) < 0){
                cout << " " << endl;
                cout << "Não foi possível criar o repositório '" << DIR_REP << "' devido ao erro: " << strerror(errno) << endl;
                exit(1);
            }
        }

        // Limpa tabela de usuários, assegura que não tenha lixo na struct tabelaUsuarios
        for(i=0; i<NUM_MAX_CON; i++){
            usuarios[i].nome.clear();
            for(j=0; j<QNT_ARQS_USER; j++){
                usuarios[i].arquivos[j].clear();
            }
        }
    }

    if(procuraUsuario(usuario)>=0){ // verifica se usuário já não estava conectado
        result = ERRO_JA_CON;
    }
    else if(qntConexoes >= NUM_MAX_CON){ // se o limite máximo de conexões foi atingido
        result = ERRO_MAX_CON;
    }
    else{ // se tudo ok
        // coloca usuário na tabela de usuários conectados
        usuarios[ posicaoLivreUser() ].nome = usuario;
        qntConexoes++;
        cout << " " << endl;
        cout << "Usuário '" << usuario << "' conectado, total de " << qntConexoes << " usuário(s) conectado(s)." << endl;
        result = CON_OK;
    }
    
    debug();
    
    return ((int *) &result);
}


int *desconectuser_args_1_svc(char **userParam, struct svc_req *clnt) {
    static int result; // resultado da execução dessa função
    string usuario = (string)*userParam; // conversão da string do stub para string do C++
    int i, idUser = procuraUsuario(usuario);
   
    if(qntConexoes > 0){
        // Procura arquivos na tabela de arquivos do usuário para poder apagá-los
        for(i=0; i<QNT_ARQS_USER; i++){ // percorre toda tabela de arquivos do usuário
            if(!usuarios[idUser].arquivos[i].empty()){ // se não está vazio
                remove(((string)DIR_REP + "/" + usuarios[idUser].arquivos[i]).c_str());
            }
        }
        // Retira usuário da tabela de usuários conectados
        usuarios[idUser].nome.clear();

        qntConexoes--;
        cout << " " << endl;
        cout << "Usuário '" << usuario << "' desconectado, total de " << qntConexoes << " usuário(s) conectado(s)." << endl;
        result = CON_OK;
    }
    
    debug();
    
    return ((int *) &result);
}


int *share_args_1_svc(transporteArquivo *dados, struct svc_req *clnt) {
    static int result; // resultado da execução dessa função
    string nomeUsuario = (string)dados->usuario;
    string nomeArquivo = (string)dados->nome;
    int idUser = procuraUsuario(nomeUsuario);
    ofstream file; //criando uma variavel para arquivo de saida
    
    // verifica espaço no repositório
    cout << (TAM_MAX_REPOSITORIO - tamanhoDiretorio((string)DIR_REP) - dados->tamanho ) << endl;
    if((TAM_MAX_REPOSITORIO - tamanhoDiretorio((string)DIR_REP) - dados->tamanho ) <= 0){
        result = ERRO_SEM_ESPACO;
    }
    else if(tamanhoArquivo( (string)DIR_REP + "/" + nomeArquivo ) >= 0){ // se resultado for diferente de número negativo, então o arquivo existe
        result = ERRO_ARQ_JA_EXISTE;
    }
    else{
        file.open(((string)DIR_REP + "/" + nomeArquivo).c_str(), ofstream::binary);
        file.write(dados->conteudo, dados->tamanho);
        file.close();
        
        usuarios[ idUser ].arquivos[ posicaoLivreArquivo(idUser) ] = nomeArquivo;
        
        cout << " " << endl;
        cout << "Usuário '" << nomeUsuario << "' compartilhou o arquivo '" << nomeArquivo << "', espaço livre: " << (int)( TAM_MAX_REPOSITORIO - tamanhoDiretorio((string)DIR_REP) ) << "bytes." << endl;
        result = CON_OK;
    }
    
    debug();
    
    return ((int *) &result);
}

transporteArquivo *download_args_1_svc(transporteArquivo *dados, struct svc_req *clnt) {
    static transporteArquivo arquivo;
    string nomeUsuario = (string)dados->usuario;
    string nomeArquivo = (string)dados->nome;
    ifstream file; //criando uma variavel para arquivo de entrada
    
    if((arquivo.tamanho = tamanhoArquivo((string)DIR_REP + "/" + nomeArquivo)) >= 0){ // se retornar valor válido
        file.open(((string)DIR_REP + "/" + nomeArquivo).c_str(), ios::binary );
        file.read(arquivo.conteudo, arquivo.tamanho); // guarda conteúdo do arquivo
        file.close();
        cout << " " << endl;
        cout << "Usuário '" << nomeUsuario << "' realizou download do arquivo '" << nomeArquivo << "'." << endl;
    }
    
    debug();
    
    return ((transporteArquivo *) &arquivo);
}

char **list_1_svc(void *dump, struct svc_req *clnt) {
    string texto, bufTam;
    DIR *dirstream;
    struct dirent *direntry;
    int i, j, tam;

    dirstream = opendir(DIR_REP);

    if (!dirstream) {
      cout << " " << endl;
      cout << "Não foi possível listar." << endl;
      exit(1);
    }
    texto += "    _____________________________________________________________________________________________________________________________\n";
    texto += "    |\t "+preencheComEspaco("Arquivo")+"\t|\t"+preencheComEspaco("Usuario")+"\t| \t"+preencheComEspaco("Tamanho (bytes)")+"\t|\n";
    while (direntry = readdir (dirstream)){ // percorre cada arquivo do repositório
        if(direntry->d_name[0]!='.'){ // dentro de cada diretório existe '.' e '..', não incluir eles na contagem
            for(i=0; i<NUM_MAX_CON; i++){ // percorre toda tabela de usuários
                if(!usuarios[i].nome.empty()){ // se não está vazio
                    for(j=0; j<QNT_ARQS_USER; j++){ // percorre toda tabela de arquivos do usuário
                        if(usuarios[i].arquivos[j] == (string)direntry->d_name){ // se o arquivo pertence ao usuário
                            tam = tamanhoArquivo(((string)DIR_REP + "/" + (string)direntry->d_name)); 
                            bufTam = convertInt(tam);
                            texto += "    |\t "+preencheComEspaco((string)direntry->d_name)+"\t|\t"+preencheComEspaco(usuarios[i].nome)+"\t| \t"+preencheComEspaco(bufTam)+"\t|\n";
//texto += "        |      \t"+(string)direntry->d_name+"\t      |      \t"+usuarios[i].nome+"\t      |   \t      "+bufTam+"     \t | \n";
                        }
                    }
                }
            }
        }
    }
    (void) closedir (dirstream);    
    
    // faz conversão para retorno de string dinamicamente
    static char* r = new char(texto.length());
    strcpy(r,texto.c_str());
    
    return ((char **) &r);
}
