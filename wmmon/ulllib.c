/*
 *  Unsigned long long arithmetic limited library, tailored for wmmon's
 *  specific needs.
 *
 *  Copyright (c) 2014 Pedro Gimeno Fortea
 *
 *  This file is part of wmmon.
 *
 *  wmmon is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  wmmon is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with wmmon; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef HAVE_LONG_LONG_INT

#include "ulllib.h"

void ullreset(ullong *target) {
	target->H = target->L = 0;
}

void ulladd(ullong *target, const ullong *toadd) {
	unsigned long tmpL = toadd->L, tmpH = toadd->H;
	target->L += tmpL;

	/* Carry if the result is less than one of the operands */
	target->H += tmpH + (target->L < tmpL);
}

long ullsub(const ullong *a, const ullong *b) {
	/* Will wrap around correctly if necessary. Result is assumed to
	   fit a signed long. */
	/* assert((a->H == b->H && a->L >= b->L)
	          || (a->H == b->H + 1 && a->L < b->L)); */
	return a->L - b->L;
}

void ullparse(ullong *target, const char *str) {
	ullong tmp;

	ullreset(target);
	while (*str >= '0' && *str <= '9') {
		tmp = *target;
		ulladd(target, target);	/* *2 */
		ulladd(target, target);	/* *4 */
		ulladd(target, &tmp);	/* *5 */
		ulladd(target, target);	/* *10 */
		tmp.H = 0;
		tmp.L = *str - '0';
		ulladd(target, &tmp);	/* + digit */
		++str;
	}
}

#endif /* HAVE_LONG_LONG_INT */

typedef int make_iso_compilers_happy;

