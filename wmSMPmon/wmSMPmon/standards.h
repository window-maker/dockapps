/*######################################################################
  #                                                                    #
  # This file contains the definitions of the functions that wmSMPmon  #
  # uses to determine CPU load, memory and swap information.           #
  # All functions should be implemented by any OS dependent module     #
  # that is added to wmSMPmon. See sysinfo-linux.c as an example.      #
  #                                                                    #
  # (c) 2004 Thomas Ribbrock <emgaron@gmx.net>                         #
  #                                                                    #
  # This file is placed under the conditions of the GNU Library        #
  # General Public License, version 2, or any later version.           #
  # See file COPYING for information on distribution conditions.       #
  #                                                                    #
  ######################################################################*/

#ifndef WMSMP_STANDARDS_H
#define WMSMP_STANDARDS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

/*###### Valeurs de test ################################################*/
#define	VRAI	1
#define FAUX	0
#define OK	0
#define ERREUR	-1

/*###### Buffer Sizes ###################################################*/
#define SMLBUFSIZ   256
#define BIGBUFSIZ  2048

/*###### Image Size Definitions #########################################*/
#define	TAILLE_T 44 /* Width Graph */
#define	HAUTEUR  31 /* Max. height of CPU Load Bar+Graph */

/* Program name used in error messages */
char *Myname;

/* NumCPUs_DoInit returns the number of CPUs present in the system and
   performs any initialization necessary for the sysinfo-XXX module */
unsigned int NumCpus_DoInit(void);

/* Get_CPU_Load returns an array of CPU loads, one for each CPU, scaled
   to HAUTEUR. The array is defined and allocated by the main program
   and passed to the function as '*load'. The number of CPUs present
   is given in 'Cpu_tot' */
unsigned int *Get_CPU_Load(unsigned int *load, unsigned int Cpu_tot);

/* return current memory/swap usage on a scale from 0-100 */
unsigned int Get_Memory(void);
unsigned int Get_Swap(void);

#endif /* WMSMP_STANDARDS_H */
