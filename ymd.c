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
