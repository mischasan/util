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

int ymd2day(YMD x)
{
    if (x.mm > 2)  x.mm -= 3; else x.mm += 9, --x.yyyy;

    // This is sufficient back to 1500-03-01. 1753 is the start of Gregorian Calendar.
    int days = (x.yyyy*1461)/4 + (x.mm*979 + 17)/32 + x.dd - BASE_DATE;
    return days 
            + (days < -72871-3) + (days < -36465-2) + (days < 59-1) 
            - (days >  73107+1) - (days > 109631+2); // Mar 1 (1700,1800,1900,2100,2200)
}

YMD day2ymd(int nDate)
{
    YMD		x;
    nDate -= -BASE_DATE
            + (nDate < -72871 ? 3 : nDate < -36465 ? 2 : nDate < 59 ? 1 : nDate > 109631 ? -2 : nDate > 73107 ? -1 : 0);

    x.yyyy = (nDate * 4  - 1)/ 1461;
    nDate -= (x.yyyy * 1461)/ 4;
    x.mm = (nDate * 32 - 17) / 979;
    x.dd = nDate - (x.mm * 979 + 17) / 32;

    if (x.mm > 9)   x.mm -= 9, ++x.yyyy; 
    else	    x.mm += 3;

    return  x;
}
