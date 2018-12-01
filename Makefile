all:
	CC=gcc
	$(CC) -o duplicadosT duplicadosT.c libmd5.a -pthread
make run:
	./duplicadosT -t 1 -m e -d prueba
	./duplicadosT -t 1 -m l -d prueba
	./duplicadosT -t 5 -m e -d prueba
	./duplicadosT -t 5 -m l -d prueba
	./duplicadosT -t 10 -m e -d prueba
	./duplicadosT -t 10 -m l -d prueba
	./duplicadosT -t 15 -m e -d prueba
	./duplicadosT -t 15 -m l -d prueba
	./duplicadosT -t 20 -m e -d prueba
	./duplicadosT -t 20 -m l -d prueba
clear:
	-rm duplicadosT
