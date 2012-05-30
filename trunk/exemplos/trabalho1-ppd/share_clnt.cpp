// share_clnt.c código do cliente para a aplicação Share Center

#include "biblioteca.h"

// Estrutura que encapsula dados para lista de reenvio de arquivos ao servidor 
struct listaEspera {
//    int qtArquivos;
    string nomeArquivo;
    int status; 
};


listaEspera listaReenvio[QNT_ARQS_USER];

int qtArquivos = 0;

int iniciaLista(void){
  int i;

  for(i=0;i<=QNT_ARQS_USER;i++){
    listaReenvio[i].status = LISTA_ARQ_VAZIA;
  }
}

int incluiLista(string nomeArquivo){
    int ind, incluiu = 1;

    ind = qtArquivos;

    if (qtArquivos <= QNT_ARQS_USER) {
        listaReenvio[ind].nomeArquivo = nomeArquivo; 
        listaReenvio[ind].status = AGUARDANDO_ENVIO;

        qtArquivos++;
    }
    else
        incluiu = 0; //não foi possivel incluir arquivo na lista

    return incluiu;
}

/*Remove e reorganiza na lista os arquivos não enviados*/
void removeEnviadosLista(){
    listaEspera listaAux[QNT_ARQS_USER];
    int i, indice=0;

    //realiza copia para lista auxiliar dos nomes de arquivos que nao foram enviados
    for (i=0;i<qtArquivos;i++) {
        if ((listaReenvio[i].status != ENVIO_SUCESSO) && (listaReenvio[i].status != LISTA_ARQ_VAZIA)) { 
            listaAux[indice].nomeArquivo = listaReenvio[i].nomeArquivo; 
            listaAux[indice].status = AGUARDANDO_ENVIO;
            indice++;
        }
    }

    qtArquivos = indice;
    //inclui na lista do cliente somente os arquivos pendentes
    if (qtArquivos > 0) {
		cout << " " << endl;
		cout << "____________________________Arquivos Pendentes/Lista de envio para o servidor____________________________" << endl;

		for (i=0;i<qtArquivos;i++) {
            switch(listaReenvio[i].status){
                case ERRO_SEM_ESPACO:
                    cout << "   " << listaReenvio[i].nomeArquivo << "\t  status: espaço insuficiente no diretório" << endl;
                break;
                case ERRO_ARQ_JA_EXISTE:
                    cout << "   " << listaReenvio[i].nomeArquivo << "\t  status: arquivo já existe no servidor, portanto será excluido da lista" << endl;
                    listaReenvio[i].status = LISTA_ARQ_VAZIA;
                    qtArquivos--;
                break;
                case ERRO_ABRIR_ARQ:
                    cout << "   " << listaReenvio[i].nomeArquivo << "\t  status: erro arquivo ao abrir arquivo, portanto será excluido da lista" << endl;
                    listaReenvio[i].status = LISTA_ARQ_VAZIA;
                    qtArquivos--;
                break;
                default:
                    cout << "   " << listaReenvio[i].nomeArquivo << "\t  status: resposta inesperada/falha na conexão" << endl;
                    listaReenvio[i].status = LISTA_ARQ_VAZIA;
                    qtArquivos--;
            }

            if ((listaAux[i].status == AGUARDANDO_ENVIO) && (listaReenvio[i].status != LISTA_ARQ_VAZIA)) { 
    	        listaReenvio[i].nomeArquivo = listaAux[i].nomeArquivo; 
                listaReenvio[i].status = listaAux[i].status;
		    }
 
		}
		cout << "_________________________________________________________________________________________________________" << endl;
    }
    else {
		cout << " " << endl;
		cout << " Não há arquivos na lista de espera para envio ao servidor. " << endl;
    }
}

