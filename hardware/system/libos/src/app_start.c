/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */



extern  void SVC_Handler (void);
extern  void PendSV_Handler (void);
extern  void SysTick_Handler (void);



//
// ROM function : set vector table 11 / 14 / 15 to the relative function 
//
#define InterruptForOSInit VectorTableInitForOSRtl8195A

extern __attribute__ ((long_call))
void
VectorTableInitForOSRtl8195A(
    void *PortSVC,
    void *PortPendSVH,
    void *PortSysTick    
);


// The Main App entry point
void _AppStart(void)
{
    InterruptForOSInit((void*)SVC_Handler,
                       (void*)PendSV_Handler,
                       (void*)SysTick_Handler);
    __asm (
        "ldr   r0, =SystemInit\n"
        "blx   r0\n"
        "ldr   r0, =_start\n"
        "bx    r0\n"
    );

}



