        IMPORT madCurTCB
        IMPORT madHighRdyTCB
        IMPORT __initial_sp
            
        IMPORT madThreadCheckReady
        IMPORT madSysTick
            
        EXPORT madOSStartUp
        EXPORT PendSV_Handler
        EXPORT SysTick_Handler

        PRESERVE8
        AREA    MadArchS, CODE, READONLY
        THUMB

SCB_ISCR     EQU  (0x004 + 0x0D00 + 0xE000E000)
PendSV_Mask  EQU  (0x00000001<<28)

;*****************************************
;              madOSStartUp
;*****************************************
madOSStartUp PROC
    
    LDR 	R0, =madCurTCB		    ; R0  = Addr{madCurTCB}
    LDR		R0, [R0]				; R0  = madCurTCB
    LDR		SP, [R0]				; MSP = madCurTCB->pStk

    POP 	{R4-R11, R14}			; POP R4-R11, EXC_RETURN
    POP 	{R0-R3, R12, LR}		; POP R0-R3, R12, LR
    POP 	{R14}					; POP PC   => LR
    POP 	{R12}					; POP xPSR => R12
    
    MSR 	xPSR, R12				; Recover xPSR
    MOV		R12, SP					; Recover PSP
    MSR		PSP, R12
    LDR 	SP, =__initial_sp		; Recover MSP

    MRS		R12, CONTROL
    ORR		R12, #0x00000002		; Go to Privileged-Mode using PSP.
    MSR		CONTROL, R12
    
    BX		LR						; Jump to the task which has the highest priority value.
    
    ENDP
    
;*****************************************
;              PendSV_Handler
;*****************************************
PendSV_Handler	PROC

    CPSID	I
			
    PUSH	{LR}
    LDR     R0, =madThreadCheckReady
    BLX		R0
    POP		{LR}					; Recover EXC_RETURN
    CMP		R0, #1				
    BNE		PendSV_Handler_DONE		; There is no need to switch task.
    
    MRS		R0, PSP					; R0 <==> PSP
    STMDB	R0!, {R4-R11, LR}		; There is no need to save MSP.
                                    
    LDR		R1, =madCurTCB			; R1 = Addr{madCurTCB}
    LDR		R2, [R1]				; R2 = madCurTCB
    STR		R0,	[R2]				; madCurTCB->pStk = R0(PSP)
    
    ;LDR		R1, =madCurTCB		; R1 = Addr{madCurTCB}
    LDR		R2, =madHighRdyTCB		; R2 = Addr{madHighRdyTCB}
    LDR		R2, [R2]				; R2 = madHighRdyTCB
    STR		R2, [R1]				; madCurTCB = madHighRdyTCB			
    LDR		R0, [R2]				; R0(PSP) = madHighRdyTCB->pStk
    
    LDMIA	R0!, {R4-R11, LR}		; Pop R4-R11, EXC_RETURN by hand.
    MSR		PSP, R0					; Recover PSP

PendSV_Handler_DONE
    CPSIE	I
    BX		LR
    
    ENDP

;*****************************************
;              SysTick_Handler
;*****************************************
SysTick_Handler PROC
    
    PUSH    {LR}
    LDR     R0, =madSysTick
    BLX     R0
    CMP		R0, #1				
    BNE		SysTick_DONE
    
    LDR		R0, =SCB_ISCR
    LDR		R1, =PendSV_Mask
    STR		R1, [R0]
    
SysTick_DONE
    POP     {PC}
    
    ENDP
        
        ALIGN
        END