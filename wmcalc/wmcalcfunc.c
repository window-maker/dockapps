/****************************************************************
 *  File:     wmcalcfunc.c
 *  Version:  0.21
 *  Author:   Edward H. Flora <ehflora@access1.net>
 *
 *  This file is a part of the wmcalc application.  As such, this
 *  file is licensed under the GNU General Public License, version 2.
 *  A copy of this license may be found in the file COPYING that should
 *  have been distributed with this file.  If not, please refer to
 *  http://www.gnu.org/copyleft/gpl.html for details.
 *
 ****************************************************************
    Description:
      This file contains the code for the actual calculator functions,
      such as a add, subt, clear, etc.

    Change History:
    Date       Modification
    11/03/00   File Header added

 ****************************************************************/

/***** Includes *************************************************/
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "wmcalc_c.h"
#include "wmcalc_err.h"
#include "wmcalc_f.h"


/****************************************************************
 *  Function:     clearcalc
 ****************************************************************
    Description:
      This function will clear the calculator display, and internal
      flags.

    Change History:
    Date       Modification
    11/03/00   Function header added
    11/04/00   Replaced magic numbers with DISPSIZE
 ****************************************************************/
void clearcalc(void) {
  extern int    StrCnt;
  extern char   PlusMinusFlag;
  extern int    ExpFlag;
  extern int    DecFlag;
  extern double RegisterA;
  extern double RegisterB;
  extern char   DispString[];
  extern char   OpFlag;
  int i;

  RegisterA = 0.0;
  RegisterB = 0.0;
  ExpFlag = 0;
  DecFlag = 0;
  PlusMinusFlag   = '+';
  StrCnt = 0;
  for (i=0; i < DISPSIZE; i++) {
    DispString[i] = ' ';
  }
  OpFlag = ' ';
} /***** End of function clearcalc() *****************************/

/****************************************************************
 *  Function:     clearnum
 ****************************************************************
    Description:
     Clears the current number being entered.

    Change History:
    Date       Modification
    11/03/00   Updated function header
    11/04/00   Replaced magic numbers with DISPSIZE
 ****************************************************************/
void clearnum(void) {
  extern int    StrCnt;
  extern char   PlusMinusFlag;
  extern int    ExpFlag;
  extern int    DecFlag;
  extern double RegisterA;
  extern char   DispString[];
  int i;

  RegisterA = 0.0;
  ExpFlag = 0;
  DecFlag = 0;
  PlusMinusFlag   = '+';
  StrCnt = 0;
  for (i=0; i < DISPSIZE; i++) {
    DispString[i] = ' ';
  }

} /***** End of function clearnum() ******************************/

/****************************************************************
 *  Function:     charkey
 ****************************************************************
    Description:
      Add characters to the number being entered.

    Change History:
    Date       Modification
    11/03/00   Updated function header
    11/04/00   Replaced magic numbers with DISPSIZE
 ****************************************************************/
void charkey(char ch) {
  extern int Verbose;
  extern int StrCnt;
  extern int DecFlag;
  extern double RegisterA, RegisterB;
  extern char DispString[];
  int i;

  if (Verbose) printf("In function charkey\n");

  if (StrCnt < DISPSIZE) {
    if (ch == '.') {
      if (DecFlag == 0) {
	for (i = 1; i < DISPSIZE; i++)
	  DispString[i-1] = DispString[i];
	DecFlag = 1;
	StrCnt++;
	DispString[DISPSIZE - 1] = ch;
      }
    }
    else {
      for (i = 1; i < DISPSIZE; i++)
	DispString[i-1] = DispString[i];
      DispString[9] = ch;
      StrCnt++;
    }
  } /* endif (StrCnt < DISPSIZE) */
  else if (StrCnt == CALCDONE) {
    RegisterB = RegisterA;
    clearnum();
    if (ch == '.') {
      for (i = 1; i < DISPSIZE; i++)
	DispString[i-1] = DispString[i];
      DecFlag = 1;
      StrCnt++;
      DispString[DISPSIZE -1] = ch;
    }
    else {
      for (i = 1; i < DISPSIZE; i++)
	DispString[i-1] = DispString[i];
      DispString[DISPSIZE - 1] = ch;
      StrCnt++;
    }
  } /* endif (StrCnt == CALCDONE) */
  RegisterA = atof(DispString);
} /***** End of Function charkey() *******************************/


/****************************************************************
 *  Function:     chgsignnum
 ****************************************************************
    Description:
     Change the sign of the number currently being entered

    Change History:
    Date       Modification
    11/03/00   Updated Function header
 ****************************************************************/
