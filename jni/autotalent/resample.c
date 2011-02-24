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
    double factor = outputRate/(double)inputRate;
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

    memset(X1, 0, sizeof(short)*Xoff);
    memset(X2, 0, sizeof(short)*Xoff);

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
