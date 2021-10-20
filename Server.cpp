#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define MSG_SIZE 1024
using namespace std;
class concetion_handler;
class MessageHandler;
class Clients;


class Server{
private:
   char ip_address[50];
   int portno;
   int sockfd;
   
   int Initialize_server(){
      int sockfd;
      struct sockaddr_in serv_addr;     
      // socket function 
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0) {
         perror("ERROR opening socket");
         return -1;
      }
      bzero((char *) &serv_addr, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = INADDR_ANY;
      serv_addr.sin_port = htons(portno);
      /* Now bind the host address using bind() call.*/
      if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
         perror("ERROR on binding");
         return -1;
      }
      listen(sockfd,5);
      cout<<" Server is listing on "<<portno<<endl;
      return sockfd;
      }

public:
   Server(int port=8000){
      portno = port;
      sockfd = Initialize_server();
   }
   int Accept_client_connection(){
      if(sockfd==-1){
         cout<<"Server Initialization failed"<<endl;
         exit(1);

      }
      int client_con;//clilen;
      struct sockaddr_in cli_addr;
      socklen_t clilen = sizeof(cli_addr);   
      client_con = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); 
      if (client_con < 0) {
         perror("ERROR on accept Client Resquest");
         return -1;
      }
      return client_con;
   }
};





class concetion_handler{
   Clients *client_id;
   int server_id;
   int exitflag;
public:
   concetion_handler(Clients *client){
      exitflag = true;
      client_id = client;
   }
   void readMsgHandler();
};


void myread(concetion_handler *c){
   c->readMsgHandler();
}

void thread_handler(Clients *client){
   concetion_handler *c = new concetion_handler(client);
   //cout<<" client has connted"<<endl;
   //thread *t2 = new thread(mywrite,c);
   thread *t1 = new thread(myread,c);
  // t1->join();
  // cout<<"returnint from thread_handler"<<endl;
}




class Clients{
   int client_sock;
   int client_ID;
   char name[20];
   int exitflag = 0;
public:
   Clients(int Aclient_sock):client_sock(Aclient_sock){
      exitflag = 1;
   }
   Clients(int Aclient_sock, int Aclient_ID, char *ch){
      client_sock = Aclient_sock;
      client_ID = Aclient_ID;
      strcpy(name,ch);
      exitflag = 1;
   }
   int& getClientSockfd(){
      return client_sock;
   }
   void getClientName(char *ch){
      strcpy(ch,name);
   }
   int getClientID(){
      return client_ID;
   }
   void setClientName(char *ch){
      strcpy(name,ch);
   }
   void setClientID(int id){
       client_ID = id;
   }
   void show(){
      cout<<" Client ID = "<<client_ID<<endl;
      cout<<" sockfd  = "<<client_sock<<endl;
      cout<<name;
   }
   char * getCleintDetails(int i){
      char *rtbuff = new char[50];
      rtbuff[0] = '\0';
      //sprintf(rtbuff,"%d\t%d\t%d\t%s",i,client_ID,client_sock,name);
      sprintf(rtbuff,"%d\t%s",i,name);
      return rtbuff;
   }
   void setClientsOnlineStatus(int i){
      exitflag = i;
   }
   int getClientOnlineStatus(){
      return exitflag;
   }
   ~Clients(){

     // cout<<"client destroctor called"<<endl;
   }
};



class ClientStateHandler{

   ClientStateHandler(){}; 
public:
   static vector<Clients *> client;
   static void addClient(Clients *c){
     ClientStateHandler::client.push_back(c);
   }
   static void  removeClient(Clients *c){
      int i  = 0 ;
      vector<Clients *>::iterator it1,it2;
      it1 = ClientStateHandler::client.begin();
      it2 = ClientStateHandler::client.end();
         
      for(;it1!=it2;++it1){
         if(*it1==c){
            client.erase(it1);
            delete c;   
         }
      }
   }

  static Clients* getClientBySockfd(int sockfd){
      vector<Clients *>::iterator it;
      it = ClientStateHandler::client.begin();
      for(;it!=ClientStateHandler::client.end();++it){
         if((*it)->getClientSockfd()==sockfd){
            return (*it);
         }
      }
   }

   static Clients* getClientByIndex(int i){
      if(i<ClientStateHandler::client.size())
      return ClientStateHandler::client[i];
      return nullptr;
   }
   
