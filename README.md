# SO_2022

No âmbito da cadeira de Sistemas Operativos, implementou-se um programa em C que torna possível o armazenamento segura e eficiente de cópias de ficheiros.

Assim, o programa é capaz de aplicar funcionalidades de compressão/descompressão e cifragem/decifragem dos ficheiros a serem armazenados, entre elas:
   - __bcompress__ / __bdecompress__. Comprime / descomprime dados com o formato bzip2.
   - __gcompress__ / __gdecompress__. Comprime / descomprime dados com o formato gzip.
   - __encrypt__ / __decrypt__. Cifra / decifra dados.
   - __nop__. Copia dados sem realizar qualquer transformação.


---------------

Começamos por compilar o programa através do comando:
 
 `$ make`
 
 Para descompilar o comando:
 
 `$ make clean`
 
 ---------------
  
De seguida, executamos o programa abrindo dois terminais dentro da diretoria _temp/_, um do servidor e outro do cliente.

O servidor deve ser executado primeiro com o seguinte formato.
 
 `$ ./servidor <server-configfile> <pasta de executáveis das transformações>`
 
 Neste caso, este formato já tem os dados necessários pré-definidos:
  
 ``` $ ./servidor ../src/config.txt ../bin/ ```
  
  
Para executar os pedidos utiliza-se o comando com os formatos: 
 
 `./cliente status` : para saber o estado do servidor.
 `./cliente proc-file input output transf1 transf2 ...` : para executar   transformações no ficheiro input.
 Um exemplo de uma execução de transformações é:
 `./cliente proc-file ../input.txt output nop bcompress nop encrypt gcompress`
 E para retornar o ficheiro de output à sua forma original faz-se as operações contrárias:
 ```./cliente proc-file output output2 gdecompress decrypt nop bdecompress nop```
 
---------------
