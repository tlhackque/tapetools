/* Backup-10 for POSIX environments
 */

/* Copyright (c) 2015 Timothe Litt litt at acm ddot org
 * All rights reserved.
 *
 * This software is provided under GPL V2, including its disclaimer of
 * warranty.  Licensing under other terms may be available from the author.
 *
 * See the COPYING file for the well-known text of GPL V2.
 *
 * Bug reports, fixes, suggestions and improvements are welcome.
 *
 * This is a rewrite of backup.c and backwr.c, which have been
 * distributed on the internet for some time - with no authors listed.
 * This rewrite fixes many bugs, adds features, and has at most
 * a loose relationship to its predecessors.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "data36.h"

#ifndef __BYTE_ORDER__
#  define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif
#ifndef __ORDER_LITTLE_ENDIAN__
#  define __ORDER_LITTLE_ENDIAN__ 1234
#endif
#ifndef __ORDER_PDP_ENDIAN__
#  define __ORDER_PDP_ENDIAN__ 3412
#endif
#  ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__ 4321
#endif

/* Decode 36-bit word into a 64-bit buffer */

uint8_t *decode36( wd36_T *data, uint8_t *buf ) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    buf[0] =         data->rh  & 0377;
    buf[1] =  (data->rh >>  8) & 0377;
    buf[2] = ((data->rh >> 16) & 0003) | ((data->lh << 2) & 0374);
    buf[3] =  (data->lh >>  6) & 0377;
    buf[4] =  (data->lh >> 14) & 0017;
    buf[5] =
        buf[6] =
        buf[7] = 0;
#elif __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
    buf[0] =
        buf[1] =
        buf[2] = 0;
    buf[3] =  (data->lh >> 14) & 0017;
    buf[4] =  (data->lh >>  6) & 0377;
    buf[5] = ((data->rh >> 16) & 0003) | ((data->lh << 2) & 0374);
    buf[6] =  (data->rh >>  8) & 0377;
    buf[7] =         data->rh  & 0377;
#else /* PDP_ENDIAN */
    buf[0] =
        buf[1] = 0;

    buf[2] =  (data->lh >> 14) & 0017;
    buf[3] = 0;

    buf[4] = ((data->rh >> 16) & 0003) | ((data->lh << 2) & 0374);
    buf[5] =  (data->lh >>  6) & 0377;

    buf[6] =         data->rh  & 0377;
    buf[7] =  (data->rh >>  8) & 0377;
#endif

    return buf + 8;
}

/* Encode a 64-bit buffer into 36-bit word(s) */

uint8_t *encode36( const uint8_t *buf, wd36_T *data, size_t wds ) {
    abort();/* TBD -- endian?*/
}

/* Decode an ASCIZ string from 36-bit words into a new string */

char *decodeasciz( wd36_T *data ) {
    size_t len;
    uint8_t *string, *ep;

    len = data++->rh * sizeof( uint8_t );
    string = malloc( (len * 5) + 1 );
    if( !string )
        return NULL;

    ep = string;

    while( len-- ) {
        ep = decode7ascii( data++, ep );
    }
    *ep = '\0';
    return (char *)string;
 }

/* Encode an ASCII string into a 7-bit ASCIZ string in 36-bit words */

size_t encodeasciz( const char *string, wd36_T *data, size_t wds ) {
    size_t used;

    if( wds == 0 )
        return 0;

    used = encode7ascii( string, data, wds );

    /* encode7ascii has padded unused space with 0.
     */

    if( used == wds ) { /* Output full, truncate if last word full */
        data += used -1;
        if( (data->rh & (0177 << 1)) != 0 )
            data->rh &= ~(0177 << 1);
    } else { /* Not full.  If no output or last word full, add terminating NUL */
        if( !used || ((data+used-1)->rh & (0177 << 1)) != 0 )
            used++;
    }

    return used;
}

