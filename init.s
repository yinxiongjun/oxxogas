.text
    .global  entry_point
	.extern  AppInfo
	.global  AppInfo

	.extern  main
	.global  main

entry_point:
 	B        PosMain
	B        AppInfo

PosMain:
    STMFD	  SP!, {LR}	 @ Store the return address
    BL        INT_Clear_BSS
    BL        main
    LDMFD     SP!, {PC}^

INT_bss_start:
    .word     Image__bss__ZI__Base
INT_bss_end:
    .word     Image__bss__ZI__Limit
INT_Clear_BSS:
    @ Get BSS start and end addresses
	LDR     r0,INT_bss_start
    LDR     r1,INT_bss_end
   @ Get a value to clear BSS
	MOV     r2,#0
   @ Clear the entire memory range
INT_BSS_Clear_Loop:
	CMP     r0,r1                           @ Are the start and end equal?
	STRNE   r2,[r0],#4                      @ Clear 4-bytes and move to next 4-bytes
	BNE     INT_BSS_Clear_Loop              @ Continue clearing until done
	MOV     PC,LR


