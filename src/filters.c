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
#include "sms.h"

/*! \brief coefficient for pre_emphasis filter */
#define SMS_EMPH_COEF    .9   

/* pre-emphasis filter function, it returns the filtered value   
 *
 * float fInput;   sound sample
 */
float PreEmphasis (float fInput)
{
	static float fLastValue = 0;
	float fOutput = 0;
  
	fOutput = fInput - SMS_EMPH_COEF * fLastValue;
	fLastValue = fOutput;
  
	return (fOutput);
}

/* de-emphasis filter function, it returns the filtered value 
 *
 * float fInput;   sound input
 */
float DeEmphasis (float fInput)
{
	static float fLastValue = 0;
	float fOutput = 0;
  
	fOutput = fInput + SMS_EMPH_COEF * fLastValue;
	fLastValue = fInput;
  
	return(fOutput);
}
