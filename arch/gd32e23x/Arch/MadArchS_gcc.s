    .syntax unified
    .cpu cortex-m23
    .fpu softvfp
    .thumb

.global  madOSStartUp
.global  madPendSVHandler
.global  madSysTickHandler

/*****************************************
              madOSStartUp
*****************************************/
    .section .text.madOSStartUp
    .type    madOSStartUp, %function
madOSStartUp:
    LDR     R0,         =#0
    MSR     MSPLIM,     R0
    MSR     PSPLIM,     R0
    LDR     R0,         =_estack       @ Recover MSP (__initial_sp)
    MSR     MSP,        R0
    MRS     R0,         CONTROL        @ Go to Privileged-Mode using PSP.
    LDR     R1,         =#0x00000002
    ORRS    R0,         R1
    MSR     CONTROL,    R0
    LDR     R0,         =MadCurTCB     @ R0  = Addr{MadCurTCB}
    LDR     R0,         [R0]           @ R0  = MadCurTCB
    LDR     R0,         [R0]           @ R0  = MadCurTCB->pStk
    MSR     PSP,        R0             @ PSP = MadCurTCB->pStk
    POP     {R0-R7}                    @ POP { R4-R11 }, Droped.
    POP     {R0-R7}                    @ POP { R0-R3, R12, LR, PC, xPSR }
    MOV     LR,         R6             @ LR = PC
    MSR     xPSR_nzcvq, R7             @ Recover xPSR
    BX      LR
    .size   madOSStartUp, .-madOSStartUp

/*****************************************
              PendSV_Handler
*****************************************/
    .section .text.madPendSVHandler
    .type    madPendSVHandler, %function
madPendSVHandler:
    CPSID	I
    PUSH    {LR}
    LDR     R0,         =madThreadCheckReady
    BLX		R0
    POP		{R1}                         @ Recover EXC_RETURN
    MOV     LR,         R1
    CMP		R0,         #1				
    BNE		PendSV_Handler_DONE          @ There is no need to switch task.
    MRS     R12,        MSP              @ Record MSP (R12)
    MRS		R0,         PSP              @ R0  = PSP
    MSR     MSP,        R0               @ MSP = PSP
    PUSH    {R4-R7}                      @ Push R4-R7
    MOV     R4,         R8               @ Push R8-R11
    MOV     R5,         R9
    MOV     R6,         R10
    MOV     R7,         R11
    PUSH    {R4-R7}
    MRS     R0,         MSP              @ R0 = PSP!
    LDR		R1,         =MadCurTCB       @ R1 = Addr{MadCurTCB}
    LDR		R2,         [R1]             @ R2 = MadCurTCB
    STR		R0,	        [R2]             @ MadCurTCB->pStk = PSP
    LDR		R2,         =MadHighRdyTCB   @ R2 = Addr{MadHighRdyTCB}
    LDR		R2,         [R2]             @ R2 = MadHighRdyTCB
    STR		R2,         [R1]             @ MadCurTCB = MadHighRdyTCB
    LDR		R0,         [R2]             @ R0 = MadHighRdyTCB->pStk
    MSR     MSP,        R0               @ MSP = PSP
    POP     {R4-R7}                      @ Pop R4-R7
    POP     {R0-R3}                      @ Pop R8-R11
    MOV     R8,         R0
    MOV     R9,         R1
    MOV     R10,        R2
    MOV     R11,        R3
    MRS     R0,         MSP              @ Record PSP!
    MSR     MSP,        R12              @ Recover MSP
    MSR     PSP,        R0               @ Recover PSP
PendSV_Handler_DONE:
    CPSIE	I
    BX		LR
    .size   madPendSVHandler, .-madPendSVHandler

/*****************************************
              SysTick_Handler
*****************************************/
.equ SCB_ICSR,    0xE000E000 + 0x0D00 + 0x04
.equ PENDSV_MASK, 0x00000001 << 28

    .section .text.madSysTickHandler
    .type    madSysTickHandler, %function
madSysTickHandler:
    CPSID	I
    PUSH    {LR}
    LDR     R0,         =madSysTick
    BLX     R0
    CMP		R0,         #1
    BNE		SysTick_DONE
    LDR		R0,         =SCB_ICSR
    LDR		R1,         =PENDSV_MASK
    STR		R1,         [R0]
SysTick_DONE:
    CPSIE	I
    POP     {PC}
    .size   madSysTickHandler, .-madSysTickHandler
