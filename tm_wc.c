#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

int main(int count, char **parameters)
{
int fd,cs,byte_extracted;
int lines,words,bytes;
int tLines,tWords,tBytes;
int i,j;
char buffer[1025];
char *file_name,l,w,c;
char *e;
char **file_names,**tmp;
int file_count,capacity;
file_count=0;
capacity=10;
file_names=(char**)malloc(sizeof(char*)*capacity);
l=w=c=0;
// Parse command line arguments
for(i=1;i<count;i++)
{
if(parameters[i][0]=='-')
{
if(parameters[i][1]=='l' || strcmp(parameters[i],"--lines")==0) l=1;
if(parameters[i][1]=='w' || strcmp(parameters[i],"--words")==0) w=1;
if(parameters[i][1]=='c' || strcmp(parameters[i],"--bytes")==0) c=1;
}
else 
{
file_names[file_count]=(char*)malloc(sizeof(char)*(strlen(parameters[i])+1));
strcpy(file_names[file_count],parameters[i]);
file_count++;
if(file_count==capacity)
{
capacity+=5;
tmp=(char**)malloc(sizeof(char*)*capacity);
for(j=0;j<file_count;j++)
{
tmp[j]=(char*)malloc(sizeof(char)*(strlen(file_names[j])+1));
strcpy(tmp[j],file_names[j]);
}
for(j=0;j<file_count;j++) free(file_names[j]);
free(file_names);
file_names=tmp;
}
}
}
if(!l && !w && !c) l=w=c=1;
tLines=tWords=tBytes=0;
for(i=0;i<file_count;i++)
{
lines=words=bytes=0;
e=NULL;
fd=open(file_names[i],O_RDONLY);
if(fd==-1)
{
printf("tm_wc:: %s file not found, error number %d\n",file_names[i],errno);
for(j=0;j<file_count;j++) free(file_names[j]);
free(file_names);
return 0;
}
while(1)
{
if(!e || !*e)
{
byte_extracted=read(fd,buffer,1024);
if(byte_extracted<=0) break;
buffer[byte_extracted]='\0';
e=buffer;
}
while(*e)
{
if(*e=='\n') lines++;
if(*e==' ' || *e=='\n') words++;
bytes++;
e++;
}
}
tLines+=lines;
tWords+=words;
tBytes+=bytes;
if(l) printf(" %d",lines);
if(w) printf(" %d",words);
if(c) printf(" %d",bytes);
printf(" %s\n",file_names[i]);
cs=close(fd);
if(cs) printf("Failed to close file %s, error number %d\n", file_names[i],errno);
}
if(l) printf(" %d",tLines);
if(w) printf(" %d",tWords);
if(c) printf(" %d",tBytes);
printf(" total\n");
for(j=0;j<file_count;j++) free(file_names[j]);
free(file_names);
return 0;
}
