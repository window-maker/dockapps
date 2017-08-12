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

#ifndef __ULLLIB_H_
#define __ULLLIB_H_

#ifdef HAVE_LONG_LONG_INT

typedef unsigned long long ullong;

#define ullreset(x) (*(x) = 0)
#define ulladd(x, y) (*(x) += *(y))
#define ullsub(x, y) ((long)(*(x) - *(y)))
#define ullparse(x, y) (*(x) = atoll(y))

#else /* ! HAVE_LONG_LONG_INT */

typedef struct {
	unsigned long H;
	unsigned long L;
} ullong;

void ullreset(ullong *);
void ulladd(ullong *, const ullong *);
long ullsub(const ullong *, const ullong *);
void ullparse(ullong *, const char *);

#endif /* HAVE_LONG_LONG_INT */

#endif /* __ULLIB_H_ */