void share(string usuario, string nomeArquivo, CLIENT *cl){
    transporteArquivo *arquivo = (transporteArquivo *) malloc(sizeof(transporteArquivo));
    ifstream file;
    int result;
    // guarda previamente nome do usuário na estrutura de transferência de arquivos
    strncpy(arquivo->usuario, usuario.c_str(), TAM_MAX_NOME_USER); 
    
    if(nomeArquivo.length()>TAM_MAX_NOME_ARQ){ // se nome do arquivo for muito grande
        cout << " " << endl;
        cout << "Nome do arquivo não pode ter mais de " << TAM_MAX_NOME_ARQ << " caracteres." << endl;
    }
    else if((arquivo->tamanho = tamanhoArquivo((usuario + "/" + nomeArquivo))) < 0){ // se retornar -1
        //file.open((usuario + "/" + nomeArquivo).c_str(), ios::binary );
        //if(!file.is_open() || !file.good()){
        //if(!(file = fopen( (usuario + "/" + nomeArquivo).c_str(), "r"))){
        cout << " " << endl;
        cout << "Não foi possível abrir o arquivo." << endl;
    }
    else if(arquivo->tamanho > TAM_MAX_REPOSITORIO){
        //file.seekg(0, ios::end);
        //fseek(file, 0, SEEK_END);
        //arquivo->tamanho = file.tellg();
        //file.seekg(0, ios::beg);
        
        cout << " " << endl;
        cout << "Arquivo não pode conter mais de " << TAM_MAX_REPOSITORIO << " bytes." << endl;
    }
    else{
        // guarda nome do arquivo na estrutura de tranferência de arquivos
        strncpy(arquivo->nome, nomeArquivo.c_str(), TAM_MAX_NOME_ARQ); 
        file.open((usuario + "/" + nomeArquivo).c_str(), ios::binary );
        file.read(arquivo->conteudo, arquivo->tamanho); // guarda conteúdo do arquivo
        file.close();

        cout << " " << endl;
        cout << "Executando upload do arquivo '" << nomeArquivo << "'..." << endl;
        // executa e verifica resultado
        if((result = *share_args_1(arquivo,cl))){
            switch(result){
                case ERRO_SEM_ESPACO:
                    cout << " " << endl;
                    cout << "Sem espaço no servidor, arquivo colocado na lista de espera, execute o comando 'emptyqueue' para tentar enviar os arquivos da lista da espera." << endl;
                    incluiLista(nomeArquivo);
                break;
                case ERRO_ARQ_JA_EXISTE:        
                    cout << " " << endl;
                    cout << "Arquivo já compartilhado, escolha outro." << endl;
                break;
                default:
                    cout << " " << endl;
                    cout << "Resposta inesperada ou conexão perdida com o servidor." << endl;
            }
        }
        else{
            cout << " " << endl;
            cout << "Envio de arquivo com sucesso!" << endl;
        }
    }
}

void download(string usuario, string nomeArquivo, CLIENT *cl){
    // variável utilizada somente para poder enviar usuário e nome do arquivo solicitado
    transporteArquivo *dados = (transporteArquivo *) malloc(sizeof(transporteArquivo)); 
    // variável que receberá o conteúdo do arquivo solicitado
    transporteArquivo arquivo; 
    
    ofstream file;
    
    if(tamanhoArquivo(usuario + "/" + nomeArquivo) >= 0){ // se arquivo existe localmente
        cout << " " << endl;
        cout << "Não é possível fazer o download do arquivo '" << nomeArquivo << "', pois ele já existe no diretório local." << endl;
    }
    else{
        cout << " " << endl;
        cout << "Executando download do arquivo '" << nomeArquivo << "'..." << endl;
        
        // prepara argumentos para solicitação
        strncpy(dados->usuario, usuario.c_str(), TAM_MAX_NOME_USER);
        strncpy(dados->nome, nomeArquivo.c_str(), TAM_MAX_NOME_ARQ);
        
        arquivo = *download_args_1(dados,cl);
        
        if(arquivo.tamanho < 0){ // se o arquivo não foi encontrado no servidor
            cout << " " << endl;
            cout << "Não foi possível encontrar ou abrir o arquivo '" << nomeArquivo << "' no repositório." << endl;
        }
        else{
            file.open((usuario + "/" + nomeArquivo).c_str(), ofstream::binary );
            file.write(arquivo.conteudo, arquivo.tamanho); // guarda conteúdo do arquivo baixado no arquivo local
            file.close();

            cout << " " << endl;
            cout << "Download do arquivo realizado com sucesso!" << endl;
        }
    }
}

