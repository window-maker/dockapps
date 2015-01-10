/****************************************************************
 *  File:     wmcalcswitch.c
 *  Version:  0.21
 *  Date:     November 1, 2000
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
      This file contains system level functions, such as read/write of
      the config file, character map boundaries, etc.

    Change History:
    Date       Modification
    11/1/00    Updated Function headers, cleaned up comments some.

 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wmcalc_err.h"
#include "wmcalc_c.h"
#include "wmcalc_f.h"

/****************************************************************
 *  Function:     ExecFunc
 ****************************************************************
    Description:
      This function determines which button was pressed, and performs
      the appropriate function.

    Change History:
    Date       Modification
    11/01/00   Function header updated
 ****************************************************************/
void ExecFunc( int val ) {
  extern int Verbose;

  if (Verbose) printf("Execute function for button %d\n", val);

  switch(val) {
  case 100:
    clearcalc();
    break;
  case 101:
    sqrtnum();
    break;
  case 102:
    charkey('7');
    break;
  case 103:
    charkey('8');
    break;
  case 104:
    charkey('9');
    break;
  case 105:
    divnums();
    break;
  case 106:
    sqrnum();
    break;
  case 107:
    charkey('4');
    break;
  case 108:
    charkey('5');
    break;
  case 109:
    charkey('6');
    break;
  case 110:
    multnums();
    break;
  case 111:
    // scinotation();
    startcalc();
    break;
  case 112:
    charkey('1');
    break;
  case 113:
    charkey('2');
    break;
  case 114:
    charkey('3');
    break;
  case 115:
    subtnums();
    break;
  case 116:
    chgsignnum();
    break;
  case 117:
    charkey('0');
    break;
  case 118:
    charkey('.');
    break;
  case 119:
    equalfunc();
    break;
  case 120:
    addnums();
    break;
  case 200:
    clrallmem();
    break;
  case 201:
    sqrtnum();
    //    userdef201();
    break;
  case 202:
    recallmem(7);
    break;
  case 203:
    recallmem(8);
    break;
  case 204:
    recallmem(9);
    break;
  case 205:
    divnums();
    //    userdef205();
    break;
  case 206:
    sqrnum();
    //    userdef206();
    break;
  case 207:
    recallmem(4);
    break;
  case 208:
    recallmem(5);
    break;
  case 209:
    recallmem(6);
    break;
  case 210:
    multnums();
    //    userdef210();
    break;
  case 211:
    startcalc();
    break;
  case 212:
    recallmem(1);
    break;
  case 213:
    recallmem(2);
    break;
  case 214:
    recallmem(3);
    break;
  case 215:
    subtnums();
    //    userdef215();
    break;
  case 216:
    chgsignnum();
    break;
  case 217:
    recallmem(0);
    break;
  case 218:
    //    userdef218();
    break;
  case 219:
    equalfunc();
    break;
  case 220:
    addnums();
    //    userdef220();
    break;
  case 300:
    clearnum();
    break;
  case 301:
    sqrtnum();
    //    userdef301();
    break;
  case 302:
    stormem(7);
    break;
  case 303:
    stormem(8);
    break;
  case 304:
    stormem(9);
    break;
  case 305:
    divnums();
    //    userdef305();
    break;
  case 306:
    sqrnum();
    //    userdef306();
    break;
  case 307:
    stormem(4);
    break;
  case 308:
    stormem(5);
    break;
  case 309:
    stormem(6);
    break;
  case 310:
    multnums();
    //    userdef310();
    break;
  case 311:
    startcalc();
    break;
  case 312:
    stormem(1);
    break;
  case 313:
    stormem(2);
    break;
  case 314:
    stormem(3);
    break;
  case 315:
    subtnums();
    //    userdef315();
    break;
  case 316:
    chgsignnum();
    break;
  case 317:
    stormem(0);
    break;
  case 318:
    //    userdef318();
    break;
  case 319:
    equalfunc();
    break;
  case 320:
    addnums();
    //    userdef320();
    break;
  } /* End of switch statement */

} /* End of function ExecFunc() *********************************/