   static char * getAllClientInformation(){
      char *returnbuff = new char[1024];
      char *client_info;
      returnbuff[0] = '\0';
      vector<Clients *>::iterator it;
      it = ClientStateHandler::client.begin();
      int i =1;
     
      sprintf(returnbuff,"Index:\tClientName");
      for(;it!=ClientStateHandler::client.end();++it){   
            client_info = (*it)->getCleintDetails(i++);
            strcat(returnbuff,"\n");
            strcat(returnbuff,client_info);
            delete client_info;         
      }
      return returnbuff;
   }

};

void IntializationOfClient(int &sockfd){
   Clients *c = new Clients(sockfd);
   char name[30];
   read(sockfd,name,sizeof(name));
   //char name[]="Vishal kumar";
   cout<<"Login By Client: "<<name<<endl;
   int ID = 120+sockfd;
   c->setClientID(ID);
   c->setClientName(name);
   ClientStateHandler::addClient(c);
   thread_handler(c);
}



class MessageHandler{
   Clients *sender, *receiver;
   char msg[MSG_SIZE];

public:
   MessageHandler(){}
   MessageHandler(char *ch){
      strcpy(msg,ch);
   }
   void setMessageHandler(char *ch, Clients *Asender){
      strcpy(msg,ch);
      sender = Asender;
      receiver = nullptr;
   }
   MessageHandler(char *ch,Clients *Asender,Clients *Areceiver){
      strcpy(msg,ch);
      sender = Asender;
      receiver = Areceiver;
   }
   void setClients(Clients *Asender,Clients *Areceiver){
      sender = Asender;
      Areceiver = Areceiver;
   }
   void setreceiver(int i){
      receiver = ClientStateHandler::getClientByIndex(i-1);
   }
   void sendMessageToClient(int i = 0){
     // setreceiver();

      if(receiver==nullptr){
         cout<<"receiver not found"<<endl;
         receiver = sender;
         sprintf(msg,"Server Response: Sorry receiver is Offline \n");
         i = 1;
         //return;
      }

      int &client_sockfd = receiver->getClientSockfd();
      int &client_sockfd_sender = sender->getClientSockfd();
      
      char *senderName = new char[20];
      
      if(i==0){ sender->getClientName(senderName);    
               char tempbuff[MSG_SIZE];
               strcpy(tempbuff,(msg+2));
               sprintf(msg,"%s Say : %s",senderName,tempbuff);
               char msgres[50] ="Server Response: Message has delivered";
              if(receiver!=sender) write(client_sockfd_sender,(msgres),sizeof(msgres));
      }
      delete senderName;

      if(write(client_sockfd,(msg),sizeof(msg))>0){
       //  cout<<"Message sent:"<<endl;  
     }
   }
  void sendMessageToAllClient(){
      char *senderName = new char[20];
      sender->getClientName(senderName);
      char temp[1000];
      strcpy(temp,msg+2);
      sprintf(msg,"%s Say: %s",senderName,temp);
      int i = -1;
      while(1){
         i++;
         Clients *c = ClientStateHandler::getClientByIndex(i);
         if(c==nullptr)break;
         if(c == sender) {
            continue;
         }
         if(write(c->getClientSockfd(),msg,sizeof(msg))>0){
          //  cout<<"Message sent:"<<endl;  
         }
      }
      char msgres[100] ="Server Response: Message has delivered to all users";
      write(sender->getClientSockfd(),msgres,sizeof(msgres));

      
  }
  void parseMessage(){
   //cout<<"receiver msg"<<endl;
   //cout<<msg;
   int d1 = msg[0]-'0';
   if(d1==1){
      int d2 = msg[1]-'0';
      if(d2==0){
         sendMessageToAllClient();
      }else{
         setreceiver(d2);
         sendMessageToClient();
      }
   }
   if(d1==2){
      receiver = sender;
      char *client_info = ClientStateHandler::getAllClientInformation();
      sprintf(msg,"%s",client_info);
      delete client_info;
      sendMessageToClient(1);
   }
   
   if(d1==3){
      //cout<<"receiverd mas"<<endl;
     // cout<<msg;
      receiver = sender;
      int itr = 0;
      int i = 1;
      while(msg[i]!='\0'){
         char x = msg[i];
         if(x==' ') break;
         int ax = x - '0';
         itr = (itr*10) + ax;
         i++;
      }
      char *client_name = new char[20];
      sender->getClientName(client_name);
      char filename[50];
      strcpy(filename,client_name);
      delete client_name;
      strcat(filename,msg+i);
      //cout<<filename;
      downloadFile(itr,filename);
   }
   if(d1==4){
      receiver = sender;
      char filename[30];
      strcpy(filename,msg+1);
      sendFile(filename);
      // send file;
      //char str[30]
   }

   if(d1==5){
      char *Name = new char[30];
      sender->getClientName(Name);
      cout<<"Logout by client "<<Name<<endl;
      delete Name;
      sender->setClientsOnlineStatus(0);
      int sockfd = sender->getClientSockfd();
      close(sockfd);
   }
  }
  void sendFile(char *filename){
   char send_buffer[1024];
   FILE *fp= NULL;
   int sock = sender->getClientSockfd();
   fp = fopen(filename,"rb");
   if(fp==NULL){
      bzero(send_buffer, MSG_SIZE);
      sprintf(send_buffer,"-1");
      write(sock, send_buffer,sizeof(send_buffer));
      printf("\n Error to read %s \n",filename);
      return;
   }
   fseek(fp, 0, SEEK_END);
   long long int total_byte=ftell(fp);
   int itr = (total_byte/1024) + 1;
   printf("total lengeth of file is %lld  total ite = %d \n",total_byte,itr);
   bzero(send_buffer, MSG_SIZE);
   sprintf(send_buffer,"0%d",itr);
   send(sock, send_buffer,sizeof(send_buffer), 0);
   bzero(send_buffer, MSG_SIZE);
   fseek(fp, 0, 0);
   char ch;
   int i = 0;
   int  x1 = 0;
   long long int x = total_byte;
   while(1){
      ch = fgetc(fp);
      send_buffer[i++] = ch;
      x--;
      if(x==0)
      {  printf("sending %d part \n",x1++);
         write(sock,send_buffer,sizeof(send_buffer));
         break;
      }
      if(i==1023){
         i = 0;
         write(sock,send_buffer,sizeof(send_buffer));
         printf("sending %d part \n",x1++);
      }
   }
   fclose(fp);
   printf("complet send \n");
}

