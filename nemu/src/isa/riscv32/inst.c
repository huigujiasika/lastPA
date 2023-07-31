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

#include "local-include/reg.h"
#include <common.h>
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S,TYPE_J,TYPE_RR,TYPE_B,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; }   while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 30, 21) << 1) | (BITS(i, 20, 20) << 11) | (BITS(i, 19, 12) << 12); } while(0)
#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1); } while(0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_RR: src1R(); src2R();        break;  //  用RR 代指 R
    case TYPE_B:  src1R(); src2R();immB(); break;
  }
}

static int decode_exec(Decode *s) {
  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  //TODO:: 此处所有考虑的 pc 都是未自增的
  //TODO:: hello-str 到最后有问题

  INSTPAT_START();      //TODO:: 测试当程序写错了会怎么样

  //ret 伪指令 可以由 jalr 扩展
  //中文写的是 010 实际应该是000
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr  , I, R(dest) =s->pc+4;  s->dnpc=src1+imm );
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal   , J, R(dest) = s->pc + 4; s->dnpc = s->pc + imm  );
  //[20] 表示最高位，用于指示跳转目标地址的符号位
  //[10:1] 表示第 2 到第 11 位，用于指定目标地址的低 10 位（即偏移量的低 10 位）。
  //[11] 表示第 12 位，用于指示目标地址的第 12 位，即偏移量的第 11 位。该位的值必须与立即数的符号位相同。
  //[19:12] 表示第 13 到第 20 位，用于指定目标地址的高 8 位（即偏移量的高 8 位），且必须与符号位和第 12 位相同。

  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne  , B,  if(src1!=src2) s->dnpc=s->pc+imm);
  // beqz 伪指令 src1==0 跳转 可用 beq 扩展
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq   ,B,  if(src1==src2) s->dnpc=s->pc+imm );
  //blez 伪指令  rs2<=0 bge 扩展
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge   ,B,   if((sword_t)src1>=(sword_t)src2) s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu  ,B,  if(src1>=src2) s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt   ,B,  if((sword_t)src1<(sword_t)src2)  s->dnpc=s->pc+imm );
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu  ,B,  if(src1<src2)  s->dnpc=s->pc+imm );

  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) =imm+s->pc );
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm  );  // 将立即数放入寄存器
   
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai    , I, R(dest)=(sword_t)src1>>(imm & 0x1f)   );   // 仅当 shamt[5]=0 时指令有效 所以用掩码保留低五位 转成有符号数是为了算数移位
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , RR, R(dest)=(sword_t)src1>>src2   );  // R(dest)=(sword_t)src1>>(src2 & 0x1f)   ); 原来写的
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , RR, R(dest)=src1>>src2    );
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , RR, R(dest)=src1<<src2  );
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli    , I, R(dest)=src1<<(imm & 0x1f)   );
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli    , I, R(dest)=src1>>(imm & 0x1f)   );

  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor     ,RR, R(dest) = src1 ^ src2 );  
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori    , I, R(dest) = src1 ^ imm );
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or      ,RR, R(dest) = src1 | src2 );  
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and     ,RR, R(dest) = src1 & src2 );  


  // li 伪指令 可以由 addi 扩展
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi    ,I,  R(dest)=src1+imm  );
  //INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add     ,RR, R(dest)=src1+src2  ); // 寄存器 rs1+rs2  忽略算数溢出
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi    ,I, R(dest)=src1 & imm  ); 
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub     ,RR, R(dest)=src1-src2  ); // 寄存器 rs1+rs2  忽略算数溢出

  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul     ,RR, R(dest)=src1 * src2  );
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh    ,RR, R(dest) = (SEXT(src1, 32) * SEXT(src2, 32)) >> 32);  //高位指最高位  signed×signed
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulu    ,RR, R(dest) = ((uint64_t)src1 * src2) >> 32);  //高位指最高位  signed×signed
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div     ,RR, R(dest)=src1 / (sword_t)src2  ); 
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu    ,RR, R(dest)=src1 / src2  ); 
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem     ,RR, R(dest)=src1 % (sword_t)src2  );   // 有符号数求余

  // 伪指令seqz 被扩展为 sltiu rd, rs1, 1
  // seqz 如果 x[rsi]==0 ? 1 ：0
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu   ,I,  R(dest) = src1<imm ? 1:0   );
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu    ,RR, R(dest) = src1<src2 ? 1:0  );
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    ,RR,  R(dest) = (sword_t)src1<(sword_t)src2 ? 1:0  );

  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = SEXT(Mr(src1 + imm, 4), 32));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(dest) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(dest) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(dest) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2)); //取半字
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));


  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));  //非法指令
  INSTPAT_END();

  //INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
  //0表示相应的位只能匹配0 1表示相应的位只能匹配1   ?表示相应的位可以匹配0或1

  R(0) = 0; // reset $zero to 0

  return 0;
}



int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  IFDEF(CONFIG_ITRACE, trace_inst(s->pc, s->isa.inst.val));  //收集环形指令

  return decode_exec(s);
}