/****************************************************************
 *  Function:     getboundaries
 ****************************************************************
    Description:
      This function returns the x,y boundaries for each character
      that is to be displayed on the display.

      There must be a better way to do this, as by changing the file
      charmap.xpm, one may have to adjust these constants.

    Change History:
    Date       Modification
    11/01/00   Function header updated
    10/30/00   Added characters for the Memory indicator bar.
 ****************************************************************/
ButtonArea getboundaries(char ch) {
  ButtonArea xybounds;

  switch (ch) {
  case '0':
    xybounds.x = 1;    xybounds.i = 6;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '1':
    xybounds.x = 7;    xybounds.i = 12;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '2':
    xybounds.x = 13;   xybounds.i = 18;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '3':
    xybounds.x = 19;   xybounds.i = 24;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '4':
    xybounds.x = 25;   xybounds.i = 30;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '5':
    xybounds.x = 31;   xybounds.i = 36;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '6':
    xybounds.x = 37;   xybounds.i = 42;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '7':
    xybounds.x = 43;   xybounds.i = 48;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '8':
    xybounds.x = 49;   xybounds.i = 54;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '9':
    xybounds.x = 55;   xybounds.i = 60;
    xybounds.y = 1;    xybounds.j = 8;
    break;
  case '+':
    xybounds.x = 55;   xybounds.i = 60;
    xybounds.y = 28;   xybounds.j = 35;
    break;
  case '-':
    xybounds.x = 49;   xybounds.i = 54;
    xybounds.y = 28;   xybounds.j = 35;
    break;
  case '.':
    xybounds.x = 7;    xybounds.i = 12;
    xybounds.y = 10;   xybounds.j = 17;
    break;
  case 'e':
    xybounds.x = 1;    xybounds.i = 6;
    xybounds.y = 37;   xybounds.j = 44;
    break;
  case 'i':
    xybounds.x = 7;    xybounds.i = 12;
    xybounds.y = 37;    xybounds.j = 44;
    break;
  case 'j':
    xybounds.x = 13;    xybounds.i = 18;
    xybounds.y = 37;   xybounds.j = 44;
    break;
  case 'n':
    xybounds.x = 19;   xybounds.i = 24;
    xybounds.y = 37;   xybounds.j = 44;
    break;
  case 'a':
    xybounds.x = 25;   xybounds.i = 30;
    xybounds.y = 37;   xybounds.j = 44;
    break;
  case 'f':
    xybounds.x = 31;   xybounds.i = 36;
    xybounds.y = 37;   xybounds.j = 44;
    break;
  case ' ':
    xybounds.x = 1;    xybounds.i = 6;
    xybounds.y = 10;   xybounds.j = 17;
    break;
  case '=':
    xybounds.x = 55;   xybounds.i = 60;
    xybounds.y = 55;   xybounds.j = 56;
    break;
  case '#':
    xybounds.x = 55;   xybounds.i = 60;
    xybounds.y = 57;   xybounds.j = 58;
    break;
  case '_':
    xybounds.x = 55;   xybounds.i = 60;
    xybounds.y = 59;   xybounds.j = 60;
    break;
  default:
    xybounds.x = 1;    xybounds.i = 6;
    xybounds.y = 10;   xybounds.j = 17;
    break;
  } /* end of switch statement */

  return(xybounds);
} /***** End of function getboundaries() ************************/

/****************************************************************
 *  Function:     write_config
 ****************************************************************
    Description:
      This function updates the configuration file as memory locations
      are updated in the program.  It re-writes the entire file, but
      should ignore all lines that do not start with "Mem".

    Change History:
    Date       Modification
    11/01/00   Function Header updated
    11/05/00   Added Locked Memory Handling
 ****************************************************************/
