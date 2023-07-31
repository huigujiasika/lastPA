#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
//TDOO:: 未测试
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
char* int2str(int d,char* s);
#define BUFFER 128

int printf(const char *fmt, ...) {
    char buffer[BUFFER];

    va_list args;  //可变参数列表
    va_start(args,fmt);
    int len=vsprintf(buffer,fmt,args);
    va_end(args);

    for(int i=0;i<len;i++)
      putch(buffer[i]);

    return len;
}




void sprint_format(char** pout, const char** pin, va_list args) {      
  // 此处为了返回指针的值 使用双重指针
  char* s,buff[50];
  size_t len;
  int d;
  
  switch (**pin){             //TDO:: 两个代码目前有冗余
    case  'd':               
      (*pin)++;
      d=va_arg(args,int);

      assert(d==1);

      s=int2str(d,buff);
      len=strlen(s);

      assert(len==2);

      strcpy(*pout,s);
      (*pout)+=len;   
      break;

    case  's':
      (*pin)++;
      s=va_arg(args,char*);

      len=strlen(s);
      strcpy(*pout,s);
      (*pout)+=len;   //继续来到空位置
      break;
  }

}


//they  are  called  with  a va_list  instead  of  a  variable number of arguments.
// These functions do not call the va_end macro.
//Because they invoke the va_arg macro, the value of ap is undefined after the call.
int vsprintf(char *out, const char *fmt, va_list ap) {
  char* pout=out;
  const char* pin=fmt;

  while (*pin){
    switch(*pin){
      case '%':
        pin++;
        sprint_format(&pout,&pin,ap);  //TODO:: va_list* args这里有问题但是现在好像能跑
        break;
      
      default:
        *pout=*pin;
        pin++;
        pout++;       

      }
  }

  *pout='\0';
  
  return pout-out;
}


int sprintf(char *out, const char *fmt, ...) {

  va_list args;  //可变参数列表
  va_start(args,fmt);
  int len=vsprintf(out,fmt,args);
  va_end(args);

  return len;
}



int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}


// va_start(ap, end);  end必须为最后一个知道类型的参数 




char* int2str(int d,char* s){
  const char hex_char[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  int i=10;
  s[i--]='\0';
  
  if(d==0){
    s[i--]='0';
    i++;
    return &s[i];
  }
  while(d){
    s[i--]=hex_char[d%10];
    d/=10;

  }

  i++;   //多往前了一个
  return &s[i];
}


#endif

 
