/* resample.c - sampling rate conversion subroutines
 *
 * Original version available at the 
 * Digital Audio Resampling Home Page located at
 * http://ccrma.stanford.edu/~jos/resample/.
 *
 * Modified for use on Android by Ethan Chen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "resample.h"

#define IBUFFSIZE 4096                         /* Input buffer size */


static inline short WordToHword(int v, int scl)
{
    short out;
    int llsb = (1<<(scl-1));
    v += llsb;          /* round */
    v >>= scl;
    if (v>MAX_HWORD) {
        v = MIN_HWORD;
    }
    out = (short) v;
    return out;
}


int FilterUp(short Imp[], short ImpD[], 
             unsigned short Nwing, int Interp,
             short *Xp, short Ph, short Inc)
{
    short *Hp, *Hdp = NULL, *End;
    short a = 0;
    int v, t;
    
    v=0;
    Hp = &Imp[Ph>>Na];
    End = &Imp[Nwing];
    if (Interp) {
        Hdp = &ImpD[Ph>>Na];
        a = Ph & Amask;
    }   
    if (Inc == 1) {         /* If doing right wing drop extra coeff, so when Ph is */
        End--;              /* 0.5, we don't do too many mult's */
        if (Ph == 0) {      /* If the phase is zero then we've already skipped the */
            Hp += Npc;      /* first sample, so we must also  */
            Hdp += Npc;     /* skip ahead in Imp[] and ImpD[] */
        }
    }   
    if (Interp) {
        while (Hp < End) {
            t = *Hp;                    /* Get filter coeff */
            t += (((int)*Hdp)*a)>>Na;   /* t is now interp'd filter coeff */
            Hdp += Npc;                 /* Filter coeff differences step */
            t *= *Xp;                   /* Mult coeff by input sample */
            if (t & (1<<(Nhxn-1))) {    /* Round, if needed */
                t += (1<<(Nhxn-1));
            }
            t >>= Nhxn;     /* Leave some guard bits, but come back some */
            v += t;         /* The filter output */
            Hp += Npc;      /* Filter coeff step */
            Xp += Inc;      /* Input signal step. NO CHECK ON BOUNDS */
        }   
    } else {
        while (Hp < End) {
            t = *Hp;        /* Get filter coeff */
            t *= *Xp;       /* Mult coeff by input sample */
            if (t & (1<<(Nhxn-1))) {    /* Round, if needed */
                t += (1<<(Nhxn-1));
            }
            t >>= Nhxn;     /* Leave some guard bits, but come back some */
            v += t;         /* The filter output */
            Hp += Npc;      /* Filter coeff step */
            Xp += Inc;      /* Input signal step. NO CHECK ON BOUNDS */
        }
    }
    return(v);
}


int FilterUD(short Imp[], short ImpD[],
             unsigned short Nwing, int Interp,
             short *Xp, short Ph, short Inc, unsigned short dhb)
{
    short a;
    short *Hp, *Hdp, *End;
    int v, t;
    unsigned int Ho; 
    
    v=0;
    Ho = (Ph*(unsigned int)dhb)>>Np;
    End = &Imp[Nwing];
    if (Inc == 1) {     /* If doing right wing drop extra coeff, so when Ph is */
        End--;          /* 0.5, we don't do too many mult's */
        if (Ph == 0) {  /* If the phase is zero...           */
            Ho += dhb;  /* ...then we've already skipped the */
        }
    }                   /* first sample, so we must also  */
                        /* skip ahead in Imp[] and ImpD[] */
    if (Interp) {
        while ((Hp = &Imp[Ho>>Na]) < End) {
            t = *Hp;                    /* Get IR sample */
            Hdp = &ImpD[Ho>>Na];        /* get interp (lower Na) bits from diff table*/
            a = Ho & Amask;             /* a is logically between 0 and 1 */
            t += (((int)*Hdp)*a)>>Na;  /* t is now interp'd filter coeff */
            t *= *Xp;                   /* Mult coeff by input sample */
            if (t & 1<<(Nhxn-1)) {      /* Round, if needed */
                t += 1<<(Nhxn-1);
            }
            t >>= Nhxn;                 /* Leave some guard bits, but come back some */
            v += t;                     /* The filter output */
            Ho += dhb;                  /* IR step */
            Xp += Inc;                  /* Input signal step. NO CHECK ON BOUNDS */
        }   
    } else {
        while ((Hp = &Imp[Ho>>Na]) < End) {
            t = *Hp;                    /* Get IR sample */
            t *= *Xp;                   /* Mult coeff by input sample */
            if (t & 1<<(Nhxn-1)) {      /* Round, if needed */
                t += 1<<(Nhxn-1);
            }
            t >>= Nhxn;                 /* Leave some guard bits, but come back some */
            v += t;                     /* The filter output */
            Ho += dhb;                  /* IR step */
            Xp += Inc;                  /* Input signal step. NO CHECK ON BOUNDS */
        }
    }
    return(v);
}