int write_config(void) {
  extern int Verbose;
  extern double MemArray[];
  extern char configfile[];
  extern char tempfile[];
  extern char *CfgVarList[];
  FILE *fp, *fptmp;
  char *line = NULL;
  int mem_ndx = 0;
  int err_code = OKAY;
  char movefilecmd[2 * CONFIGFILEMAX + 10];  /* make sure enough room in string */

  /* Open current Config file */
  if ((fp = fopen(configfile, "r")) == NULL) {  // Can't find config file
    printf("%s: Cannot create configuration file\n", configfile);
    return(ERR_FILE_NOT_FOUND);
  }

  /* We cannot write to the global config-file... */
  if(!strcmp(configfile, CONFIGGLOBAL)) {
    strcpy(configfile, getenv("HOME"));  // Added to wmbutton by Gordon Fraser, 9/21/01
    strcat(configfile, CONFFILENAME);
  }

  /* Open Temporary File */
  if ((fptmp = fopen(tempfile, "w")) == NULL) {  // Can't open file in /tmp
    fprintf(stderr, "%s: Temporary File Open Failed\n", tempfile);
    strcpy(tempfile, getenv("HOME"));
    strcat(tempfile, "wmcalc.tmp");
    if ((fptmp = fopen(tempfile, "w")) == NULL) {  // Can't open file in HOME
      fprintf(stderr, "%s: Temporary File Open Failed\n", tempfile);
      return(ERR_TMP_FILE_FAILED);
    }
  }

  while ((line = readln(fp)) != NULL) {    // Read Lines in config file
    if (Verbose)  printf("line:%s", line);

    if ((strncmp(line, CfgVarList[MEM_LABEL_0],
		 strlen(CfgVarList[MEM_LABEL_0]) - 1) == 0)) {
      // -1 to generalize to all Mem? strings
      // If we've found a memory entry
      mem_ndx = atoi(line+strlen(CfgVarList[MEM_LABEL_0])-1);
      if ((mem_ndx >= 0) && (mem_ndx <= (NUM_MEM_CELLS - 1))) {
	fprintf(fptmp,  "%s\t%f\n", CfgVarList[mem_ndx],
		                    MemArray[mem_ndx]);
      }
    }
    else {
      fprintf(fptmp, "%s", line);
    }
    free(line);
  } /* End of while loop */

  /* Close open files */
  fclose(fp);
  fclose(fptmp);

  /* Copy temp file over original */
  /* Note:  If changing command, make sure to adjust size of string above!! */
  sprintf(movefilecmd, "mv -f %s %s\n", tempfile, configfile);
  err_code = system(movefilecmd);

  if(Verbose) printf("New config file written.\n");

  return(err_code);
} /***** End of function write_config() *************************/


/****************************************************************
 *  Function:     read_config
 ****************************************************************
    Description:
     This function reads the configuration file on program startup,
     and sets the appropriate configuration options.
     (By default, this is ~/.wmcalc, or a user set value)

    Change History:
    Date       Modification
    11/01/00   Function header updated.
    11/05/00   Added Lcoked Memory Capabilities
 ****************************************************************/
