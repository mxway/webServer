#include<stdio.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<stdlib.h>

#define PORT 12345
#define WebRoot /home/meng/webpage
/**
 * 
 * �ַ���s1�Ƿ���s2����
 */
int endsWith(char s1[],char s2[]){
  int len1 = strlen(s1);
  int len2 = strlen(s2);
  int i=len1-1,j=len2-1;
  if(len1<len2)return 0;
  for(;i>=0&&j>=0;i--,j--){
    if(s1[i]!=s2[j])return 0;
  }
  return 1;
}
int main(){
  int sockfd,fd,client;
  struct sockaddr_in addr;
  int len=sizeof(addr);
  char recbuf[2048];
  //socket����
  if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
    perror("socket.\n");
    exit(1);
  }

  //����IP�˿�
  bzero(&addr,sizeof(addr));
  
  addr.sin_family=AF_INET;
  addr.sin_port=htons(PORT);
  addr.sin_addr.s_addr=inet_addr("127.0.0.1");

  //bind
  if(bind(sockfd,(struct sockaddr*)&addr,sizeof(addr))<0){
    perror("bind.\n");
    exit(1);
  }

  //listen ���������
  if(listen(sockfd,10)<0){
    perror("listen.\n");
    exit(1);
  }
  
  printf("listening...\n");
  while(1){
	  //ȡ�ÿͻ��˵ı�ʶ
	  if((client=accept(sockfd,(struct sockaddr*)&addr,&len))<0){
		  perror("accept.\n");
		  exit(1);
	  }
	  //��ȡ�����ͷ��
	  int buf=0;
	  if((buf=read(client,recbuf,2048))>=0){
		  recbuf[buf]='\0';
	  }
    //printf("%s\n",recbuf);
	  char *token=strtok(recbuf," ");
    //���HTTP�����һ���м䲿�֣�Ҳ����URI
	  if(token!=NULL){
		  token=strtok(NULL," ");
	  }
    char type[256];
    /**
     * ����������Դ��β����չ����ͬ��
     * ��Ӧ��ͬmine����
     */
    if(endsWith(token,".jpg")){
      strcpy(type,"image/jpeg\r\nAccept-Ranges:bytes");
    }else if(endsWith(token,".gif")){
      strcpy(type,"image/gif");
    }else if(endsWith(token,".js")){
      strcpy(type,"text/javascript");
    }else if(endsWith(token,".css")){
      strcpy(type,"text/css");
    }else{
      strcpy(type,"text/html");
    }
	  //��ȡ�ļ�����
	  char filePath[1024];
	  char content[2048*512];
	  int fileSize;
	  strcpy(filePath,"/home/meng/webpage");
	  strcat(filePath,token);
	  fd=open(filePath,O_RDONLY,0766);
    if(fd==-1){
	   strcpy(content,"Not Found");
	  }else{
		  fileSize=read(fd,content,sizeof(content));
      //printf("%d\n",strlen(content));
      content[fileSize]='\0';
		  if(fileSize<=0)strcpy(content,"Not Found");
		  close(fd);
	  }

	  if(fileSize<=0){
		  fileSize=strlen("Not Found");
	  }
	  char sendBuf[2048*1024];
	  char tmp[20];
	  sprintf(tmp,"%d",fileSize);
	  memset(sendBuf,'\0',sizeof(sendBuf));
	  //����HTTP��Ӧ��ͷ����Ϣ
    //���еķ���״̬����200���ݲ���404��������Ӧ״̬���жϹ���
	  strcat(sendBuf,"HTTP/1.1 200 OK\r\n");
	  strcat(sendBuf,"Server:MENGXIANLU\r\n");
	  strcat(sendBuf,"Content-Length:");
	  strcat(sendBuf,tmp);
	  strcat(sendBuf,"\r\n");
	  strcat(sendBuf,"Content-Type: ");
    strcat(sendBuf,type);
    strcat(sendBuf,"\r\n");
    //���漫����Ҫ����ʾ��Ӧͷ���Ľ�����
    //ͷ��������������Ĳ�����
    strcat(sendBuf,"\r\n");
	  /** ��Ӧͷ��  END */

    int i,j;
    /** ��Ӧ����(����)  */
    /** ����ʹ�� strcat�����ַ����������ˡ�*/
    /**����������ͼƬ�����ֽ������п��ܰ���'\0'���ʹ��strcat�п��������Ϣ�Ľض�*/
    for(i=0,j=strlen(sendBuf);i<fileSize;i++,j++){
      sendBuf[j]=content[i];
    }
    /** һ��׼������ */
    sendBuf[j]='\0';
    /**������ظ��ͻ���*/
	  write(client,sendBuf,j);
	  close(client);
  }
  close(sockfd);
  return 0;
}