#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#define MSG_SIZE 1024

using namespace std;
int Menu();
void option();
class Client{
private:
   int sockfd;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   char server_address[50];
   int portno;

public:   
   Client(int port,char *serverAdd){
      strcpy(server_address,serverAdd);
      portno = port;
   }
   Client(int port){
      strcpy(server_address,"127.0.0.1");
      portno = port;
   }
   Client(){
    strcpy(server_address,"127.0.0.1");
    portno = 8000;  
   }
   int connect_to_server() {
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0) {
         cout<<"ERROR opening socket"<<endl;
         return -1;
      }
      server = gethostbyname(server_address);
      if (server == NULL) {
         fprintf(stderr,"ERROR, no such host\n");
         return -1;
      }
      bzero((char *) &serv_addr, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
      serv_addr.sin_port = htons(portno);
      if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
         perror("Error ");
         return -1;
      }
   return sockfd;
   }
   ~Client(){
      close(sockfd);
   }
};


class client_handler{
   Client *C;
   int client_id;
   int server_id;
   int exitflag;
   char filename[30];
public:
   client_handler(Client *c){
      exitflag = true;
      C = c;
      server_id = C->connect_to_server();
      if(server_id==-1){
         cout<<"Error : Server is not reachable."<<endl;
         exit(1);
      }
   }
   client_handler(int server, int client){
      server_id = server;
      client_id = client;
      exitflag = true;
   }
   void read_form_server(){
      char buf[MSG_SIZE];
      int countErrorMsg = 0;
      while(exitflag){
         int re;
         bzero(buf, sizeof(buf));
         re = read(server_id,buf,sizeof(buf));
         if(re<=0) {
            countErrorMsg++;
            if(countErrorMsg == 10){
               cout<<"Error: Conncetion lost "<<endl;
               close(server_id);
               exit(0);
            }
         }else{ countErrorMsg = 0;}
         if(buf[0]=='0'){
            int itr = atoi(buf);
            downloadFile(itr);
            option();
            continue;
         }
         cout<<endl<<"Message recived: "<<endl;
         cout<<"-------------------------------"<<endl;
         cout<<buf<<endl;
         cout<<"-------------------------------"<<endl<<endl;
         option();
      }
   }
   void sendName(char *buf,int len){
      write(server_id,buf,len);
   }
   void write_to_server(){
      char buf[MSG_SIZE];
      string str;
      option();
      while(exitflag){
         bzero(buf, sizeof(buf));
         //int ch = Menu();
         int ch;
         cin>>ch;
         if(ch<1||ch>6){
            cout<<"wrong choice"<<endl;
            option();
            continue;
         }
         if(ch==5){
            exitflag = false;
            buf[0] = '5';
            buf[1] ='\0';
         }else
         if(ch==2){
            sprintf(buf,"22");
         }else

         if(ch==3||ch==4){
            cout<<"Enter filename"<<endl;
            cin>>str;
            sprintf(filename,"%s",str.c_str());
            sprintf(buf,"%d%s",ch,str.c_str());
            
            if(ch==3){
               sendFile();
            }else{
               write(server_id,buf,sizeof(buf));
              // downloadFile();
            }
            option();
            continue;

         }else{
            cout<<"Enter client number to send all active Client press 0"<<endl;
            int client_no;
            cin>>client_no;
            cout<<"Enter Your Message:"<<endl;
            
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            getline(cin,str);
            sprintf(buf,"1%d%s",client_no,str.c_str());
           }
            if(write(server_id,buf,sizeof(buf))>0){
               if(ch==5){
                  cout<<"Connection close Message has sent."<<endl;
                  close(server_id);
                  delete this;
               }else{
                  cout<<"Message has sent:"<<endl;   
               }
               
         }
                
      }
   }
   int getServerId(){
      return server_id;
   }

   void downloadFile(int itr){
   char loaclbuf[1030];

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
    //  printf("reading file %d\n",j);
      i = 0;
      int l = read(server_id,loaclbuf,sizeof(loaclbuf));
      //printf("%d recived byte %s \n",l,loaclbuf);
       while(i<1023){
         ch = loaclbuf[i++];
         if(ch==EOF)break;
         fputc(ch,fout);
       }
   }
   fclose(fout);
   printf("\nFile download Complete\n");
   //send(sock, send_buffer, strlen(send_buffer), 0);
}


void sendFile(){
   char send_buffer[1024];
   FILE *fp= NULL;
   int sock = server_id;
   fp = fopen(filename,"r");
   if(fp==NULL){
      bzero(send_buffer, MSG_SIZE);
     // sprintf(send_buffer,"3-1");
     // write(sock, send_buffer,sizeof(send_buffer));
      printf("\n Error to read file %s \n",filename);
      return;
   } 
   fseek(fp, 0, SEEK_END);
   long long int total_byte=ftell(fp);
   int itr = (total_byte/1024) + 1;
   printf("total lengeth of file is %lld  total ite = %d \n",total_byte,itr);
   bzero(send_buffer, MSG_SIZE);
   sprintf(send_buffer,"3%d %s",itr,filename);
   cout<<"-------------------"<<endl;
   cout<<send_buffer<<endl;
   cout<<"-------------------"<<endl;
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

};

void myWrite(client_handler *c){
   c->write_to_server();
}
void myread(client_handler *c){
   c->read_form_server();
}

int Menu(){
   int i;
   while(1){
      cout<<"1. Send Message"<<endl;
      cout<<"2. Online Clients"<<endl;
      cout<<"3. Upload file"<<endl;
      cout<<"4. Download file"<<endl;
      cout<<"5. log out"<<endl<<"Enter your choice"<<endl;
      
      cin>>i;
      if(i<1){
         cout<<"wrong choice"<<endl;
         continue;
      }
      if(i>5){
         cout<<"wrong choice"<<endl;
         continue;
      }
   }
   return i;//-'0';
}
void option(){
   cout<<"1. Send Message"<<endl;
   cout<<"2. Online Clients"<<endl;
   cout<<"3. Upload file"<<endl;
   cout<<"4. Download file"<<endl;
   cout<<"5. log out"<<endl<<"Enter your choice"<<endl;
}

int main(int argc, char *argv[]) {
   int ptr;
   char server_address[30];
   char Name[30];
   Client *Cli;
   client_handler *ch;
   string n;
   cout<<"Please Enter your name"<<endl;
   getline(cin,n);
   strcpy(Name,n.c_str());
   if(argc>=3){
      cout<<"Connecting  to server"<<argv[2]<<" on Port "<<argv[1]<<"."<<endl;
      ptr = atoi(argv[1]);
      strcpy(server_address,argv[2]);
      Cli = new Client(ptr,server_address);
   }else{
      if(argc==2){
         cout<<"Connecting  to server 127.0.0.1 on Port "<<argv[1]<<"."<<endl;
         ptr = atoi(argv[1]);
         Cli = new Client(ptr);
      } else{
         cout<<"Connecting  to server 127.0.0.1 on Port 8000."<<endl;
         Cli = new Client();
      }
   }
   
   ch = new client_handler(Cli);
   ch->sendName(Name,30);
   
       thread t1(myread,ch);
       thread t2(myWrite,ch);
       t2.join();
       exit(0);
   return 0;
}