#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>

typedef struct __node__
{
void *data;
struct __node__ *next;
}node;

typedef struct __linked_list_
{
node *head;
node *tail;
int size;
}linked_list;

typedef struct __queue__
{
linked_list *list;
int size;
}queue;

linked_list *create_list()
{
linked_list *list;
list=(linked_list*)malloc(sizeof(linked_list));
list->head=list->tail=NULL;
list->size=0;
return list;
}

queue * create_queue()
{
queue * q;
q=(queue*)malloc(sizeof(queue));
q->list=create_list();
return q;
}

int is_queue_empty(queue *q)
{
return !q || !q->list || q->list->size==0;
}

void push_on_queue(queue *q,void *data)
{
node *n;
n=(node*)malloc(sizeof(node));
n->data=data;
n->next=NULL;
if(!q->list->head) q->list->head=n;
else q->list->tail->next=n;
q->list->tail=n;
q->list->size++;
}

void * pop_from_queue(queue *q)
{
void *data;
node *n;
data=q->list->head->data;
n=q->list->head;
q->list->head=q->list->head->next;
free(n);
q->list->size--;
return data;
}

void * front_from_queue(queue *q)
{
if(is_queue_empty(q)) return NULL;
return q->list->head->data;
}
void * back_from_queue(queue *q)
{
if(is_queue_empty(q)) return NULL;
return q->list->tail->data;
}

int queue_size(queue *q)
{
if(!q || !q->list) return 0;
return q->list->size;
}

void destroy_queue(queue *q)
{
void *data;
if(!q) return;
while(!is_queue_empty(q))
{
data=pop_from_queue(q);
free(data);
}
free(q->list);
free(q);
}

typedef struct __elements__
{
int field;
char seperator;
}Elements;

typedef struct __cut_elements__
{
char delimiter;
// fields are -f 2-
int contains_all;
// list of fields -f 1,3,4 then list is [1,3,4]
int *fields;
// list of size of the the field list
int field_size;
// list of name of files
char **file_name;
// size of file_name list
int file_count;
}cut_elements;

void destroy(cut_elements *ce)
{
int i;
if(!ce) return;
if(ce->fields) free(ce->fields);
if(ce->file_name)
{
for(i=0;i<ce->file_count;i++) free(ce->file_name[i]);
free(ce->file_name);
}
free(ce);
}

// if user provide -f 3,1,2 then  in that case sorting is required
void insertionSort(int *fields,int size)
{
int y,j,num;
for(y=1;y<size;y++)
{
num=fields[y];
for(j=y-1;j>=0 && fields[j]>num;j--) fields[j+1]=fields[j];
fields[j+1]=num;
}
}

char findString(const char *source,const char *target)
{
const char *e,*f;
e=source;
f=target;
while(*e)
{
if(*e==*f)
{
while(*e && *f && *e==*f)
{
e++;
f++;
}
if(!*f) return 1;
f=target;
}
e++;
}
return 0;
}


