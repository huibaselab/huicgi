


#ifndef __CGI_UTIL_H__
#define __CGI_UTIL_H__


#include <huibase.h>

using namespace HUIBASE;

class CCgiUtil {
public:
    CCgiUtil () { }

public:
    static void NullToZero (HSTRR str);

    static HSTR TypeToCoin (HCSTRR strType);

    static HSTR CoinToType (HCSTRR strCoin);

    static HSTR WebFloat (HCSTRR str);

    static HSTR WebFloat (HDOUBLE d);

    static HSTR WebFloat4 (HCSTRR str);

    static HSTR WebFloat4 (HDOUBLE d);

    static HSTR WebFloat6 (HCSTRR str);

    static HSTR WebFloat6 (HDOUBLE d);

    static HSTR DoubleStr (HCSTRR str, HINT n);

    static HSTR DoubleStr (HDOUBLE d, HINT n);

};


#endif //__CGI_UTIL_H__

