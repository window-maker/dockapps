/*
 * longrun.h - longrun module header file
 *
 * Copyright (c) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

void longrun_init(char *cpuid_dev, char *msr_dev);
void longrun_get_stat(int *percent, int *flags, int *mhz, int *voltz);
