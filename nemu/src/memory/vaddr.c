/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>
#include <common.h>

char* readmtrace_address="readmtrace.txt";
char* writemtrace_address="writemtrace.txt";
#define MTRACE 1
//TODO:: define逻辑需要修改 借助menuconfig

word_t vaddr_ifetch(vaddr_t addr, int len) {
  IFDEF(MTRACE,read_mtrace(addr,len ,readmtrace_address));
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  IFDEF(MTRACE,read_mtrace(addr,len ,readmtrace_address));
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  IFDEF(MTRACE,write_mtrace(addr,len,data ,writemtrace_address));
  paddr_write(addr, len, data);
}