int read_config(void) {
  extern int Verbose;
  extern double MemArray[];
  extern int    MemLock[];
  extern char *CfgVarList[];
  extern char ImagChar;
  extern char configfile[];
  extern char SysCalcCmd[];
  FILE *fp;
  int i = 0;
  int err_code = OKAY;
  char *line   = NULL;
  char  sep_ch = '\t';
  char *cfg_var_ptr = NULL;


  if ((fp = fopen(configfile, "r")) == NULL) {    // Can't find config file
    strcpy(configfile, CONFIGGLOBAL);             // ...so try to open global config
    if ((fp = fopen(configfile, "r")) == NULL) {  // Can't find global config file
      fprintf(stderr, "%s: Configuration File not found\n", configfile);
      return(ERR_FILE_NOT_FOUND);
    } else {
      write_config();                             // if global config opened, save in homedir
    }
  }
  else {
    if (Verbose) printf("%s: Found Configuration File\n", configfile);
  }

  do {                               // Read Lines in config file
    line = readln(fp);
    if (Verbose) printf("Line Read:%s\n", line);
    if (line == NULL) break;         // if end of file, quit
    if ( (line[0] != '#') && (line[0] != '\n')) { /* Ignore comments and
						     blanks */
      if (strchr(line, sep_ch) != NULL) {  /* The line has a tab, so let's
					      see if the variable is
					      understood */
	i = 0;
	/* Loop while we haven't found the variable */
	while (((strncmp(line, CfgVarList[i], strlen(CfgVarList[i]))) != 0)
	       && (i < MAX_LABEL)) {
	  i++;
	}
	/* If we've found the variable, let's set the appropriate value */
	if (i <= MAX_LABEL) {
	  if (Verbose) printf("Variable %s found\n", CfgVarList[i]);

	  /* Point to the 'tab' character, to read the value */
	  cfg_var_ptr = strchr(line, sep_ch);
	  cfg_var_ptr++;  // ++ to avoid tab character itself

	  /* Now set the appropriate variable */
	  switch(i) {
	  case MEM_LABEL_0:
	  case MEM_LABEL_1:
	  case MEM_LABEL_2:
	  case MEM_LABEL_3:
	  case MEM_LABEL_4:
	  case MEM_LABEL_5:
	  case MEM_LABEL_6:
	  case MEM_LABEL_7:
	  case MEM_LABEL_8:
	  case MEM_LABEL_9:
	    /* Set Locked Flag */
	    MemLock[i] = 0;
	    /* Set Memory Element */
	    MemArray[i] = atof(cfg_var_ptr);
	    if (Verbose)
	      printf("Assign Memory Element %d to %f\n", i, MemArray[i]);
	    break;

	  case MEM_LOCK_0:
	  case MEM_LOCK_1:
	  case MEM_LOCK_2:
	  case MEM_LOCK_3:
	  case MEM_LOCK_4:
	  case MEM_LOCK_5:
	  case MEM_LOCK_6:
	  case MEM_LOCK_7:
	  case MEM_LOCK_8:
	  case MEM_LOCK_9:
	    /* Set Locked Flag */
	    MemLock[i - MEM_LOCK_0] = 1;
	    /* Set Memory Element */
	    MemArray[i - MEM_LOCK_0] = atof(cfg_var_ptr);
	    if (Verbose)
	      printf("Assign Memory Element %d to %f\n", i -
		     MEM_LOCK_0, MemArray[i - MEM_LOCK_0]);
	    break;

	  case IMAG_LABEL:
	    /* Get the character that the user wishes to use to
	       represent sqrt(-1) (i or j) */
	    if ((cfg_var_ptr[0] == 'i') || cfg_var_ptr[0] == 'j') {
	      ImagChar = cfg_var_ptr[0];
	    }
	    if (Verbose)
	      printf("Use character '%c' to represent sqrt(-1)\n", ImagChar);
	    break;

	  case CALC_LABEL:
	    /* Set external calculator start command */
	    strcpy(SysCalcCmd, cfg_var_ptr);
	    if (Verbose)
	      printf("Assign Calc Command: %s\n", cfg_var_ptr);
	    break;

	  default:
	    if (Verbose) printf("Unknown Config Variable: %s\n", line);
	    break;
	  }

	}
	/* Otherwise, we don't recognize the variable */
	else {
	  if (Verbose) printf("Unknown Config Variable: %s\n", line);
	}
      }
      /* Otherwise, we had a poorly formatted line in the config file. */
      else {
	if (Verbose) printf("Poorly formatted config file line\n:%s\n", line);
      }
    }
    free(line);
  } while (line != NULL);

  fclose(fp);



  return(err_code);
} /***** End of function read_config **************************/
