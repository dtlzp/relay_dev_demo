/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_macro.h
 * function: dingtian macro define
 */

#ifndef _DT_MACRO_H_
#define _DT_MACRO_H_

#define BIT_ALIGN(type, len)         ( ((len)+((sizeof(type)<<3)-1))/(sizeof(type)<<3) )

#define BIT_ALIGN_BYTE(type, len)    ( BIT_ALIGN(type,len) * sizeof(type) )

#define BYTE_ALIGN(blk, len)        ( (((len)+((blk)-1)) / (blk))*(blk) )

#ifndef NBBY
#define NBBY (8)
#endif

#ifndef setbit
/* Bit map related macros.  */
#define setbit(a,i)     ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i)     ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)      ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i)      (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)
#endif
#define notbit(a,i)     ((a)[(i)/NBBY] ^= (1<<((i)%NBBY)))
#define mkbit(a,i,b)    (b?setbit(a,i):clrbit(a,i))

/* ms >= 10 */
#define HW_MS(ms)       ((ms)/portTICK_PERIOD_MS)

#define S2US    (1000000)
#define S2MS    (1000)
#define MS2US   (1000)
#define US2NS   (1000)

#define SECOND_MS(x)   ((x)*S2MS)
#define MINUTE_MS(x)   ((x)*SECOND_MS(60))
#define HOUR_MS(x)     ((x)*MINUTE_MS(60))
#define T_MS(ms)       (ms)
#define T_MS2US(ms)    ((ms)*MS2US)

#endif /* end _DT_MACRO_H_ */