void chgsignnum(void) {
  extern int Verbose;
  extern double RegisterA;
  extern char DispString[];

  if (Verbose) printf("In function chgsignnum\n");

  RegisterA = -RegisterA;
  sprintf(DispString, "%10.5g", RegisterA);

} /***** End of function chgsignnum() *****************************/


/****************************************************************
 *  Function:     sqrnum
 ****************************************************************
    Description:
      Square the number in RegisterA
    Change History:
    Date       Modification
    11/03/00   Updated Function header
 ****************************************************************/
void sqrnum(void) {
  extern int Verbose;
  extern int StrCnt;
  extern double RegisterA;
  extern char DispString[];
  extern int  ImgFlag;

  if (Verbose) printf("In function sqrnum\n");
  RegisterA = atof(DispString);
  RegisterA = pow(RegisterA, 2.0);
  if (ImgFlag) {
    RegisterA = -RegisterA;
    ImgFlag = 0;
  }
  sprintf(DispString, "%10.5g", RegisterA);
  StrCnt = CALCDONE;

} /***** End of Function sqrnum() *******************************/

/****************************************************************
 *  Function:     sqrtnum
 ****************************************************************
    Description:
      Take the square root of the number in RegisterA
    Change History:
    Date       Modification
    11/03/00   Updated function header
    11/04/00   Replaced magic numbers with DISPSIZE
 ****************************************************************/
void sqrtnum(void) {
  extern int Verbose;
  extern int StrCnt;
  extern double RegisterA;
  extern int ImgFlag;
  extern char ImagChar;
  extern char DispString[];
  int i;

  if (Verbose) printf("In function sqrtnum\n");
  RegisterA = atof(DispString);
  if (RegisterA >= 0) {
    RegisterA = pow(RegisterA, 0.5);
    sprintf(DispString, "%10.5g", RegisterA);
  }
  else {
    RegisterA = pow(-RegisterA, 0.5);
    ImgFlag = 1;
    sprintf(DispString, "%10.4g", RegisterA);
    for(i=1; i < DISPSIZE - 1; i++)
      DispString[i] = DispString[i+1];
    DispString[DISPSIZE - 1] = ImagChar;
  }
  StrCnt = CALCDONE;

} /***** End of function sqrtnum() ********************************/

/****************************************************************
 *  Function:     addnums
 ****************************************************************
    Description:
      Add the number in Registers A to Register B.
    Change History:
    Date       Modification
    11/03/00   Updated Function header
 ****************************************************************/
void addnums(void) {
  extern int Verbose;
  extern int StrCnt;
  extern double RegisterA, RegisterB;
  extern char OpFlag;

  if(Verbose) printf("In function addnums: ");

  if(OpFlag != ' ') {
    equalfunc();
  }
  else {
    if(Verbose) printf("%g + ?? = ??\n", RegisterB);
    RegisterB = RegisterA;
  }

  StrCnt = CALCDONE;
  OpFlag = '+';
} /***** End of function addnums() *********************************/


/****************************************************************
 *  Function:     subtnums
 ****************************************************************
    Description:
      Subtract current number (in RegisterA) from accumulated total.
    Change History:
    Date       Modification
    11/03/00   Updated Function header
 ****************************************************************/
void subtnums(void) {
  extern int Verbose;
  extern int StrCnt;
  extern double RegisterA, RegisterB;
  extern char OpFlag;

  if(Verbose) printf("In function subtnums: ");

  if (OpFlag != ' ') {
    equalfunc();
  }
  else {
    if(Verbose) printf("%g - ?? = ??\n", RegisterB);
    RegisterB = RegisterA;
  }

  StrCnt = CALCDONE;
  OpFlag = '-';

} /*****  End of function subtnums() *****************************/

/****************************************************************
 *  Function:     multnums
 ****************************************************************
    Description:
      Multiply number in RegisterA by the accumulated total.
    Change History:
    Date       Modification
    11/03/00   Updated function header
 ****************************************************************/
void multnums(void) {
  extern int Verbose;
  extern int StrCnt;
  extern char OpFlag;
  extern double RegisterA, RegisterB;

  if(Verbose) printf("In function multnums: ");

  if(OpFlag != ' ') {
    equalfunc();
  }
  else {
    if(Verbose) printf("%g * ?? = ??\n", RegisterB);
    RegisterB = RegisterA;
  }

  StrCnt = CALCDONE;
  OpFlag = '*';
} /***** End of function multnums() *****************************/

/****************************************************************
 *  Function:     divnums
 ****************************************************************
    Description:
      Divide the accumulated total by the current number in RegisterA

    Change History:
    Date       Modification
    11/04/00   Updated Function Header
 ****************************************************************/
