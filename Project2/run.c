/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
    unsigned char rs;   // chars to save register
    unsigned char rt;
    unsigned char rd;
    unsigned char shamt;
    short imm;
    uint32_t target;

    instruction* instr = get_inst_info(CURRENT_STATE.PC);       // get info of PC

    CURRENT_STATE.PC += 4;                                      // fetch the PC to PC+4

    if (CURRENT_STATE.PC >= MEM_TEXT_START + NUM_INST * 4)      // if the PC is over the instruction, stop
            RUN_BIT = FALSE;

    if (instr->opcode == 0x00) // if the instruction is R-type
    {
        rs = RS(instr);         // bring the information of the istroction
        rt = RT(instr);
        rd = RD(instr);
        shamt = SHAMT(instr);

        if (instr->func_code == 0x21) // addu
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
        else if (instr->func_code == 0x24) // and
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
        else if (instr->func_code == 0x08) // jr
            CURRENT_STATE.PC = CURRENT_STATE.REGS[rs];
        else if (instr->func_code == 0x27) // nor
            CURRENT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
        else if (instr->func_code == 0x25) // or
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
        else if (instr->func_code == 0x2b) // sltu
            CURRENT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt])? 1 : 0;
        else if (instr->func_code == 0x00) // sll
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
        else if (instr->func_code == 0x02) // srl
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
        else if (instr->func_code == 0x23) // subu
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
        //else{} // Error
    }
    else if ((instr->opcode == 0x02) || (instr->opcode == 0x03)) // if the instruction is J-type
    {
        target = TARGET(instr);

        if (instr->opcode == 0x02) // j
            CURRENT_STATE.PC = (target << 2);
        else if (instr->opcode == 0x03) // jal
        {
            CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
            CURRENT_STATE.PC = (target << 2);
        }
        //else{} // Error
    }
    else // if the instruction is I-type
    {
        rs = RS(instr);
        rt = RT(instr);
        imm = IMM(instr);

        if (instr->opcode == 0x09) // addiu
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + SIGN_EX(imm);
        else if (instr->opcode == 0x0c) // andi
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & (0 | imm);
        else if (instr->opcode == 0x04) // beq
        {
            if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt])
                CURRENT_STATE.PC += (SIGN_EX(imm) << 2);
        }
        else if (instr->opcode == 0x05) // bne
        {
            if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt])
                CURRENT_STATE.PC += (SIGN_EX(imm) << 2);
        }
        else if (instr->opcode == 0x0f) // lui
            CURRENT_STATE.REGS[rt] = (imm << 16);
        else if (instr->opcode == 0x23) // lw
            CURRENT_STATE.REGS[rt] = mem_read_32(CURRENT_STATE.REGS[rs] + SIGN_EX(imm));
        else if (instr->opcode == 0x0d) // ori
            CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | imm;
        else if (instr->opcode == 0x0b) // sltiu
            CURRENT_STATE.REGS[rt] = (CURRENT_STATE.REGS[rs] < SIGN_EX(imm)) ? 1 : 0;
        else if (instr->opcode == 0x2b) // sw
            mem_write_32(CURRENT_STATE.REGS[rs] + SIGN_EX(imm), CURRENT_STATE.REGS[rt]);
        //else{} // Error
    }
}
