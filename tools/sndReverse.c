/* 
 * Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA 
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
/*
 *	Usage:  sndReverse <inputSoundFile> <outputSoundFile>
 */

#include "../sms.h"

static void fail(char *s, char *s2)
{
    printf("*** %s%s\n\n",s,s2);
    exit(1);                        /* Exit, indicating error */
}

int processSound (int inCount, short *inPtr, short *outPtr, int nChans)
{
    int i;
	short *pNew = (short *) (inPtr+inCount);

    for (i=0; i<inCount; i++)
      *outPtr++ = *pNew--;
    return inCount;
}

void main(int argc,char **argv) {
    SNDSoundStruct *sndin, *sndout;
    short *inPtr, *outPtr;
    FILE *outstream;
    int inCount, outCount, width, nChans;
    
    if (SNDReadSoundfile(*++argv,&sndin))
      fail("could not open input file ",*argv);
    else
      SNDGetDataPointer(sndin, (char **)(&inPtr), &inCount, &width);
    if (width != 2)
      fail("Can only handle 16-bit soundfiles","");
    nChans = sndin->channelCount;
    inCount /= nChans;		/* to get sample frames */
    
    SNDAlloc(&sndout,
	     sndin->dataSize,
	     SND_FORMAT_LINEAR_16,
	     sndin->samplingRate,
	     nChans,4);

    SNDGetDataPointer(sndout, (char **)(&outPtr), &outCount, &width);
    outCount /= nChans;		/* to get sample frames */
    
    if (NULL == (outstream = fopen(*++argv,"w")))
      fail("could not open output file ",*argv);
    if (SNDWriteHeader(fileno(outstream),sndout))
      fail("could not write output file ",*argv);
    
    outCount = processSound(inCount, inPtr, outPtr, nChans);
    
    fwrite(outPtr,2*nChans,outCount,outstream);
    
    SNDFree(sndin);
    fclose(outstream);
    SNDFree(sndout);
    
    exit(0);
}