 void downloadFile(int itr,char *filename){
   char loaclbuf[1030];
   int client_sock = sender->getClientSockfd();
   if(itr == -1){
      printf("\nFile not found at Server\n");
      return;
   }
      //printf("\nFile itr %d \n",itr);
   FILE *fout = NULL;
   //char filename[20]="temp.data";
   fout = fopen(filename,"w");
   if(fout==NULL){
      printf("\nError to Write File\n");
      return;
   }

   int i = 0,j; 
   char ch;
   for(j = 0; j < itr; j++){
     // printf("reading file %d\n",j);
      i = 0;
      int l = read(client_sock,loaclbuf,sizeof(loaclbuf));
      //printf("%d recived byte %s \n",l,loaclbuf);
       while(i<1023){
         ch = loaclbuf[i++];
         if(ch==EOF)break;
         fputc(ch,fout);
       }
   }
   fclose(fout);
   printf("\n%s File saved Complete\n",filename);
   //send(sock, send_buffer, strlen(send_buffer), 0);
}


};


vector<Clients *> ClientStateHandler::client = vector<Clients *>(0);
int main( int argc, char *argv[] ) {
   vector<int> client(100);
   int i = 1;
   Server s;
   while(1){
      client[i] = s.Accept_client_connection();
      IntializationOfClient(client[i]);
   }
    return 0;
}


void concetion_handler::readMsgHandler(){
      char buf[MSG_SIZE];
      int count = 0;
      MessageHandler mh;
      if(client_id==nullptr) return;
      int server_id = client_id->getClientSockfd();
      while(exitflag){
         int re;
         if(client_id==nullptr) return;
         exitflag = client_id->getClientOnlineStatus();
         if(exitflag==0)break;
         bzero(buf, sizeof(buf));
         re = read(server_id,buf,sizeof(buf));
         if(client_id==nullptr) return;
         exitflag = client_id->getClientOnlineStatus();
         if(exitflag==0)break;
         if(re<=0) {
              // cout<<"total recive butye "<<re<<count<<endl;
            count++;
            if(count==10){
               cout<<"Error: conncetoin lost \nClosing Client socket"<<endl;
               close(server_id);
               exitflag = false;
               client_id->setClientsOnlineStatus(0);
               ClientStateHandler::removeClient(client_id);
               return;
            }
            continue;
         }else{
            count = 0;
         }
         mh.setMessageHandler(buf,client_id);
         mh.parseMessage();
      }
      ClientStateHandler::removeClient(client_id);
   }