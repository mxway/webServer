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
 * 字符串s1是否以s2结束
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
  //socket连接
  if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
    perror("socket.\n");
    exit(1);
  }

  //设置IP端口
  bzero(&addr,sizeof(addr));
  
  addr.sin_family=AF_INET;
  addr.sin_port=htons(PORT);
  addr.sin_addr.s_addr=inet_addr("127.0.0.1");

  //bind
  if(bind(sockfd,(struct sockaddr*)&addr,sizeof(addr))<0){
    perror("bind.\n");
    exit(1);
  }

  //listen 最大连接数
  if(listen(sockfd,10)<0){
    perror("listen.\n");
    exit(1);
  }
  
  printf("listening...\n");
  while(1){
	  //取得客户端的标识
	  if((client=accept(sockfd,(struct sockaddr*)&addr,&len))<0){
		  perror("accept.\n");
		  exit(1);
	  }
	  //读取请求的头部
	  int buf=0;
	  if((buf=read(client,recbuf,2048))>=0){
		  recbuf[buf]='\0';
	  }
    //printf("%s\n",recbuf);
	  char *token=strtok(recbuf," ");
    //获得HTTP请求第一行中间部分，也就是URI
	  if(token!=NULL){
		  token=strtok(NULL," ");
	  }
    char type[256];
    /**
     * 根据请求资源结尾的扩展名不同，
     * 响应不同mine类型
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
	  //读取文件内容
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
	  //返回HTTP响应的头部信息
    //所有的返回状态都是200，暂不做404等其它响应状态的判断工作
	  strcat(sendBuf,"HTTP/1.1 200 OK\r\n");
	  strcat(sendBuf,"Server:MENGXIANLU\r\n");
	  strcat(sendBuf,"Content-Length:");
	  strcat(sendBuf,tmp);
	  strcat(sendBuf,"\r\n");
	  strcat(sendBuf,"Content-Type: ");
    strcat(sendBuf,type);
    strcat(sendBuf,"\r\n");
    //下面极其重要，表示响应头部的结束。
    //头部结束后就是正文部分了
    strcat(sendBuf,"\r\n");
	  /** 响应头部  END */

    int i,j;
    /** 响应内容(正文)  */
    /** 不能使用 strcat进行字符串的连接了。*/
    /**如果请求的是图片，其字节码中有可能包含'\0'如果使用strcat有可能造成信息的截断*/
    for(i=0,j=strlen(sendBuf);i<fileSize;i++,j++){
      sendBuf[j]=content[i];
    }
    /** 一切准备就绪 */
    sendBuf[j]='\0';
    /**结果返回给客户端*/
	  write(client,sendBuf,j);
	  close(client);
  }
  close(sockfd);
  return 0;
}