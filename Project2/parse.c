/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;

instruction parsing_instr(const char *buffer, const int index)  // parse the instruction and return
{
    instruction instr;

    /* chars to save instruction pieces */
    /* Instruction Format (+1 length to put NULL at the end)*/
    char opcode[7];
    char rs[6];
    char rt[6];

    char rd[6];
    char shamt[6];
    char funct[7];

    char imm[17];
    char addr[27];

    memcpy(opcode, buffer, 6);              // get opcode
    opcode[6] = '\0';
    instr.opcode = fromBinary(opcode);

    if (instr.opcode == 0) // if the instuction is R-instruction
    {
        memcpy(rs, buffer + 6, 5);          //fill in rs, rt, rd, shamt, funct
        memcpy(rt, buffer + 11, 5);
        memcpy(rd, buffer + 16, 5);
        memcpy(shamt, buffer + 21, 5);
        memcpy(funct, buffer + 26, 6);
        rs[5] = '\0';
        rt[5] = '\0';
        rd[5] = '\0';
        shamt[5] = '\0';
        funct[6] = '\0';

        instr.r_t.r_i.rs = fromBinary(rs);      // fill in struct
        instr.r_t.r_i.rt = fromBinary(rt);
        instr.r_t.r_i.r_i.r.rd = fromBinary(rd);
        instr.r_t.r_i.r_i.r.shamt = fromBinary(shamt);
        instr.func_code = fromBinary(funct);
    }
    else if (instr.opcode == 2 || instr.opcode == 3) // if the instruction is J-instruction
    {
        memcpy(addr, buffer + 6, 26);       // fill in addresss
        addr[26] = '\0';

        instr.r_t.target = fromBinary(addr);    // fill in struct
    }
    else // if the instruction is I-instruction
    {
        memcpy(rs, buffer + 6, 5);      // fill in rs, rt, imm
        memcpy(rt, buffer + 11, 5);
        memcpy(imm, buffer + 16, 16);
        rs[5] = '\0';
        rt[5] = '\0';
        imm[16] = '\0';

        instr.r_t.r_i.rs = fromBinary(rs);  // fill in struct
        instr.r_t.r_i.rt = fromBinary(rt);
        instr.r_t.r_i.r_i.imm = fromBinary(imm);
    }
    return instr;
}

void parsing_data(const char *buffer, const int index)  // parse data and change the memory
{
    uint32_t addr;
    uint32_t r_addr;
    char s_value[33];
    uint32_t value;

    memcpy(s_value, buffer, 32);    // copy the buffer
    s_value[32] = '\0';
    r_addr = index;
    value = fromBinary(s_value);    // and change it to 32bit unsigned int

    mem_write_32(r_addr + MEM_DATA_START, value);   // then write to memory
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
        printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
        printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

        switch(INST_INFO[i].opcode)
        {
            //Type I
            case 0x9:                //(0x001001)ADDIU
            case 0xc:                //(0x001100)ANDI
            case 0xf:                //(0x001111)LUI        
            case 0xd:                //(0x001101)ORI
            case 0xb:                //(0x001011)SLTIU
            case 0x23:                //(0x100011)LW        
            case 0x2b:                //(0x101011)SW
            case 0x4:                //(0x000100)BEQ
            case 0x5:                //(0x000101)BNE
                printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
                printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
                printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
                break;

                //TYPE R
            case 0x0:                //(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
                printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
                printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
                printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
                printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
                printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
                break;

                //TYPE J
            case 0x2:                //(0x000010)J
            case 0x3:                //(0x000011)JAL
                printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
                break;

            default:
                printf("Not available instruction\n");
                assert(0);
        }
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
        printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
        printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