/* Decode 5 7-bit ASCII chars from a 36-bit word into an 8-bit file  buffer */

uint8_t *decode7ascii( wd36_T *data, uint8_t *buf ) {
    buf[0] =  (data->lh >> 11) & 0177;
    buf[1] =  (data->lh >>  4) & 0177;
    buf[2] = ((data->lh <<  3) & 0170) | ((data->rh >> 15) & 0007);
    buf[3] =  (data->rh >>  8) & 0177;
    buf[4] =  (data->rh >>  1) & 0177;

    return buf + 5;
}

/* Encode a string  into 36-bit word(s) of 5 7-bit ASCII characters */

size_t encode7ascii( const char *string, wd36_T *data, size_t wds ) {
    size_t used = 0;

    while( wds && *string ) {
        used++;

        data->lh =
            data->rh = 0;
        if( *string ) {
            data->lh = (*string++ & 0177) << 11;
            if( *string ) {
                data->lh |= (*string++ & 0177) << 4;
                if( *string ) {
                    data->lh |= (*string >> 3) & 017;
                    data->rh |= (*string++ & 07) << 15;
                    if( *string ) {
                        data->rh |= (*string++ & 0177) << 8;
                        if( *string ) {
                            data->rh |= (*string++ & 0177) << 1;
                        }
                    }
                }
            }
        }
        data++;
    }
        
    while( wds-- ) {
        data->lh = 0;
        data++->rh = 0;
    }
    return used;
}

/* Decode 4 8-bit ASCII chars from a 36-bit word into an 8-bit file  buffer */

uint8_t *decode8ascii( wd36_T *data, uint8_t *buf ) {
    buf[0] =  (data->lh >> 10) & 0377;
    buf[1] =  (data->lh >>  2) & 0377;
    buf[2] = ((data->lh <<  6) & 0300) | ((data->rh >> 12) & 0077);
    buf[3] =  (data->rh >>  4) & 0377;

    return buf + 4;
}

/* Encode a string  into 36-bit word(s) of 4 8-bit ASCII characters */

size_t encode8ascii( const char *string, wd36_T *data, size_t wds ) {
    size_t used = 0;

    while( wds && *string ) {
        used++;

        data->lh =
            data->rh = 0;
        if( *string ) {
            data->lh = (*string++ & 0377) << 10;
            if( *string ) {
                data->lh |= (*string++ & 0377) << 2;
                if( *string ) {
                    data->lh |= (*string >> 6) & 03;
                    data->rh |= (*string++ & 077) << 12;
                    if( *string ) {
                        data->rh |= (*string++ & 0377) << 4;
                    }
                }
            }
        }
        data++;
    }
        
    while( wds-- ) {
        data->lh = 0;
        data++->rh = 0;
    }
    return used;
}

/* Decode a TOPS version from a 36-bit word into a string of size at least VERSION_BUFFER_SIZE */

char *decodeversion( wd36_T *data, char *buffer ) {
    uint32_t major, minor, edit, cust;
    char *ep;

    major = (data->lh & 0077700) >> 6;
    minor = (data->lh & 0000077);
    cust  = (data->lh & 0700000) >> (6 + 9);
    edit  = data->rh;

    ep = buffer;
    *ep = '\0';

    if( major )
        ep += sprintf( ep, "%" PRIo32, major );

    if( minor ) {
        ldiv_t minv;

        minv = ldiv( minor -1, 26 );

        if( minv.quot )
            ep += sprintf( ep, "%c%c",
                           (int) ('A' -1 + minv.quot),
                           (int) ('A' + minv.rem) );
        else
            ep += sprintf( ep, "%c",
                           (int) ('A' + minv.rem) );
    }

    if( edit )
        ep += sprintf( ep, 
                       ( edit & (1 << 17) )? "(%" PRId32 ")": "(%" PRIo32 ")",
                       edit );

    if( cust )
        sprintf( ep, "-%" PRIo32, cust );

    return buffer;
}
