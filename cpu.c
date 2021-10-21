enum program_state {
    READY,
    RUNNING,
    BLOCKED
};

struct PCB {
	int PID;
	int state;
	int PID[MAXSIZE];
    int priority;
    int state;
    char* recv;
    char* proc_message;
};



/*
#include<stdio.h>
#include<malloc.h>
#include<string.h>
#define N 10  //定义最大进程数
#define TIME 2//定义时间片大小
typedef struct pcb{
    char id[10];//进程标识数
    int arrivetime;//到达时间
    int runtime;//进程已经占用的cpu时间
    int needtime;//进程还需要的时间
    char state[12];//进程运行状态：wait or runing
    struct pcb *next;
}pcb,*PCB;
PCB head;//设置全局变量用来修改就绪队列
PCB tail;
int count=0;//记录就绪队列中进程数
void CreatProcess(){
    //创建进程
    PCB p,q;//进程的头尾指针都有
    int num;//记录要创建的进程数
    int i,j;
    int arrive[N];
    head=tail=(PCB)malloc(sizeof(pcb));
    head->next=NULL;
    p=head;
    printf("输入你要创建的进程数:");
    scanf("%d",&num);
    count=num;
    printf("********按照进程到达时间从小到大创建就绪队列******\n");
    //初始对其排序来创建就绪队列
    for(i=1;i<=num;i++){
        p->next=(PCB)malloc(sizeof(pcb));
        p=p->next;
        tail=p;
        printf("输入进程%d的标示符:",i);
        scanf("%s",p->id);
        printf("输入进程%d的到达时间：",i);
        scanf("%d",&p->arrivetime);
        printf("输入进程%d已占用的cpu时间：",i);
        scanf("%d",&p->runtime);
        printf("输入进程%d还需要的cpu时间：",i);
        scanf("%d",&p->needtime);
        printf("输入进程%d当前状态:(run 或者wait):",i);
        scanf("%s",p->state);
    }
    tail->next=p->next=NULL;
}
void RR_RunProcess(){
    //运行进程,简单轮转法Round Robin
    PCB p,q,temp;
    p=head->next;
    while(1){
    if(head->next==NULL)
    {
        printf("此时就绪队列中已无进程!\n");
            return ;
    }
    else
    {
        while(p){
            if((p->needtime>0)&&!(strcmp(p->state,"wait"))){
                printf("进程%s开始,\n",p->id );
                strcpy(p->state,"run");
                p->runtime+=TIME;
                p->needtime-=TIME; 
                if(p->needtime<0)
                    p->needtime=0;
            }
            temp=p;//把该时间片内运行完的进程存到临时temp中
            //把temp接到链表尾部,销毁P；
            if(temp->needtime>0){//把该时间片内运行完的进程接到就绪队列的尾部
                if(count>1){
                head->next=temp->next;
                tail->next=temp;
                tail=tail->next;
                strcpy(tail->state,"wait");
                tail->next=NULL;
                }
                else if(count==1){//当只有一个进程等待时，分开讨论
                    head->next=temp;
                    tail=temp;
                    strcpy(tail->state,"wait");
                    tail->next=NULL;
 
                }
                 
            }
            if(temp->needtime==0){//销毁就绪队列中已经结束的进程
                count--;//此时就绪队列中进程数减1
                printf("进程%s结束.\n",p->id);
                head->next=temp->next;
                free(temp);//撤销就绪队列中已经结束的进程
 
            }
 
            p=head->next;
 
        }
 
    }
    }
}
void main(){
    printf("**************进程的初始状态!**************\n");
    CreatProcess();
    printf("*******************************************\n\t\t程序运行结果如下:\n\n");
    printf("*******************************************\n");
    RR_RunProcess();//简单轮转法Round Robin
 
}
*/