/* Sampling rate conversion using linear interpolation for maximum speed. */
static int SrcLinear(short X[], short Y[], double factor, unsigned int *Time,
                     unsigned short Nx)
{
    short iconst;
    short *Xp, *Ystart;
    int v,x1,x2;
    
    double dt;                  /* Step through input signal */ 
    unsigned int dtb;                  /* Fixed-point version of Dt */
    unsigned int endTime;              /* When Time reaches EndTime, return to user */
    
    dt = 1.0/factor;            /* Output sampling period */
    dtb = dt*(1<<Np) + 0.5;     /* Fixed-point representation */
    
    Ystart = Y;
    endTime = *Time + (1<<Np)*(int)Nx;
    while (*Time < endTime)
    {
        iconst = (*Time) & Pmask;
        Xp = &X[(*Time)>>Np];      /* Ptr to current input sample */
        x1 = *Xp++;
        x2 = *Xp;
        x1 *= ((1<<Np)-iconst);
        x2 *= iconst;
        v = x1 + x2;
        *Y++ = WordToHword(v,Np);   /* Deposit output */
        *Time += dtb;               /* Move to next sample by time increment */
    }
    return (Y - Ystart);            /* Return number of output samples */
}


/* Sampling rate up-conversion only subroutine;
 * Slightly faster than down-conversion; */
static int SrcUp(short X[], short Y[], double factor, unsigned int *Time,
                 unsigned short Nx, unsigned short Nwing, unsigned short LpScl,
                 short Imp[], short ImpD[], int Interp)
{
    short *Xp, *Ystart;
    int v;
    
    double dt;                  /* Step through input signal */ 
    unsigned int dtb;                  /* Fixed-point version of Dt */
    unsigned int endTime;              /* When Time reaches EndTime, return to user */
    
    dt = 1.0/factor;            /* Output sampling period */
    dtb = dt*(1<<Np) + 0.5;     /* Fixed-point representation */
    
    Ystart = Y;
    endTime = *Time + (1<<Np)*(int)Nx;
    while (*Time < endTime)
    {
        Xp = &X[*Time>>Np];      /* Ptr to current input sample */
        /* Perform left-wing inner product */
        v = FilterUp(Imp, ImpD, Nwing, Interp, Xp, (short)(*Time&Pmask),-1);
        /* Perform right-wing inner product */
        v += FilterUp(Imp, ImpD, Nwing, Interp, Xp+1, 
		      /* previous (triggers warning): (short)((-*Time)&Pmask),1); */
                      (short)((((*Time)^Pmask)+1)&Pmask),1);
        v >>= Nhg;              /* Make guard bits */
        v *= LpScl;             /* Normalize for unity filter gain */
        *Y++ = WordToHword(v,NLpScl);   /* strip guard bits, deposit output */
        *Time += dtb;           /* Move to next sample by time increment */
    }
    return (Y - Ystart);        /* Return the number of output samples */
}


/* Sampling rate conversion subroutine */
static int SrcUD(short X[], short Y[], double factor, unsigned int *Time,
                 unsigned short Nx, unsigned short Nwing, unsigned short LpScl,
                 short Imp[], short ImpD[], int Interp)
{
    short *Xp, *Ystart;
    int v;
    
    double dh;                  /* Step through filter impulse response */
    double dt;                  /* Step through input signal */
    unsigned int endTime;              /* When Time reaches EndTime, return to user */
    unsigned int dhb, dtb;             /* Fixed-point versions of Dh,Dt */
    
    dt = 1.0/factor;            /* Output sampling period */
    dtb = dt*(1<<Np) + 0.5;     /* Fixed-point representation */
    
    dh = MIN(Npc, factor*Npc);  /* Filter sampling period */
    dhb = dh*(1<<Na) + 0.5;     /* Fixed-point representation */
    
    Ystart = Y;
    endTime = *Time + (1<<Np)*(int)Nx;
    while (*Time < endTime)
    {
        Xp = &X[*Time>>Np];     /* Ptr to current input sample */
        v = FilterUD(Imp, ImpD, Nwing, Interp, Xp, (short)(*Time&Pmask),
                     -1, dhb);  /* Perform left-wing inner product */
        v += FilterUD(Imp, ImpD, Nwing, Interp, Xp+1, 
		      /* previous (triggers warning): (short)((-*Time)&Pmask), */
                      (short)((((*Time)^Pmask)+1)&Pmask),
                      1, dhb);  /* Perform right-wing inner product */
        v >>= Nhg;              /* Make guard bits */
        v *= LpScl;             /* Normalize for unity filter gain */
        *Y++ = WordToHword(v,NLpScl);   /* strip guard bits, deposit output */
        *Time += dtb;           /* Move to next sample by time increment */
    }
    return (Y - Ystart);        /* Return the number of output samples */
}


