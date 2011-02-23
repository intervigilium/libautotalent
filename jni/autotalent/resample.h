/* resample.h - sampling rate conversion subroutines
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

#define PI (3.14159265358979232846)
#define PI2 (6.28318530717958465692)
#define D2R (0.01745329348)          /* (2*pi)/360 */
#define R2D (57.29577951)            /* 360/(2*pi) */

#define MAX(x,y) ((x)>(y) ?(x):(y))
#define MIN(x,y) ((x)<(y) ?(x):(y))
#define ABS(x)   ((x)<0   ?(-(x)):(x))
#define SGN(x)   ((x)<0   ?(-1):((x)==0?(0):(1)))

#define Nhc       8
#define Na        7
#define Np       (Nhc+Na)
#define Npc      (1<<Nhc)
#define Amask    ((1<<Na)-1)
#define Pmask    ((1<<Np)-1)
#define Nh       16
#define Nb       16
#define Nhxn     14
#define Nhg      (Nh-Nhxn)
#define NLpScl   13

#define MAX_HWORD (32767)
#define MIN_HWORD (-32768)


int resample(
    short *inputL,
    short *inputR,
    int inputRate,
    short *outputL,
    short *outputR,
    int outputRate,
    int numSamples,
    int nChans
);
