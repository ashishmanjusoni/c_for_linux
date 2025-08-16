#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>

int main(int count,char **parameters)
{
int result;
int i;
if(count<2) 
{
printf("Pass directory name\n");
return 0;
}
for(i=1;i<count;i++)
{

result=mkdir(parameters[i],S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
if(result==-1) printf("Unable to create directory [%s], error number [%d]\n",parameters[i],errno);
}
return 0;
}
