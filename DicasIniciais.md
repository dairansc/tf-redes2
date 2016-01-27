## Instalação ##

Ferramentas necessárias:
  * Subversion - faz o controle de versões SVN
  * CodeBlocks - compilador e editor de projetos C/C++
  * RabbitVCS - gerenciador gráfico para o SVN no próprio Nautilus
  * Meld - comparador (diff) gráfico de arquivos

Instalar ferramentas com os seguintes comandos:
```
sudo add-apt-repository ppa:rabbitvcs/ppa
sudo apt-get update
sudo apt-get install codeblocks meld subversion 
```
Se a interface do linux for Unit
```
sudo apt-get rabbitvcs-nautilus3 rabbitvcs-gedit rabbitvcs-cli
```
senão
```
sudo apt-get rabbitvcs-nautilus rabbitvcs-gedit rabbitvcs-cli
```

## Iniciar Repositório ##

Para obter o repositório do projeto, como usuário normal através do terminal, acesse o diretório onde será armazenado um repositório local, por exemplo _desktop_, e digite o comando:
```
svn checkout https://tf-redes2.googlecode.com/svn/trunk/ tf-redes2 --username <usuario>@gmail.com

Quando for perguntado, entre com o a senha gerada em http://code.google.com/hosting/settings.
```
**checkout** será utilizado somente desta vez, ele cria um diretório _tf-redes2_ que será o repositório local do projeto.


## Testar ##

Abra o arquivo _tf-redes2.cbp_ encontrado em _tf-redes2/codigos_ com o programa CodeBlocks, faça alguma modificação em algum arquivo e salve a modificação.
Agora voltando ao terminal, acesse o diretório _tf-redes2_ que foi criado pelo comando _svn checkout_ e digite o comando:
```
svn status
```
**status** verifica todo o diretório por modificações tanto no repositório local como no servidor, se houver, será mostrado o nome do arquivo que foi modificado, e antes uma letra que pode ser:
  * **A** adicionado
  * **D** deletado
  * **M** modificado
  * **C** existe conflito, foi modificado no repositório local e no servidor
  * **?** arquivo não foi adicionado ao repositório, para adicionar ele digite `svn add <caminho e nome do arquivo>`
Se anteriormente foi modificado apenas um arquivo, provavelmente ele aparecerá aqui, para ver o que foi modificado digite:
```
svn diff <caminho e nome do arquivo>
```
**diff** mostra as linhas que foram alteradas, colocando **+** antes das linhas que foram adicionadas e **-** antes das linhas que foram removidas.
Para enviar as modificações para o servidor, antes de tudo e para evitar conflitos de arquivos digite o comando:
```
svn update
```
**update** atualiza o conteúdo do repositório local com as modificações do servidor.
E aí sim digite:
```
svn commit
```
**commit** envia as modificações feitas no repositório local, para o servidor.
Os comandos _commit_ e _update_ podem ser feitos de um ou mais arquivos específicos, bastando passar o caminho e nome dos arquivos após o comando.
Lembrando que por exemplo um `svn commit` somente ira fazer commit do diretório e subdiretórios em que se está acessando, então se não for fazer de arquivos específicos, utilizar estes comandos, a partir da raiz do repositório (diretório tf-redes2).

E para voltar atrás digite:
```
svn update --revision PREV <caminho e nome do arquivo ou nenhum para tudo>
```
ao invés de PREV, poderia ser o número da versão em que se deseja voltar atrás.


## Modificações em diretórios ou arquivos ##
Basicamente são os mesmos comandos por linha de comando do linux, mas colocando o comando `svn <operação>`, por exemplo:
```
svn [delete|del|remove|rm] <arquivo ou diretório>
svn mv <arquivo ou diretório>
svn [copy|cp] <arquivo ou diretório>
svn mkdir <diretório>
```
Após fazer essas modificações, se for executado o comando `svn status` ele vai indicar quais alterações foram feitas e ainda não foram postadas para o servidor.


## Resumo ##

...