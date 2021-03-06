/*
 * svccalls.s
 *
 *  Created on: Mar 6, 2021
 *      Author: Evgeny
 */

 .section .text.millis,"ax",%progbits
 .global millis
 .thumb
 .align 4
millis:
  push {r0,r3}
  mov r0, sp
  movs r3, #2
  svc 0
  pop {r0, r3}
  bx lr

