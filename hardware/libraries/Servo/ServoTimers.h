/*
  Copyright (c) 2013 Arduino LLC. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

/*
 * Defines for 16 bit timers used with  Servo library
 *
 * If _useTimerX is defined then TimerX is a 16 bit timer on the current board
 * timer16_Sequence_t enumerates the sequence that the timers should be allocated
 * _Nbr_16timers indicates how many 16 bit timers are available.
 */

#define _useTimer1
//#define _useTimer2
//#define _useTimer3
//#define _useTimer4
//#define _useTimer5

#if defined (_useTimer1)
#define CHANNEL_FOR_TIMER1  0
#endif


//typedef enum { _timer1, _timer2, _timer3, _timer4, _timer5, _Nbr_16timers } timer16_Sequence_t ;

//NeoJou : try to use one hardware timer first
typedef enum { _timer1=0, _Nbr_16timers=1 } timer16_Sequence_t ;

