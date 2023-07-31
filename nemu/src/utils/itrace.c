#include <stdio.h>
#include <common.h>

#define MAX_LENGTH_ITRACE 10    //最大显示条数

typedef struct itrace
{
    word_t pc;
    uint32_t inst;
    char asm_inst[20];
}ItraceNode;

ItraceNode iringbuf[MAX_LENGTH_ITRACE];

int cur=0;
bool full=false;

void trace_inst(word_t pc, uint32_t inst){   // 初始化 irring_buf

    iringbuf[cur].pc=pc;           //cur的位置是等待填充位置
    iringbuf[cur].inst=inst;

    if(!full){       
        if(cur==MAX_LENGTH_ITRACE){
            cur=0;
            full=true;
        }else
            cur++;
    }else{
        cur++;
        cur= cur%MAX_LENGTH_ITRACE;  
    }
      

}


void display_iringbuf(){         //iringbuf

    if(!full&&!cur) return;

    void disassemble(char *str, int size,
               uint64_t pc, uint8_t *code, int nbyte);

    char buf[128];
    char *p;
    int end = cur;
    int i = full?cur:0;

    printf("\nMost recently executed instructions\n\n");
    do{
        p=buf;
        uint8_t* inst=(uint8_t *)&iringbuf[i].inst;

        p+=sprintf(buf,"%s" FMT_WORD ":",(i+1)%MAX_LENGTH_ITRACE==end?" --> ":"     ",iringbuf[i].pc);
        int ilen=4;                                 //此处确定使用riscv32
        for (int j = ilen - 1; j >= 0; j --) 
            p += sprintf(p, " %02x", inst[j]);
        p += sprintf(p, "   ");
        
        //p += sprintf(buf, "%s" FMT_WORD ": %08x ", (i+1)%MAX_LENGTH_ITRACE==end?" --> ":"     ", iringbuf[i].pc, iringbuf[i].inst);
        
        IFDEF(CONFIG_ITRACE,disassemble(p, buf+sizeof(buf)-p, iringbuf[i].pc, (uint8_t *)&iringbuf[i].inst, 4));
        
        if ((i+1)%MAX_LENGTH_ITRACE==end) printf(ANSI_FG_RED);  //打印红色
        puts(buf);

    }while( (i = (i+1)%MAX_LENGTH_ITRACE) != end );
    puts(ANSI_NONE);         //终止红色

}



//Mtrace
void read_mtrace(vaddr_t addr, int len,char* readmtrace_address){   
    printf("READ at " FMT_PADDR " len=%d\n", addr, len);

    // FILE* fp=fopen(readmtrace_address,"w+");
    // fprintf(fp,"\n\n\nBEGIN MEMORy:\n");
    // fprintf(fp,"pread at " FMT_PADDR " len=%d\n", addr, len);
    // fclose(fp);

}

void write_mtrace(vaddr_t addr,int len,word_t data,char* writemtrace_address){
    printf("WRITE at " FMT_PADDR " len=%d, data=" FMT_WORD "\n", addr, len, data);
    // FILE* fp=fopen(writemtrace_address,"w+");
    // fprintf(fp,"\n\n\nBEGIN MEMORy:\n");
    // fprintf(fp,"pwrite at " FMT_PADDR " len=%d, data=" FMT_WORD "\n", addr, len, data);
    // fclose(fp);

}