void emptyqueue(string usuario, CLIENT *cl){
    transporteArquivo *arquivo = (transporteArquivo *) malloc(sizeof(transporteArquivo));
    ifstream file;
    int result, indice;
    // guarda previamente nome do usuário na estrutura de transferência de arquivos
    strncpy(arquivo->usuario, usuario.c_str(), TAM_MAX_NOME_USER); 

    for(indice=0;indice<qtArquivos;indice++){
		// guarda tamanho do arquivo para testes
		arquivo->tamanho = tamanhoArquivo((usuario + "/" + listaReenvio[indice].nomeArquivo));

		if(arquivo->tamanho < 0){ // se retornar -1
		    listaReenvio[indice].status = ERRO_ABRIR_ARQ;
		}
		else if(arquivo->tamanho > TAM_MAX_REPOSITORIO){
		    listaReenvio[indice].status = ERRO_SEM_ESPACO;
		}
		else{
		    // guarda nome do arquivo na estrutura de tranferência de arquivos
		    strncpy(arquivo->nome, listaReenvio[indice].nomeArquivo.c_str(), TAM_MAX_NOME_ARQ); 
		    file.open((usuario + "/" + listaReenvio[indice].nomeArquivo).c_str(), ios::binary );
		    file.read(arquivo->conteudo, arquivo->tamanho); // guarda conteúdo do arquivo
		    file.close();

		    //cout << "Executando upload do arquivo '" << nomeArquivo << "'..." << endl;
		    // executa e verifica resultado
		    if((result = *share_args_1(arquivo,cl))){
		        switch(result){
		            case ERRO_SEM_ESPACO:
		                listaReenvio[indice].status = ERRO_SEM_ESPACO;
		            break;
		            case ERRO_ARQ_JA_EXISTE:
		                listaReenvio[indice].status = ERRO_ARQ_JA_EXISTE;
		            break;
		            default:
		                listaReenvio[indice].status = ERRO_SERVIDOR;
		        }
		    }
		    else{ //enviado com sucesso
	           listaReenvio[indice].status = ENVIO_SUCESSO;
               if (qtArquivos > 0)
                 qtArquivos--;
		    }
		}
	}
    removeEnviadosLista();
}

void list(CLIENT *cl){
    char *result;
    result = *list_1((void *) NULL, cl);
    
    cout << " " << endl;
    cout << "Lista de arquivos compartilhados: " << endl;
    cout << result << endl;

    cout << "  " << endl;

}

