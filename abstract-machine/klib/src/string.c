#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

//size_t long unsinged int 4字节或者8字节
size_t strlen(const char *s) {
  size_t len=0;
  while(s[len++]!='\0');
  
  return len-1;
}

char *strcpy(char *dst, const char *src) {
  size_t len=strlen(src);

  size_t i=0;
  while( i<len ){
    dst[i]=src[i];

    i++;
  }
  dst[len]='\0';

  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;

  for(i=0;i<n && src[i]!='\0';i++){       //如果其中没有 '\0' 不会做处理
    dst[i]=src[i];
  }
  for(;i<n;i++){
    dst[i]='\0';
  }

  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t len_dst=strlen(dst);
  size_t i;

  for(i=0;src[i] !='\0';i++){
    dst[len_dst+i]=src[i];
  }
  dst[len_dst+i]='\0';

  return dst;
}


int strcmp(const char *s1, const char *s2) {
  size_t i=0;

  while(true){

    if(s1[i]>s2[i])
        return 1;
    else if(s1[i]<s2[i])
        return -1;    
    else if(s1[i]=='\0'&&s2[i]=='\0')
      return 0;
    
    i++;
  }

}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t  i=0;
  while(i<n){
    if(s1[i]>s2[i])
      return 1;
    else if(s1[i]<s2[i])
      return -1;
    
    i++;
  }
  
  return 0;
}


//在 C 语言中，char 类型可以是有符号或无符号的，这取决于编译器和平台的实现。
//如果我们使用有符号的 char 类型进行内存操作，那么在执行一些位运算时，例如将位设置为 1，可能会导致符号扩展，即将符号位进行拓展。这可能会破坏我们希望在内存中保存的数据。
//在使用 memset 这样的内存操作函数时，最好使用无符号的 unsigned char 类型，以确保不会出现符号扩展或其他类似的问题。
void *memset(void *s, int c, size_t n) {
  unsigned char* p=(unsigned char*)s;
  unsigned char value=(unsigned char) c;

  for(size_t i=0;i<n;i++){
    p[i]=value;
  }

  return s;
}


//与 memcpy 不同的是，memmove 可以正确处理源地址和目标地址重叠的情况。
//因为使用了buffer
void *memmove(void *dst, const void *src, size_t n) {  
  unsigned char* c_dst=(unsigned char*)dst; //需要显示转变
  const unsigned char* c_src=(const unsigned char*)src;
  unsigned char  buffer[n+5];
  size_t true_len=0;

  for(size_t i=0; c_src[i]!='\0'&& i<n;true_len=++i){
    buffer[i]=c_src[i];
  }
  for(size_t i=0;i<true_len;i++){
    c_dst[i]=buffer[i];
  }
  for(size_t i=true_len;i<n;i++){
    c_dst[i]='\0';
  }

  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {//The memory areas must not  overlap
    unsigned char* c_out=(unsigned char*)out; 
    const unsigned char* c_in=(const unsigned char*)in;
    size_t i;

    for(i=0;i<n && c_out[i]!='\0';i++){       //如果其中没有 '\0' 不会做处理
      c_out[i]=c_in[i];
    }
    for(;i<n;i++){
      c_out[i]='\0';
    }


  return c_out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  unsigned char* c_s1=(unsigned char*) s1;
  const unsigned char* c_s2=(unsigned char*) s2;

  size_t  i=0;
  while(i<n){

    if(c_s1[i]>c_s2[i])
        return 1;
    else if(c_s1[i]<c_s2[i])
        return -1;    
    else if(c_s1[i]=='\0'&&c_s2[i]=='\0')
      return 0;
    
    i++;
  }

  return 0;
}

#endif
