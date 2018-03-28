        IMPORT MadCurTCB
        IMPORT MadHighRdyTCB
        IMPORT __initial_sp
            
        IMPORT madThreadCheckReady
        IMPORT madSysTick
            
        EXPORT madOSStartUp
        EXPORT PendSV_Handler
        EXPORT SysTick_Handler

        PRESERVE8
        AREA    MadArchS, CODE, READONLY
        THUMB

SCB_ICSR     EQU  (0x004 + 0x0D00 + 0xE000E000)
PendSV_Mask  EQU  (0x00000001<<28)

;*****************************************
;              madOSStartUp
;*****************************************
madOSStartUp PROC
    LDR     SP,    =__initial_sp // Recover MSP (__initial_sp)
    MRS     R12,   CONTROL
    ORR     R12,   #0x00000002   // Go to Privileged-Mode using PSP.
    MSR     CONTROL, R12
    LDR     R0,    =MadCurTCB    // R0  = Addr{MadCurTCB}
    LDR     R0,    [R0]	         // R0  = MadCurTCB
    LDR     SP,    [R0]	         // MSP = MadCurTCB->pStk
    POP     {R4-R11, R14}        // POP R4-R11, EXC_RETURN
    POP     {R0-R3, R12, LR}     // POP R0-R3, R12, LR
    POP     {LR}                 // POP PC   => LR
    POP     {R12}                // POP xPSR => R12
    MSR     xPSR_nzcvq, R12
    BX      LR                   // Jump to the task which has the highest priority value.
    ENDP
    
;*****************************************
;              PendSV_Handler
;*****************************************
PendSV_Handler	PROC
    CPSID	I
    PUSH    {LR}
    LDR     R0, =madThreadCheckReady
    BLX		R0
    POP		{LR}				    // Recover EXC_RETURN
    CMP		R0,    #1				
    BNE		PendSV_Handler_DONE		// There is no need to switch task.
    MRS		R0,    PSP				// R0 <==> PSP
    STMDB	R0!,   {R4-R11, LR}		// There is no need to save MSP.
    LDR		R1,    =MadCurTCB		// R1 = Addr{MadCurTCB}
    LDR		R2,    [R1]				// R2 = MadCurTCB
    STR		R0,	   [R2]				// MadCurTCB->pStk = R0(PSP)
    LDR		R2,    =MadHighRdyTCB	// R2 = Addr{MadHighRdyTCB}
    LDR		R2,    [R2]				// R2 = MadHighRdyTCB
    STR		R2,    [R1]				// MadCurTCB = MadHighRdyTCB
    LDR		R0,    [R2]				// R0(PSP) = MadHighRdyTCB->pStk
    LDMIA	R0!,   {R4-R11, LR}		// Pop R4-R11, EXC_RETURN by hand.
    MSR		PSP,   R0				// Recover PSP
PendSV_Handler_DONE:
    CPSIE	I
    BX		LR
    ENDP

;*****************************************
;              SysTick_Handler
;*****************************************
SysTick_Handler PROC
    PUSH    {LR}
    LDR     R0,    =madSysTick
    BLX     R0
    CMP		R0,    #1
    BNE		SysTick_DONE
    LDR		R0,    =SCB_ICSR
    LDR		R1,    =PENDSV_MASK
    STR		R1,    [R0]
SysTick_DONE:
    POP     {PC}
    ENDP
        
        ALIGN
        END