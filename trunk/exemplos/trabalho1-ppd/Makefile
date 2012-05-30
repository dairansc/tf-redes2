###############################################
# Trabalho 1 PPD - Share Center
###############################################
#                                             #
# Makefile simples para compilar o programa   #
# SHARE, exemplo de um servidor simples de    #
# rpc. Nesta funcao remota o processo         #
# sao passados como parametro dois numeros    #
# inteiros retornando a soma entre eles       #
#                                             #
#                                             #
###############################################

RM=rm -f
CC=g++
CFLAGS=


all: generate_stubs shareclient shareserver

shareclient: 
	$(CC) $(CFLAGS) share_clnt.cpp shareit_clnt.o shareit_xdr.o -o shareclient 

shareserver: 
	$(CC) $(CFLAGS) share_svc.cpp shareit_svc.o shareit_xdr.o -o shareserver


generate_stubs: 
	rpcgen shareit.x
	$(CC) -c shareit_clnt.c
	$(CC) -c shareit_svc.c
	$(CC) -c shareit_xdr.c

clean:
	$(RM) shareit_clnt.*
	$(RM) shareit_svc.*
	$(RM) shareit_xdr.*
	$(RM) shareit.h
	$(RM) shareclient
	$(RM) shareserver
	$(RM) -fr *~

