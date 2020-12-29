#define IP_SERVER "10.0.2.15"

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <queue>
#include <map>
#include "markdown.h"

using namespace std;

typedef struct datagrama{
  int seq;
  int checksum;
  char texto[512];
} Dgram;

int chksm(char arr[]){
  int pos = 0;
  int chksm = 0;
  while (arr[pos] != '\0'){
    chksm += arr[pos];
    arr++;
  }
  return chksm;
}

int main(int argc, char **argv){
  srand(time(NULL));
  time_t hora = time(NULL);
  if(argc != 2){
    printf("Usage: %s <port>\n", argv[0]);
    exit(0);
  }
  system("clear");
  //definicion de markdown para consola. ignorar.
  Markdown::Mod r(Markdown::FG_RED);
  Markdown::Mod g(Markdown::FG_LIGHT_GREEN);
  Markdown::Mod b(Markdown::FG_BLUE);
  Markdown::Mod y(Markdown::FG_YELLOW);
  Markdown::Mod d(Markdown::FG_DEFAULT);
  Markdown::Mod bold(Markdown::BOLD);
  Markdown::Mod nonb(Markdown::NO_BOLD);
  Markdown::Mod it(Markdown::ITALIC);
  Markdown::Mod nit(Markdown::NO_ITALIC);
  Markdown::Mod un(Markdown::UNDERLINE);
  Markdown::Mod nun(Markdown::NO_UNDERLINE);
  Markdown::Mod bgr(Markdown::BG_RED);
  Markdown::Mod bgd(Markdown::BG_DEFAULT);

  int port = atoi(argv[1]);
  int sockfd;
  struct sockaddr_in srvAdd;
  struct sockaddr_in cliAdd;
  char serial[512];
  char recv_ack[10];
  Dgram data;
  socklen_t addr_size;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&srvAdd, '\0', sizeof(srvAdd));
  srvAdd.sin_family = AF_INET;
  srvAdd.sin_port = htons(port);
  srvAdd.sin_addr.s_addr = inet_addr(IP_SERVER);

  bind(sockfd, (struct sockaddr*)&srvAdd, sizeof(srvAdd));
  addr_size = sizeof(cliAdd);

  //socket timeout en 7 segundos
  struct timeval tv;
  tv.tv_sec = 7;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
  //recibir cliente
  recvfrom(sockfd, serial, sizeof(serial), 0, (struct sockaddr*)&cliAdd, &addr_size);
  cout <<g<<"Recibido: "<<d<<serial<<endl<<endl;

  char synack[8] = "SYN/ACK";
  sendto(sockfd, synack, sizeof(synack), 0, (struct sockaddr *)&cliAdd, sizeof(cliAdd));
  cout <<b<<"Enviado: "<<d<<synack<<endl<<endl;
  //abrir archivo de texto
  ifstream txtfile;
  txtfile.open("file.txt");
  string str;
  int seq = 0;
  int rcv;
  //inicializar queue
  queue<Dgram> cola;
  //cargar cola con primeros 10 paquetes
  while (cola.size() < 10){
    getline(txtfile, str);
    seq++;
    size_t l = str.copy(data.texto, str.length());
    data.texto[l] = '\0';
    data.seq = seq;
    data.checksum = chksm(data.texto);
    cola.push(data);
  }
  while (cola.size() != 0){
    //meter datos en la estructura Dgram
    if (cola.size() < 10){
      if (getline(txtfile, str)){
        seq++;
        size_t l = str.copy(data.texto, str.length());
        data.texto[l] = '\0';
        data.seq = seq;
        data.checksum = chksm(data.texto);
        cola.push(data);
      }
    }
    //Serializar la estructura a un char[]
    memcpy(serial, &cola.front(), sizeof(cola.front()));
    //enviar la estructura serializada
    //? 10% posibilidad de NO enviar el paquete nunca. Simula perdida de paquete
    int rng = rand()%10;
    if (rng < 9) sendto(sockfd, serial, sizeof(serial), 0, (struct sockaddr *)&cliAdd, sizeof(cliAdd));
    //log
    cout <<d<<ctime(&hora)<<b<<bold<<un<<"--Datos enviados--"<<nun<<y<<"\n   Seq: "<<g<<cola.front().seq<<y<<nit<<bold<<"\n   Checksum: "<<d<<data.checksum<<y<<"\n   Linea: "<<d<<it<<nonb<<cola.front().texto<<nit<<endl<<endl;
    if (rng > 8) cout <<it<<r<<bold<<"  (Paquete no enviado a proposito para simular perdida)"<<nit<<nonb<<endl<<endl;
    rcv = recvfrom(sockfd, recv_ack, sizeof(recv_ack), 0, (struct sockaddr*)&cliAdd, &addr_size);
    if (rcv == -1){
      cout <<d<<ctime(&hora)<<bgr<<bold<<d<<un<<"SOCKET TIMEOUT"<<nonb<<bgd<<nun<<d<<endl;
    }
    else{
      int recseq;
      memcpy(&recseq, recv_ack, sizeof(recseq));
      cout <<d<<ctime(&hora)<<g<<bold<<un<<"--ACK Recibido--"<<nun<<y<<"\n   Seq: "<<g<<recseq<<nonb<<endl<<endl;
      if (recseq == cola.front().seq) cola.pop();
      else{
        cout <<bgr<<bold<<un<<"ACK incorrecto"<<nonb<<nun<<bgd<<d<<endl;      
      }
    }
    sleep(rand()%6+1);
  }
  char end[14] = "ENDCONNECTION";
  char endack[8];
  sendto(sockfd, end, sizeof(end), 0, (struct sockaddr *)&cliAdd, sizeof(cliAdd));
  cout <<d<<ctime(&hora)<<r<<bold<<un<<"--Enviado cierre de conexión--"<<nun<<nonb<<endl<<endl;
  while(recvfrom(sockfd, endack, sizeof(endack), 0, (struct sockaddr*)&cliAdd, &addr_size) == -1){
    sendto(sockfd, end, sizeof(end), 0, (struct sockaddr *)&cliAdd, sizeof(cliAdd));
  }
  cout <<d<<ctime(&hora)<<r<<bold<<un<<"--Recibido END ACK--"<<nun<<nonb<<endl<<endl;
  close(sockfd);
  cout <<d<<ctime(&hora)<<bgr<<d<<bold<<un<<"SOCKET CERRADO"<<nun<<nonb<<bgd<<endl<<endl;
}