main(int argc, char *argv[]) {
    
    CLIENT *cl;
    int result; // utilizado nos resultados de chamadas do servidor para verificar status da execução
    string usuario, ipServidor; // parâmetros vindos do terminal
    string comando; // comando digitado pelo usuário durante a execução do programa
    string nomeArquivo;
    iniciaLista;
    
    qtArquivos = 0; //inicia contador da lista de espera para upload em zero
        

    // Verifica se parâmetros passados na chamada do programa estão corretos
    if (argc != 3) { // se não foi passado exatamente 2 parâmetros
        cout << " " << endl;
        cout << "Use: " << argv[0] << " <usuario> <IPservidor>" << endl; 
        exit(1);
	}
	if (strlen(argv[1]) > TAM_MAX_NOME_USER){ // se nome usuário muito grande
        cout << " " << endl;
	    cout << "Nome do usuário não pode ter mais de " << TAM_MAX_NOME_USER << " caracteres." << endl;
	    exit(1);
	}
	usuario = (string)argv[1];
    ipServidor = (string)argv[2];

    // Em relação ao diretório do usuário
    if(mkdir(usuario.c_str(), 0777) == -1){ // se ocorrer um erro na criação, 0777 é o tipo de permissão do diretório
        if (errno != 17){ // se este erro não for sinalizando que o diretório já existe
            cout << " " << endl;
            cout << "Não foi possível criar um diretório com o nome do usuário '" << usuario.c_str() << "' devido ao erro: " << strerror(errno) << endl;
            exit(1);
        }
    }

    // Verifica se é possível a conexão com o servidor
	if (!(cl = clnt_create(ipServidor.c_str(), SHARECENTERPROG,SHAREVERSION,"tcp"))) {
        clnt_pcreateerror(ipServidor.c_str()); 
        exit(1); 
    }
    
    // Verifica usuário
    if((result = *conectuser_args_1(toStringRPC(usuario),cl))){
        switch(result){
            case ERRO_MAX_CON:
                cout << " " << endl;
                cout << "Número máximo de conexões atingidas no servidor. Total de usuários já conectados: " << NUM_MAX_CON << "." << endl;
                break;
            case ERRO_JA_CON:
                cout << " " << endl;
                cout << "Usuário '" << usuario << "' já conectado ao servidor." << endl;
                break;
            default:
                cout << " " << endl;
                cout << "Resposta inesperada ou conexão perdida com o servidor." << endl;
        }
        exit(1);
    }
        
    // Exibição de menu
    cout << " " << endl;
    cout << "---------------------------------------------------Share Center---------------------------------------------------" << endl;
    cout << " " << endl;
    cout << "Digite um comando:" << endl;
    cout << "\\share <nome_do_arquivo>\tCompartilha um arquivo." << endl;
    cout << "\\list\t\t\t\tSolicita a lista de todos os arquivos compartilhados." << endl;
    cout << "\\download <nome_do_arquivo>\tBaixa um arquivo que esteja compartilhado no servidor." << endl;
    cout << "\\emptyqueue\t\t\tTenta fazer o envio dos arquivos que estão na fila de espera de um dado cliente." << endl;
    cout << "\\leave\t\t\t\tTermina a conexão com o servidor e finaliza o programa." << endl << endl;
    cout << "------------------------------------------------------------------------------------------------------------------" << endl;
    cout << " " << endl;
    do{
        cout << "\\";
        getline(cin,comando); // espera por comandos
        
        if(!comando.find("share ")){
            // depois que o usuario digitar o comando
            // comando.find_first_of(" ")+1 // procura pela posição onde existe espaço em branco e adiciona 1 posição
            // comando.substr( n ) // retorna string de n até o fim
            nomeArquivo = comando.substr( comando.find_first_of(" ")+1 );

            share(usuario, nomeArquivo, cl);
        }
        
        else if(!comando.compare("list")){
            list(cl);
        }
        
        else if(!comando.find("download ")){
            nomeArquivo = comando.substr( comando.find_first_of(" ")+1 );
            download(usuario, nomeArquivo, cl);
        }
        
        else if(!comando.compare("emptyqueue")){
            emptyqueue(usuario, cl);
        }
        
        else if(comando.compare("leave")!=0){
            cout << " " << endl;
            cout << "Comando desconhecido." << endl;
        }
    }while(comando.compare("leave")!=0);
    
    // Desconecta usuário do servidor
    if((result = *desconectuser_args_1(toStringRPC(usuario),cl))){
        cout << " " << endl;
        cout << "Resposta inesperada ou conexão perdida com o servidor." << endl;
        exit(1);
    }
    cout << " " << endl;
    cout << "Programa e conexão finalizados pelo usuário." << endl;
} 
