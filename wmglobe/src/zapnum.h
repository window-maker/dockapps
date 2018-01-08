/*     WMGlobe 1.3  -  All the Earth on a WMaker Icon
 *     copyright (C) 1998,99,2000,01 Jerome Dumonteil <jerome.dumonteil@linuxfr.org>
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ***************************************************************************/

int zapnum[14][4] = {
    {90, 0, 10, 13},
    {0, 0, 10, 13},
    {10, 0, 10, 13},
    {20, 0, 10, 13},
    {30, 0, 10, 13},
    {40, 0, 10, 13},
    {50, 0, 10, 13},
    {60, 0, 10, 13},
    {70, 0, 10, 13},
    {80, 0, 10, 13},
    {100, 0, 10, 13},
    {110, 0, 10, 13},
    {100, 0, 5, 13},
    {114, 0, 5, 13}
};

/*
 * int zapnum2[14][4] =
 * {
 *      {72, 0, 8, 13},
 *      {0, 0, 8, 13},
 *      {8, 0, 8, 13},
 *      {16, 0, 8, 13},
 *      {24, 0, 8, 13},
 *      {32, 0, 8, 13},
 *      {40, 0, 8, 13},
 *      {48, 0, 8, 13},
 *      {56, 0, 8, 13},
 *      {64, 0, 8, 13},
 *      {80, 0, 8, 13},
 *      {88, 0, 8, 13},
 *      {80, 0, 6, 13},
 *      {92, 0, 6, 13}
 * };
 */

int platd[4][2] = {
    {2, 12},
    {6, 12},
    {16, 12},
    {26, 12}
};
int platm[2][2] = {
    {42, 12},
    {52, 12}
};
int plongd[4][2] = {
    {2, 36},
    {6, 36},
    {16, 36},
    {26, 36}
};
int plongm[2][2] = {
    {42, 36},
    {52, 36}
};
