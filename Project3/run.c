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
instruction* get_inst_info(uint32_t pc) {
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
    int i = 0;		// Use in iteration 
    int count = 0;	// Number of empty pipe
    short DEBUG = TRUE;

    FETCH_BIT = 4; // Unit of address increase/decrease
    
    if (CYCLE_COUNT)
        CURRENT_STATE.PC -= FETCH_BIT;

    CPU_State BUF; // Next CPU State
    memcpy(&BUF, &CURRENT_STATE, sizeof(CPU_State)); // BUF = CURRENT_STATE

    instruction* ID_EX_inst;	// Instruction in ID_EX Register
    instruction* EX_MEM_inst;	// Instruction in EX_MEM Register

    if (DEBUG)
    {
        printf("CURRENT_PIPE: ");
        for (i = 0; i < 5; ++i)
            printf("%x ", BUF.PIPE[i]);
        printf("\n");
    }
    
    /* empty pipe counting */    
    for (i = 0 ; i < 5 ; i ++)
        if (CURRENT_STATE.PIPE[i] == 0)
            count++;

    /* Check address over */
    if ((CURRENT_STATE.PC + FETCH_BIT >= MEM_TEXT_START + NUM_INST * 4)) {
        BUF.RUN++;
        if (DEBUG) printf("RUN COUNTING...\n");
    }
    if (BUF.RUN != 0)
        BUF.RUN++;

    /* RUN_BIT check */
    if ((BUF.RUN >= 5) && (count >= 3) && (CURRENT_STATE.PIPE[MEM_STAGE] != 0))
        RUN_BIT = FALSE;

    if (DEBUG) {
	printf("++++++ CYCLE COUNT: %lu ++++++\n", CYCLE_COUNT + 1);
	printf("BR_TAKE: %d, %d\n", CURRENT_STATE.EX_MEM_BR_TAKE, CURRENT_STATE.MEM_WB_BR_TAKE);
        printf("EX_MEM_FETCH: %d, %d, 0x%x\n", CURRENT_STATE.EX_MEM_WRITE, CURRENT_STATE.EX_MEM_FORWARD_REG,  CURRENT_STATE.EX_MEM_FORWARD_VALUE);
        printf("MEM_WB_FETCH1: %d, %d, 0x%x\n", CURRENT_STATE.MEM_WB_WRITE, CURRENT_STATE.MEM_WB_FORWARD_REG1, CURRENT_STATE.MEM_WB_FORWARD_VALUE1);
        printf("MEM_WB_FETCH2: %d, %d, 0x%x\n", CURRENT_STATE.MEM_WB_WRITE, CURRENT_STATE.MEM_WB_FORWARD_REG2, CURRENT_STATE.MEM_WB_FORWARD_VALUE2);
    }

    /* EX FORWARD */
    if  (CURRENT_STATE.EX_MEM_WRITE && (CURRENT_STATE.EX_MEM_FORWARD_REG != 0)
            && (CURRENT_STATE.PIPE[ID_STAGE] != 0) && (CURRENT_STATE.PIPE_STALL[ID_STAGE] == 0)) {

        if (DEBUG) printf("F1) REG1: %u, REG2: %u\n", RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])), RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])));

        if (CURRENT_STATE.EX_MEM_FORWARD_REG == RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))) {
            CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
            if (DEBUG) printf("fetch_ex_1\n");
        }
        if (CURRENT_STATE.EX_MEM_FORWARD_REG == RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))) {
            if (DEBUG) printf("fetch_ex_2\n");
            CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
        }
    }

    /* MEM FORWARD */
    if (CURRENT_STATE.MEM_WB_WRITE && (CURRENT_STATE.MEM_WB_FORWARD_REG1 != 0)
            && (CURRENT_STATE.PIPE[ID_STAGE] != 0) && (CURRENT_STATE.PIPE_STALL[ID_STAGE] == 0)) {
        if (DEBUG) printf("F2) REG1: %u, REG2: %u\n", RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])), RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])));
        if ((CURRENT_STATE.EX_MEM_FORWARD_REG != RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
                && (CURRENT_STATE.MEM_WB_FORWARD_REG1
                    == RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))) {
            if (DEBUG) printf("fetch_mem_1\n");
            CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE1;
        }
        if ((CURRENT_STATE.EX_MEM_FORWARD_REG != RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
                && (CURRENT_STATE.MEM_WB_FORWARD_REG1
                    == RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))) {
            CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE1;
            if (DEBUG) printf("fetch_mem_2\n");
        }
    }

    if (CURRENT_STATE.MEM_WB_WRITE && (CURRENT_STATE.MEM_WB_FORWARD_REG2 != 0)
            && (CURRENT_STATE.MEM_WB_FORWARD_REG2 != CURRENT_STATE.MEM_WB_FORWARD_REG1)
            && (CURRENT_STATE.PIPE[ID_STAGE] != 0) && (CURRENT_STATE.PIPE_STALL[ID_STAGE] == 0)) {
	if (DEBUG) printf("F2) REG1: %u, REG2: %u\n", RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])), RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])));
        if ((CURRENT_STATE.EX_MEM_FORWARD_REG != RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
                && (CURRENT_STATE.MEM_WB_FORWARD_REG2
                    == RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))) {
            if (DEBUG) printf("fetch_mem_1\n");
            CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE2;
        }
        if ((CURRENT_STATE.EX_MEM_FORWARD_REG != RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
                && (CURRENT_STATE.MEM_WB_FORWARD_REG2
                    == RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))) {
            CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE2;
            if (DEBUG) printf("fetch_mem_2\n");
        }
    }

    /* ID STAGE */
    BUF.PIPE[ID_STAGE] = CURRENT_STATE.PIPE[IF_STAGE];
    if (CURRENT_STATE.PIPE_STALL[IF_STAGE] == 0 && CURRENT_STATE.PIPE[IF_STAGE]) {
        if (DEBUG) printf("----STAGE: ID----\n");

        ID_EX_inst = get_inst_info(CURRENT_STATE.PIPE[IF_STAGE]);   // Instruction transmission
        BUF.ID_EX_NPC = CURRENT_STATE.PIPE[IF_STAGE];		    // NEXTPC transmission

	/* If STALL_CYCLE on, PUSH stall */
        if (CURRENT_STATE.STALL_CYCLE != 0) {
            ID_EX_inst = get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]);
            BUF.ID_EX_NPC = CURRENT_STATE.PIPE[ID_STAGE];
            CURRENT_STATE.PIPE_STALL[ID_STAGE] = 1;
        }

        BUF.ID_EX_REG1 = CURRENT_STATE.REGS[RS(ID_EX_inst)];	// REG1 = inst.rs
        BUF.ID_EX_REG2 = CURRENT_STATE.REGS[RT(ID_EX_inst)];	// REG2 = inst.rt
        BUF.ID_EX_DEST = RD(ID_EX_inst);			// DEST = inst.rd
        BUF.ID_EX_SHAMT = SHAMT(ID_EX_inst);			// save shamt

	// Load-use process
        if (CURRENT_STATE.PIPE[ID_STAGE] != 0) {
            if (OPCODE(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])) == 0x23) {
                if((RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])) == RS(ID_EX_inst))
                        || (OPCODE(ID_EX_inst) == 0
                            && RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])) == RT(ID_EX_inst))) {
                    BUF.STALL_CYCLE = 2;
                    if (DEBUG) printf("STALL_CYCLE\n");
                }
            }
        }
	
	switch (OPCODE(ID_EX_inst))
        {
	    /* Jump Insturctions (J-type and JR(R)) */
            case 0x3:        // JAL
                BUF.REGS[31] = CURRENT_STATE.IF_ID_NPC + 8; // Save address in $31
                if (DEBUG) printf("JAL INST: 0x%x\n", BUF.NEXTPC);
            case 0x2:        // J
                BUF.JUMP_PC = 2;			    // Jump flag
                BUF.NEXTPC = (TARGET(ID_EX_inst) << 2);	    // Address to jump
                if (DEBUG) printf("J INST: 0x%x\n", BUF.NEXTPC);
                break;
            case 0x0:        // R-type
                if (FUNC(ID_EX_inst) == 0x8) // JR
                {
                    BUF.JUMP_PC = 2;					// Jump flag
                    BUF.NEXTPC = CURRENT_STATE.REGS[RS(ID_EX_inst)];	// Address to jump
                    if (DEBUG) printf("JR INST: 0x%x\n", BUF.NEXTPC);
                }
                break;
            // I-type
            case 0x09:        // ADDIU
            case 0x0c:        // ANDI
            case 0x0f:        // LUI
            case 0x23:        // LW
            case 0x0d:        // ORI
            case 0x0b:        // SLTIU
            case 0x2b:        // SW
            case 0x04:        // BEQ
            case 0x05:        // BNE
                BUF.ID_EX_DEST = RT(ID_EX_inst); // DEST = RT in I-type
                break;
        }
	/* Only ANDI and ORI use ZEROIMM, and rest use SIGNIMM */
        if ((OPCODE(ID_EX_inst) != 0xc) && (OPCODE(ID_EX_inst) != 0xd))
            BUF.ID_EX_IMM = SIGN_EX(IMM(ID_EX_inst));
        else
            BUF.ID_EX_IMM = (0 | IMM(ID_EX_inst));

        if (DEBUG) printf("ID: DEST: %d REG1: %u, REG2: %u\n", BUF.ID_EX_DEST, RS(ID_EX_inst), RT(ID_EX_inst));
        if (DEBUG) printf("VALUE: REG1: 0x%x, REG2: 0x%x, RD: %d\n",  BUF.ID_EX_REG1, BUF.ID_EX_REG2, RD(ID_EX_inst));
        if (DEBUG) printf("ID: ID_EX_IMM: 0x%x\n", BUF.ID_EX_IMM);
    }

    /* EX STAGE */
    BUF.PIPE[EX_STAGE] = CURRENT_STATE.PIPE[ID_STAGE];
    if (CURRENT_STATE.PIPE_STALL[ID_STAGE] == 0 && CURRENT_STATE.PIPE[ID_STAGE]) {
        if(DEBUG) printf("----STAGE: EX----\n");
        EX_MEM_inst = get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]);  // Instruction transmission
        BUF.EX_MEM_NPC = CURRENT_STATE.PIPE[ID_STAGE];		    // NEXTPC transmission
        BUF.EX_MEM_REG = CURRENT_STATE.ID_EX_DEST;		    // REG = Destination Register

        if (DEBUG) printf("ID: REG1: 0x%x REG2: 0x%x\n", CURRENT_STATE.ID_EX_REG1, CURRENT_STATE.ID_EX_REG2);

        BUF.EX_MEM_CHOICE = 0; // flag in lw/sw. Same Memread/Memwrite role
	/* Instruction excute */
        switch (OPCODE(EX_MEM_inst)) {
            // R-type
            case 0x00:
                switch (FUNC(EX_MEM_inst)) {
                    case 0x21:  // ADDU
                        BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_REG2;
                        break;
                    case 0x24:  // AND
                        BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 & CURRENT_STATE.ID_EX_REG2;
                        break;
                    case 0x08:  // JR
                        BUF.EX_MEM_CHOICE = 0;
                        BUF.EX_MEM_REG = 0;
                        break;
                    case 0x27:  // NOR
                        BUF.EX_MEM_ALU_OUT = ~(CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_REG2);
                        break;
                    case 0x25:  // OR
                        BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_REG2;
                        break;
                    case 0x2b:  // SLTU
                        BUF.EX_MEM_ALU_OUT = (CURRENT_STATE.ID_EX_REG1 < CURRENT_STATE.ID_EX_REG2)? 1 : 0;
                        break;
                    case 0x00:  // SLL
                        BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG2 << SHAMT(EX_MEM_inst);
                        if (DEBUG) printf("SHAMT: %d\n",  CURRENT_STATE.ID_EX_SHAMT);
                        break;
                    case 0x02:  // SRL
                        BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG2 >> SHAMT(EX_MEM_inst);
                        break;
                    case 0x23:  // SUBU
                        BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 - CURRENT_STATE.ID_EX_REG2;
                        break;
                }
                break;

            // I-type
            case 0x9:       // ADDIU
                BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
                break;
            case 0xc:       // ANDI
                BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 & CURRENT_STATE.ID_EX_IMM;
                if (DEBUG) printf("andi***: 0x%x\n", CURRENT_STATE.ID_EX_IMM);
                break;
            case 0xf:       // LUI
                BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_IMM << 16;
                if (DEBUG) printf("lui****: 0x%x\n", BUF.EX_MEM_ALU_OUT);
                break;
            case 0xd:       // ORI
                BUF.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_IMM;
                if (DEBUG) printf("ori****: 0x%x\n", BUF.EX_MEM_ALU_OUT);
                break;
            case 0xb:       // SLTIU
                BUF.EX_MEM_ALU_OUT = (CURRENT_STATE.ID_EX_REG1 < CURRENT_STATE.ID_EX_IMM) ? 1 : 0;
                break;
            case 0x23:      // LW
                BUF.EX_MEM_DEST = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
                if (DEBUG) printf("lw: EX_MEM_DEST: 0x%x\n", BUF.EX_MEM_DEST);
                BUF.EX_MEM_CHOICE = 2;
                break;
            case 0x2b:      // SW
                BUF.EX_MEM_DEST = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
                BUF.EX_MEM_W_VALUE = CURRENT_STATE.ID_EX_REG2;
                BUF.EX_MEM_REG = 0;
                BUF.EX_MEM_CHOICE = 1;
                if (DEBUG) printf("sw: EX_MEM_DEST: 0x%x\n", BUF.EX_MEM_DEST);
                break;
            case 0x04:      // BEQ
                BUF.EX_MEM_REG = 0;
                if (DEBUG) printf("beq) REG1: %d, REG2: %d\n",CURRENT_STATE.ID_EX_REG1, CURRENT_STATE.ID_EX_REG2);
                if (CURRENT_STATE.ID_EX_REG1 == CURRENT_STATE.ID_EX_REG2) {
                    BUF.EX_MEM_BR_TAKE = 1; // BRANCH FLAG
                    BUF.EX_MEM_BRANCH_DEST = CURRENT_STATE.PC + (CURRENT_STATE.ID_EX_IMM << 2);
                }
                else
                    BUF.EX_MEM_BR_TAKE = 0;
                break;
            case 0x5:       // BNE
                BUF.EX_MEM_REG = 0;
                if (DEBUG) printf("bne) REG1: %d, REG2: %d\n",CURRENT_STATE.ID_EX_REG1, CURRENT_STATE.ID_EX_REG2);
                if (CURRENT_STATE.ID_EX_REG1 != CURRENT_STATE.ID_EX_REG2) {
                    BUF.EX_MEM_BR_TAKE = 1; // BRANCH FLAG
                    BUF.EX_MEM_BRANCH_DEST = CURRENT_STATE.PC + (CURRENT_STATE.ID_EX_IMM << 2);
                }
                else
                    BUF.EX_MEM_BR_TAKE = 0;
                break;
            // J-type
            case 0x2:       // J
            case 0x3:       // JAL
                BUF.EX_MEM_CHOICE = 0;
                BUF.EX_MEM_REG = 0;
        }

	/* Forward preparation */
        if (BUF.EX_MEM_CHOICE == 0) {
            BUF.EX_MEM_WRITE = 1;
            BUF.EX_MEM_FORWARD_REG = BUF.EX_MEM_REG;
            BUF.EX_MEM_FORWARD_VALUE = BUF.EX_MEM_ALU_OUT;
        }

        if (DEBUG) printf("EX: ALU_OUT: 0x%x\n", BUF.EX_MEM_ALU_OUT);
    }

    /* MEM STAGE */
    BUF.PIPE[MEM_STAGE] = CURRENT_STATE.PIPE[EX_STAGE];
    if (CURRENT_STATE.PIPE_STALL[EX_STAGE] == 0 && CURRENT_STATE.PIPE[EX_STAGE]) {
        if (DEBUG) {
	    printf("----STAGE: MEM----\n");
	    printf("ex-mem-alu-out: 0x%x\n", CURRENT_STATE.EX_MEM_ALU_OUT);
	    printf("EX-MEM-DEST: 0x%x\nCHOICE : %d\n", CURRENT_STATE.EX_MEM_DEST, CURRENT_STATE.EX_MEM_CHOICE);
	}
        BUF.MEM_WB_NPC = CURRENT_STATE.PIPE[EX_STAGE];	    // NEXTPC transmission
        BUF.MEM_WB_REG = CURRENT_STATE.EX_MEM_REG;	    // Register transmission
        BUF.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;  // ALUOUT value transmission
        BUF.MEM_WB_ALU = FALSE;				    // flag lw/sw in use WB stage

        if (CURRENT_STATE.EX_MEM_CHOICE == 1) {		// SW process
            mem_write_32(CURRENT_STATE.EX_MEM_DEST, CURRENT_STATE.EX_MEM_W_VALUE);
            BUF.MEM_WB_REG = FALSE;
            BUF.MEM_WB_ALU = TRUE;
        }
        else if (CURRENT_STATE.EX_MEM_CHOICE == 2) {	// LW process
            BUF.MEM_WB_MEM_OUT = mem_read_32(CURRENT_STATE.EX_MEM_DEST);
            BUF.MEM_WB_ALU = TRUE;
	    BUF.MEM_WB_WRITE = TRUE;
	    /* Forwarding preparation */
            BUF.MEM_WB_FORWARD_REG2 = BUF.MEM_WB_FORWARD_REG1;
            BUF.MEM_WB_FORWARD_VALUE2 = BUF.MEM_WB_FORWARD_VALUE1;
            BUF.MEM_WB_FORWARD_REG1 = BUF.MEM_WB_REG;
            BUF.MEM_WB_FORWARD_VALUE1 = BUF.MEM_WB_MEM_OUT;
        }
        else {
            BUF.MEM_WB_ALU = 0;
	    BUF.MEM_WB_WRITE = TRUE;
	    /* Forward preparation */
	    BUF.MEM_WB_FORWARD_REG2 = BUF.MEM_WB_FORWARD_REG1;
            BUF.MEM_WB_FORWARD_VALUE2 = BUF.MEM_WB_FORWARD_VALUE1;
            BUF.MEM_WB_FORWARD_REG1 = BUF.MEM_WB_REG;
            BUF.MEM_WB_FORWARD_VALUE1 = BUF.MEM_WB_ALU_OUT;
	}

	/* Branch(beq/bne) process */
        if (CURRENT_STATE.EX_MEM_BR_TAKE == 1) {
            if (DEBUG) printf("BR_TAKE\n");
            BUF.EX_MEM_BR_TAKE = 0;
            BUF.BRANCH_PC = 1;
            BUF.EX_MEM_BR_TAKE = FALSE;
            BUF.NEXTPC = CURRENT_STATE.EX_MEM_BRANCH_DEST;
        }
    }

    /* WB STAGE */
    BUF.PIPE[WB_STAGE] = CURRENT_STATE.PIPE[MEM_STAGE];
    if (CURRENT_STATE.PIPE_STALL[MEM_STAGE] == 0 && CURRENT_STATE.PIPE[MEM_STAGE]) {
        if (DEBUG) printf("----STAGE: WB----\n");

	/* WRITE DATA TO REGISTER */
        if (CURRENT_STATE.MEM_WB_REG != 0) {
            if (DEBUG) {
                printf("write data: %d(0 IF ALU)\n", CURRENT_STATE.MEM_WB_ALU);
                printf("REG: %d, ALU_OUT: 0x%x, MEM_OUT: 0x%x\n", CURRENT_STATE.MEM_WB_REG, CURRENT_STATE.MEM_WB_ALU_OUT, CURRENT_STATE.MEM_WB_MEM_OUT);
            }
            if (CURRENT_STATE.MEM_WB_ALU == 0) {    // IF WRITE ALU_OUT value
                BUF.REGS[CURRENT_STATE.MEM_WB_REG] = CURRENT_STATE.MEM_WB_ALU_OUT;
                if (DEBUG) printf("write alu_out\n");
                if (BUF.PIPE[ID_STAGE] 
                        && (BUF.EX_MEM_FORWARD_REG == CURRENT_STATE.MEM_WB_REG)
                        && (BUF.EX_MEM_FORWARD_VALUE == CURRENT_STATE.MEM_WB_ALU_OUT)
                        && (BUF.EX_MEM_FORWARD_REG != RS(ID_EX_inst)
                            && (BUF.EX_MEM_FORWARD_REG != RT(ID_EX_inst)))) {
                    BUF.EX_MEM_WRITE = FALSE;
                    if (DEBUG) printf("EX_FORWARD OFF\n");
                }
                if (BUF.PIPE[ID_STAGE] && BUF.MEM_WB_FORWARD_REG2 == CURRENT_STATE.MEM_WB_REG
                        && CURRENT_STATE.MEM_WB_FORWARD_VALUE2 == CURRENT_STATE.MEM_WB_ALU_OUT
                        && CURRENT_STATE.MEM_WB_FORWARD_REG2 != CURRENT_STATE.EX_MEM_FORWARD_REG) {
                    BUF.MEM_WB_WRITE = TRUE;
                    if (DEBUG) printf("MEM_FORWARD OFF1\n");
                }
            }
            else {                                   // IF WRITE MEM_OUT value
                BUF.REGS[CURRENT_STATE.MEM_WB_REG] = CURRENT_STATE.MEM_WB_MEM_OUT;
                if (DEBUG) printf("write mem_out\n");
                if (BUF.PIPE[ID_STAGE] && CURRENT_STATE.MEM_WB_FORWARD_REG2 == CURRENT_STATE.MEM_WB_REG
                        && CURRENT_STATE.MEM_WB_FORWARD_VALUE2 == CURRENT_STATE.MEM_WB_MEM_OUT
                        && CURRENT_STATE.MEM_WB_FORWARD_REG2 != CURRENT_STATE.EX_MEM_FORWARD_REG) {
                    BUF.MEM_WB_WRITE = TRUE;
                    if (DEBUG) printf("MEM_FORWARD OFF2\n");
                }
            }
        }
    }

    /* Add INST_COUNT if PIPE[WB] is filled */
    if (BUF.PIPE[WB_STAGE])
        ++INSTRUCTION_COUNT;

    /* IF STAGE */
    if (CURRENT_STATE.PIPE_STALL[IF_STAGE] == 0 && CURRENT_STATE.PC && CYCLE_COUNT ) {
        if (DEBUG) printf("----STAGE: IF----\n");
        /* JUMP or BRANCH process */
	if (BUF.JUMP_PC || BUF.BRANCH_PC) {
            BUF.PC = BUF.NEXTPC + FETCH_BIT;
	    /* PUSH stall */
            for (i = 4 ; i > 0 ; i--) {
                BUF.PIPE_STALL[i] = BUF.PIPE_STALL[i - 1];
            }
            if (BUF.JUMP_PC && (BUF.EX_MEM_BR_TAKE == 0)) {	// JUMP and NOT BRANCH
                if (DEBUG) printf("JUMP\n");
                BUF.PIPE_STALL[IF_STAGE] = 2;
                BUF.JUMP_PC--;
                BUF.PC -= 4;
            }
            if (BUF.BRANCH_PC) {				// BRANCH
                if (DEBUG) printf("BRANCH\n");
                BUF.PC = BUF.NEXTPC;
                BUF.PIPE_STALL[IF_STAGE] = 2;
                BUF.PIPE_STALL[ID_STAGE] = 1;
                BUF.PIPE_STALL[EX_STAGE] = 1;
                BUF.BRANCH_PC = 0;
            }
        }
	/* Normal case process */
        else {
            BUF.PC = CURRENT_STATE.PC + FETCH_BIT;
            BUF.PIPE[IF_STAGE] = BUF.PC;
        }
        BUF.IF_ID_NPC = BUF.PC;	// NEXTPC transmission
        if (BUF.PC >= MEM_TEXT_START + NUM_INST * 4)
            BUF.PC = 0;
    }
    /* BEFORE CPU(CURRENT_STATE) branch */
    else if (CURRENT_STATE.PIPE_STALL[IF_STAGE] == 2){
        if (DEBUG) {
            printf("----STAGE: IF----\nStart branch\n");
            printf("NEXTPC: %x\n", CURRENT_STATE.PC);
            BUF.PIPE[IF_STAGE] = BUF.PC;
        }
        BUF.PIPE_STALL[IF_STAGE]--;
        BUF.PC = CURRENT_STATE.PC;
        BUF.IF_ID_NPC = BUF.PC;
        BUF.PIPE[IF_STAGE] = BUF.PC;
    }
    /* First cycle */
    else if (CYCLE_COUNT == 0) {
        BUF.PIPE[IF_STAGE] = BUF.PC;
    }

    /* If normal case(not jump/branch), PUSH stall */
    if (BUF.PIPE_STALL[IF_STAGE] != 2) {
        for (i = 4 ; i > 0 ; --i) {
            BUF.PIPE_STALL[i] = BUF.PIPE_STALL[i - 1];
        }
        BUF.PIPE_STALL[IF_STAGE] = 0;
    }

    /* lw process */
    if (CURRENT_STATE.STALL_CYCLE == 2) {
        BUF.EX_MEM_WRITE = 0;
        BUF.EX_MEM_FORWARD_REG = 0;
        BUF.PIPE_STALL[EX_STAGE] = 1;
        BUF.STALL_CYCLE = 0;
        BUF.PC = BUF.PIPE[ID_STAGE];
        BUF.PIPE[IF_STAGE] = BUF.PIPE[ID_STAGE];
        BUF.PIPE[ID_STAGE] = BUF.PIPE[EX_STAGE];
        if (DEBUG) printf("STALL PUT\n");
    }

    /* Flushing / address process in jump */
    for (i = 0; i < 5; ++i) {
        if (BUF.PIPE_STALL[i] != 0)
            BUF.PIPE[i] = 0;
        if (BUF.JUMP_PC){
            if (DEBUG) printf("JR || JAL\n");
            BUF.PIPE[IF_STAGE] = CURRENT_STATE.PC + 4;
            BUF.JUMP_PC --;
        }
    }

    /* UPDATE CURRENT_STATE TO BUF */
    BUF.PC += FETCH_BIT;
    if (DEBUG)
    {
	printf("BUF.PC:0x%x\n", BUF.PC);
        printf("CURRENTPC: %x\n", CURRENT_STATE.PC);
        printf("PIPE: ");
        for (i = 0; i < 5; ++i)
            printf("%x ", BUF.PIPE[i]);
        printf("\n");
        printf("PIPE_STALL: ");
        for (i = 0; i < 5; ++i)
            printf("%x ", BUF.PIPE_STALL[i]);
        printf("\n\n");
    }

    /* If address in pipe over, stall */
    for (i = 0 ; i < 5 ; i++) {
        if (BUF.PIPE[i] >= MEM_TEXT_START + NUM_INST * 4) {
            BUF.PIPE[i] = 0;
            BUF.PIPE_STALL[i] = 1;
        }
    }

    /* [FINAL] PC(BUF.PC) process */
    if (RUN_BIT == FALSE)
	BUF.PC = BUF.PIPE[WB_STAGE] + FETCH_BIT;

    if ((INSTRUCTION_COUNT == 100) && RUN_BIT && (BUF.STALL_CYCLE != 0))
        BUF.PC -= 4;

    memcpy(&CURRENT_STATE, &BUF, sizeof(CPU_State));
    if (DEBUG & 0) printf("%ld | %d\n", CYCLE_COUNT, INSTRUCTION_COUNT);
}