int main(int count, char **args)
{
int field,i,j,l,x;
int f_cap;
int *tmp,p;
char *e,*f;
Elements *element,*element1,*element2;
int fd,cs;
char buffer[1025];
int nobe,is_word_completed,wc;
queue *q;
int is_valid;
int reach_to_next_line;
char m;
cut_elements *ce;
ce=(cut_elements*)malloc(sizeof(cut_elements));
if(count<3)
{
printf("Parameters are required\n");
return 0;
}
f_cap=10;
ce->file_count=0;
ce->file_name=(char**)malloc(sizeof(char*)*5);
ce->delimiter=' ';
ce->contains_all=0;
ce->fields=(int *)malloc(sizeof(int)*10);
ce->field_size=0;
q=create_queue();
for(i=1;i<count;i++)
{
l=strlen(args[i]);
if(args[i][0]=='-')
{
if(findString(args[i],"-d") || findString(args[i],"--delimiter"))
{
if(args[i][1]=='d' && args[i][2]) ce->delimiter=args[i][2];
else if(l>2 && args[i][11]) ce->delimiter=args[i][11];
else
{
i++;
l=strlen(args[i]);
if(l>1)
{
printf("tm_cut: delimiter should be a 'single value argument'\n");
destroy(ce);
return 0;
}
ce->delimiter=args[i][0];
}
}
else if(findString(args[i],"-f") || findString(args[i],"--fields"))
{
if(args[i][1]=='f' && args[i][2]) e=&(args[i][2]);
else if(l>2 && args[i][8]) e=&(args[i][8]);
else
{
i++;
e=args[i];
}
// validation
if(*e==',')
{
printf("tm_cut:: fields are numbered from 1\n");
destroy(ce);
return 0;
}
is_valid=1;
f=e;
while(*f)
{
// case [-f 2,,], [-f 2--],[-f --2]
if((*f=='-' && *(f+1)=='-') || (*f==',' && *(f+1)==',') || (*f=='-' && *(f+1)==',') || (*f==',' && *(f+1)=='-'))
{
printf("tm_cut: invalid field value ',' and '-' should not be together\n");
is_valid=0;
break;
}
// case other than ',' '-' [1-9] character
if(*f!=',' && *f!='-' && (*f<48 || *f>57))
{
printf("tm_cut:: invalid field %c\n",*f);
is_valid=0;
break;
}
// case [-f -2-],[-f ,2,]
if(*f=='-' || *f==',') 
{
f++;
while(*f && *f>=48 && *f<=57) f++;
if((*f=='-' || *f==',') && !*(f+1))
{
if(*f=='-') printf("tm_cut:: invalid range\n");
if(*f==',') printf("tm_cut:: field numbered from [1-9]\n");
is_valid=0;
break;
}
f--;
}
f++;
}
if(!is_valid)
{
destroy(ce);
return 0;
}
// validation ends
if(!q) 
{
printf("tm_cut:: out of memorty\n");
destroy(ce);
return 0;
}
while(*e)
{
if(*e>=48 && *e<=57)
{
if(*e==48)
{
printf("tm_cut: field should be start from number [1-9]\n");
destroy(ce);
destroy_queue(q);
return 0;
}
field=0;
for(;*e && *e>=48 && *e<=57;e++) field=field*10+(*e-48);
element=(Elements*)malloc(sizeof(Elements));
element->field=field;
element->seperator='\0';
push_on_queue(q,(void*)element);
}
if(*e=='-' || *e==',')
{
element=(Elements*)malloc(sizeof(Elements));
element->field=0;
element->seperator=*e;
push_on_queue(q,(void*)element);
e++;
}
}
// iterating queue
while(!is_queue_empty(q))
{
element1=(Elements*)pop_from_queue(q);
if(element1->field>0) 
{
if(ce->field_size==f_cap) 
{
f_cap+=10;
tmp=(int*)malloc(sizeof(int)*f_cap);
for(p=0;p<ce->field_size;p++) tmp[p]=ce->fields[p];
free(ce->fields);
ce->fields=tmp;
}
// check for duplicate field
for(p=0;p<ce->field_size;p++) if(ce->fields[p]==element1->field) break;
if(p==ce->field_size) ce->fields[ce->field_size++]=element1->field;
}
else if(element1->seperator=='-')
{
// case -f 2-
if(is_queue_empty(q))
{
// case [-f 2- -f 4-] 
if(ce->contains_all) 
{
printf("tm_cut: only one list may be specified\n");
destroy(ce);
destroy_queue(q);
return 0;
}
else ce->contains_all=1;
break;
}
element2=pop_from_queue(q);
if(ce->field_size==0) x=1;
else x=ce->fields[ce->field_size-1]+1;
for(j=x;j<=element2->field;j++) 
{
if(ce->field_size==f_cap)
{
f_cap+=10;
tmp=(int*)malloc(sizeof(int)*f_cap);
for(p=0;p<ce->field_size;p++) tmp[p]=ce->fields[p];
free(ce->fields);
ce->fields=tmp;
}
ce->fields[ce->field_size++]=j;
}
}
}
} // field else
else
{
printf("tm_cut: invalid option specify\n");
printf("tm_cut : valid options are [-f,-d]\n");
destroy(ce);
return 0;
}
} // else for option ends here
else 
{
// its file name
ce->file_name[ce->file_count]=(char*)malloc(sizeof(l+1));
strcpy(ce->file_name[ce->file_count],args[i]);
ce->file_count++;
}
} // for loop ends here
// sort field value
insertionSort(ce->fields,ce->field_size); 
/*
printf("Field Size: %d\n",ce->field_size);
for(i=0;i<ce->field_size;i++) printf("%d ",ce->fields[i]);
printf("\n");
printf("Contains All Fields: %s\n",ce->contains_all?"true":"false");
printf("File count: %d\n",ce->file_count);
for(i=0;i<ce->file_count;i++) printf("%s ",ce->file_name[i]);
printf("\n");
*/
// part 2 starts extract word and print
for(i=0;i<ce->file_count;i++)
{
fd=open(ce->file_name[i],O_RDONLY);
if(fd<0)
{
printf("Failed to open resource\n");
destroy(ce);
return 0;
}
j=0;
wc=0;
e=NULL;
if(ce->contains_all)
{
while(1)
{
if(!e || !*e)
{
nobe=read(fd,buffer,1024);
if(nobe<=0) break;
buffer[nobe]='\0';
e=buffer;
}
while(*e && *e!='\n' && wc<ce->fields[0])
{
f=e;
while(*e && *e!=ce->delimiter && *e!='\n') e++;
if(f<e) wc++;
if(wc!=ce->fields[0] && *e==ce->delimiter) e++;
}
if(*e=='\n')
{
if(wc==ce->fields[0])
{
for(;f<e;f++) printf("%c",*f);
printf(" \n");
}
else printf("\n");
wc=0;
e++;
continue;
}
while(*e)
{
for(;f<e;f++) printf("%c",*f);
printf(" ");
if(*e=='\n')
{
printf("\n");
e++;
wc=0;
break;
}
e++;
f=e;
while(*e && *e!=ce->delimiter && *e!='\n') e++;
}
}// infinite loop ends here
}// contains all field ends here
else // seperate fields
{
i=0;
reach_to_next_line=0;
while(1)
{
if(!e || !*e)
{
nobe=read(fd,buffer,1024);
if(nobe<=0) break;
buffer[nobe]='\0';
e=buffer;
}
if(reach_to_next_line)
{
while(*e && *e!='\n') e++;
if(!*e) continue;
}
while(*e)
{
f=e;
while(*e && *e!=ce->delimiter && *e!='\n') e++;
if(f<e) 
{
wc++;
if(wc==ce->fields[i])
{
for(;f<e;f++) printf("%c",*f);
printf(" ");
i++;
if(i==ce->field_size)
{
while(*e &&*e!='\n') e++;
if(!*e) reach_to_next_line=1;
}
}
if(*e=='\n')
{
printf("\n");
wc=0;
i=0;
}
}
if(*e) e++;
}
if(reach_to_next_line)
{
m=*(e-1);
if((m>=65 && m<=91)|| (m>=91 && m<=122)) wc--;
}
}
} // separete fileds else ends here
cs=close(fd);
if(cs) printf("tm_cut:: failed to close resource\n");
} // for loop
return 0;
}
