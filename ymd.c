// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 2 as
// published by the Free Software Foundation.  You may not use, modify or
// distribute this program under any other version of the GNU General
// Public License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// IF YOU ARE UNABLE TO WORK WITH GPL2, CONTACT ME.
//-------------------------------------------------------------------

#include "msutil.h"

#define MAR01_1900  693976
#define JAN01_1970  719484
#define	MAR01_1970  719543
#define	BASE_DATE   MAR01_1900

unsigned ymd2day(YMD x)
{
    if (x.mm > 2)  x.mm -= 3; else x.mm += 9, --x.yyyy;
    return  (x.yyyy*1461)/4 + (x.mm*979 + 17)/32 + x.dd - BASE_DATE;
}

YMD day2ymd(unsigned nDate)
{
    YMD		x;
    nDate += BASE_DATE;

    x.yyyy = (nDate * 4  - 1)/ 1461;
    nDate -= (x.yyyy * 1461)/ 4;
    x.mm = (nDate * 32 - 17) / 979;
    x.dd = nDate - (x.mm * 979 + 17) / 32;

    if (x.mm > 9)   x.mm -= 9, ++x.yyyy; 
    else	    x.mm += 3;

    return  x;
}