static int readDataBuffers(
    short *inputL,
    short *inputR,
    int inCount,
    short *outPtr1,
    short *outPtr2,
    int dataArraySize,
    unsigned int *framecount,
    int nChans,
    int Xoff)
{
    int i, Nsamps, nret;

    Nsamps = dataArraySize - Xoff;
    outPtr1 += Xoff;
    outPtr2 += Xoff;

    if (nChans == 1) {
        memcpy(outPtr1, inputL, sizeof(short) * (Nsamps - 1));
    } else {
        memcpy(outPtr1, inputL, sizeof(short) * (Nsamps - 1));
        memcpy(outPtr2, inputR, sizeof(short) * (Nsamps - 1));
    }

    *framecount += Nsamps;

    if (*framecount >= (unsigned) inCount) {
        return (((Nsamps - (*framecount - inCount)) - 1) + Xoff);
    } else {
        return 0;
    }
}


int resample(
    short *inputL,
    short *inputR,
    int inputRate,
    short *outputL,
    short *outputR,
    int outputRate,
    int numSamples,
    int nChans)
{
    unsigned int Time, Time2;
    unsigned short Xp, Ncreep, Xoff, Xread;
    double factor = outputRate/inputRate;
    int OBUFFSIZE = (int)(((double)IBUFFSIZE)*factor+2.0);
    short X1[IBUFFSIZE], X2[IBUFFSIZE];
    short Y1[OBUFFSIZE], Y2[OBUFFSIZE];
    unsigned short Nout, Nx;
    int i, Ycount, last;
    unsigned int framecount;
    unsigned int outIdx;

    framecount = 0;
    outIdx = 0;
    Xoff = 10;
    Nx = IBUFFSIZE - 2*Xoff;
    last = 0;
    Ycount = 0;
    Xp = Xoff;
    Xread = Xoff;
    Time = (Xoff<<Np);

    for (i=0; i<Xoff; X1[i++]=0);
    for (i=0; i<Xoff; X2[i++]=0);

    do {
        if (!last) {
            last = readDataBuffers(inputL, inputR, numSamples, X1, X2, IBUFFSIZE,
                                   &framecount, nChans, (int)Xread);
            if (last && (last-Xoff<Nx)) {
                Nx = last-Xoff;
                if (Nx <= 0) {
                    break;
                }
            }
        }

        Time2 = Time;
        Nout = SrcLinear(X1,Y1,factor,&Time,Nx);
        if (nChans==2) {
            Nout = SrcLinear(X2,Y2,factor,&Time2,Nx);
        }

        Time -= (Nx<<Np);
        Xp += Nx;
        Ncreep = (Time>>Np) - Xoff;
        if (Ncreep) {
            Time -= (Ncreep<<Np);
            Xp += Ncreep;
        }
        for (i=0; i<IBUFFSIZE-Xp+Xoff; i++) {
            X1[i] = X1[i+Xp-Xoff];
            if (nChans==2) {
                X2[i] = X2[i+Xp-Xoff];
            }
        }
        if (last) {
            last -= Xp;
            if (!last) {
                last++;
            }
        }
        Xread = i;
        Xp = Xoff;

        Ycount += Nout;
        if (Ycount>numSamples) {
            Nout -= (Ycount-numSamples);
            Ycount = numSamples;
        }

        if (nChans==1) {
            // these need to map to the complete output buffers
            for (i=outIdx; i<Nout+outIdx; i++) {
                outputL[i] = Y1[i];
            }
        } else {
            for (i=outIdx; i<Nout+outIdx; i++) {
                outputL[i] = Y1[i];
                outputR[i] = Y2[i];
            }
        }

    } while (Ycount<numSamples);

    return(Ycount);
}