void divnums(void) {
  extern int Verbose;
  extern int StrCnt;
  extern double RegisterA, RegisterB;
  extern char OpFlag;

  if(Verbose) printf("In function divnums: ");

  if(OpFlag != ' ') {
    equalfunc();
  }
  else {
    if(Verbose) printf("%g / ?? = ??\n", RegisterB);
    RegisterB = RegisterA;
  }

  StrCnt = CALCDONE;
  OpFlag = '/';
} /* End of Function divnums() ********************************/

/****************************************************************
 *  Function:
 ****************************************************************
    Description:
      Calculate result of entered calculation.
    Change History:
    Date       Modification
    11/04/00   Updated Function Header
 ****************************************************************/
void equalfunc (void) {
  extern int Verbose;
  extern int StrCnt;
  extern char DispString[];
  extern double RegisterA, RegisterB;
  extern char OpFlag;

  if (Verbose) printf("Equal Function: Operation >> %c <<\n", OpFlag);
  switch (OpFlag) {
  case '+':
    RegisterA = RegisterB + RegisterA;
    sprintf(DispString, "%10.5g", RegisterA);
    break;
  case '-':
    RegisterA = RegisterB - RegisterA;
    sprintf(DispString, "%10.5g", RegisterA);
    break;
  case '*':
    RegisterA = RegisterB * RegisterA;
    sprintf(DispString, "%10.5g", RegisterA);
    break;
  case '/':
    RegisterA = RegisterB / RegisterA;
    sprintf(DispString, "%10.5g", RegisterA);
    break;
  default:
    break;
  }
  OpFlag = ' ';
  StrCnt = CALCDONE;
} /***** End of function equalfunc() ******************************/


/****************************************************************
 *  Function:     clrallmem
 ****************************************************************
    Description:
      Clear all the values in memory
    Change History:
    Date       Modification
    11/04/00   Updated Function Header
    11/04/00   Incorporated clrmem() function into this one, to
               optimize code.
 ****************************************************************/
void clrallmem(void) {
  extern int Verbose;
  extern double MemArray[];
  extern int    MemLock[];
  int i;

  if (Verbose) printf("Clear All Memory Function\n");

  for (i = 0; i < NUM_MEM_CELLS; i++) {
    if (MemLock[i] != 1) {
      MemArray[i] = 0.0;
      if (Verbose) printf(" %f  ", MemArray[i]);
    }
  }
  if (Verbose) printf("\n");

  write_config();
} /*****  End of function clrallmem() ****************************/


/****************************************************************
 *  Function:     stormem
 ****************************************************************
    Description:
      Store value to memory cell #N

    Change History:
    Date       Modification
    11/04/00   Updated function header
    11/05/00   Added Locked Memory capabilities
 ****************************************************************/
void stormem(int mem_loc) {
  extern double MemArray[];
  extern int    MemLock[];
  extern int    Verbose;
  extern double RegisterA;
  int i;

  if (Verbose)
    printf("Store Value %f in Memory Cell %d\nMemory:", RegisterA, mem_loc);

  if (MemLock[mem_loc] != 1) {
    MemArray[mem_loc] = RegisterA;
    write_config();
  }
  else {
    if (Verbose) printf("Memory location %d Locked at %f\n",
			mem_loc, MemArray[mem_loc]);
  }

  if (Verbose) {
    for (i = 0; i < NUM_MEM_CELLS; i++)
      printf(" %f  ", MemArray[i]);
    printf("\n");
  }

} /*****  End of function stormem() ******************************/


/****************************************************************
 *  Function:     recallmem
 ****************************************************************
    Description:
      Store value to memory cell #N
    Change History:
    Date       Modification
    11/04/00   Updated function header
 ****************************************************************/
void recallmem(int mem_loc) {
  extern double MemArray[];
  extern int    Verbose;
  extern double RegisterA;
  extern char   DispString[];
  int i;

  if (Verbose)
    printf("Recall Value in Memory Cell %d\nMemory:", mem_loc);

  RegisterA = MemArray[mem_loc];

  sprintf(DispString, "%10.5g", RegisterA);

  if (Verbose) {
    for (i = 0; i < NUM_MEM_CELLS; i++)
      printf(" %f  ", MemArray[i]);
    printf("\n");
  }
} /*****  End of function recallmem() ***************************/


/****************************************************************
 *  Function:     startcalc
 ****************************************************************
    Description:
      Change the sign of the number currently being entered
    Change History:
    Date       Modification
    11/04/00   Updated function header
 ****************************************************************/
void startcalc(void) {
  extern int Verbose;
  extern char SysCalcCmd[];

  if (Verbose)
    fprintf(stderr, "Starting external calculator %s\n", SysCalcCmd);

  if (system(SysCalcCmd) == -1)
    fprintf(stderr, "%s returned an error.\n", SysCalcCmd);
} /*****  End of function startcalc *****************************/
