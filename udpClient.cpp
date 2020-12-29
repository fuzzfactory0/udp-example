#define IP_SERVER "10.0.2.15"

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <map>
#include <queue>
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
    cout << "Uso: "<<argv[0]<<" <puerto>"<<endl;
    exit(0);
  }
  system("clear");
  
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
  Markdown::Mod bgb(Markdown::BG_BLUE);
  Markdown::Mod bgd(Markdown::BG_DEFAULT);

  int port = atoi(argv[1]);
  int sockfd;
  char serial[512];
  struct sockaddr_in serverAddr;
  socklen_t addr_size = sizeof(serverAddr);

  sockfd = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&serverAddr, '\0', sizeof(serverAddr));

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr(IP_SERVER);

  char syn[4] = "SYN";
  char recv_ack[10];
  sendto(sockfd, syn, sizeof(syn), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  cout <<b<< "--Enviado: " <<d<< syn << endl<<endl;

  recvfrom(sockfd, recv_ack, 1024, 0, NULL, 0);
  cout <<g<< "--Recibido: " <<d<< recv_ack << endl<<endl;
  int lastseq = 0;
  bool endc = false;
  queue<Dgram> recibidos;
  while(!endc){
    recvfrom(sockfd, serial, 1024, 0, NULL, 0);
    if (memcmp("ENDCONNECTION", serial, 13) == 0){
      cout <<d<<ctime(&hora)<<r<<bold<<un<<"--Recibido cierre de conexión--"<<nun<<nonb<<endl<<endl;
      char endack[8] = "END/ACK";
      sendto(sockfd, endack, sizeof(endack), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
      cout <<d<<ctime(&hora)<<r<<bold<<un<<"--Enviado END ACK--"<<nun<<nonb<<endl<<endl;      
      endc = true;
      break;
    }
    Dgram data;
    //recuperar la estructura serializada
    memcpy(&data, serial, sizeof(data));
    bool enviarAck;
    if (data.seq != lastseq){
      cout <<d<<ctime(&hora)<<g<<bold<<un<< "--Datos recibidos--" <<nun<<y<<"\n  Seq: "<<b<< data.seq <<y<<nit<<nonb<<endl<<endl;
      if (recibidos.size()<10){
        if (data.checksum == chksm(data.texto)){
        cout <<d<<bold<<"--Checksum esperado: "<<data.checksum<<"\n--Checksum calculado: "<<chksm(data.texto)<<bgb<<bold<<"\n--Checksum correcto.--"<<nonb<<bgd<<endl<<endl;
          recibidos.push(data);
          enviarAck = true;
        }
        else{
          cout <<d<<bold<<"--Checksum esperado: "<<data.checksum<<"\n--Checksum calculado: "<<chksm(data.texto)<<bgr<<bold<<"Checksum incorrecto."<<data.checksum<<nonb<<bgd<<endl<<endl;
          enviarAck = false;
        }
      }
      else{
        enviarAck = false;
      }
    }
    else cout <<d<<ctime(&hora)<<g<<bold<<un<< " --Datos recibidos--" <<nun<<r<<"\n  Paquete duplicado. Descartado."<<nonb<<d<<endl<<endl;
    char serack[32];
    //? 10% posibilidad de NO enviar el ACK nunca. Simula perdida de ACK
    int rng = rand()%10;
    if (rng < 9){
      if (enviarAck){
        memcpy(serack, &data.seq, sizeof(serack));
        sendto(sockfd, serack, sizeof(serack), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        cout <<d<<ctime(&hora)<<b<<bold<<un<<"--ACK Enviado--"<<nun<<y<<"\n   Seq: "<<g<<data.seq<<nonb<<endl<<endl;
      }
      else{
        memcpy(serack, &lastseq, sizeof(serack));
        sendto(sockfd, serack, sizeof(serack), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        cout <<d<<ctime(&hora)<<b<<bold<<un<<"--ACK Enviado--"<<nun<<y<<"\n   Seq: "<<g<<lastseq<<nonb<<endl<<endl;
      }
    }
    lastseq = data.seq;
    if (rng > 8) cout <<it<<r<<bold<<"  (ACK no enviado a proposito para simular perdida)"<<nit<<nonb<<endl<<endl;
    sleep(rand()%9+1);
    int al = rand()%3;
    for (int i = 0; i < al; i++){
      if (!recibidos.empty()){
        cout <<d<<ctime(&hora)<<y<<bold<<un<< "--Imprimiendo linea--" <<nun<<y<<"\n  Seq: "<<b<< recibidos.front().seq <<y<<nit<<bold<< "\n  Linea: "<<d<<it<<nonb<< recibidos.front().texto <<nit<<endl<<endl;
        recibidos.pop();
      }
    }
  }
  close(sockfd);
  cout <<d<<ctime(&hora)<<bgr<<d<<bold<<un<<"SOCKET CERRADO"<<nun<<nonb<<bgd<<endl<<endl;
  for (int i = 0; i < recibidos.size(); i++){
    cout <<d<<ctime(&hora)<<y<<bold<<un<< "--Imprimiendo linea--" <<nun<<y<<"\n  Seq: "<<b<< recibidos.front().seq <<y<<nit<<bold<< "\n  Linea: "<<d<<it<<nonb<< recibidos.front().texto <<nit<<endl<<endl;
    recibidos.pop();
    sleep(rand()%9+1);
  }
}