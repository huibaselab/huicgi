

#include "cgiutil.h"
#include <hutil.h>


void CCgiUtil::NullToZero (HSTRR str) {

    str = str == "NULL" ? "0" : str;

}


HSTR CCgiUtil::TypeToCoin (HCSTRR strType) {

    HINT n = HCStr::stoi(strType);

    switch (n){
    case 0:
        return "BTC";
        break;
    case 1:
        return "LKT";
        break;
    case 2:
        return "LTC";
        break;
    case 3:
        return "ETH";
        break;
    case 4:
        return "ETC";
        break;
    case 5:
        return "ZEC";
        break;
    case 6:
        return "XMR";
        break;
    case 7:
        return "BYC";
        break;
    case 8:
        return "STORJ";
        break;
    case 9:
        return "BURST";
        break;
    }

    HASSERT_THROW_MSG(false, "error type", INVL_PARA);
}


HSTR CCgiUtil::CoinToType (HCSTRR strCoin) {

    if (strCoin == "BTC") {
        return "0";
    }

    if (strCoin == "LKT") {
        return "1";
    }

    if (strCoin == "LTC") {
        return "2";
    }

    if (strCoin == "ETH") {
        return "3";
    }

    if (strCoin == "ETC") {
        return "4";
    }

    if (strCoin == "ZEC") {
        return "5";
    }

    if (strCoin == "XMR") {
        return "6";
    }

    if (strCoin == "BYC") {
        return "7";
    }

    if (strCoin == "STORJ") {
        return "8";
    }

    if (strCoin == "BURST") {
        return "9";
    }

    return "-1";
}


HSTR CCgiUtil::WebFloat (HCSTRR str) {

    HDOUBLE d = HCStr::stod(str);
    return WebFloat(d);

}

HSTR CCgiUtil::WebFloat (HDOUBLE d) {

    HCHAR buf[HLEN2_C] = {0};
    sprintf(buf, "%.2f", d);

    return buf;

}


HSTR CCgiUtil::WebFloat4 (HCSTRR str) {

    HDOUBLE d = HCStr::stod(str);
    return WebFloat4(d);

}


HSTR CCgiUtil::WebFloat4 (HDOUBLE d) {

    HCHAR buf[HLEN2_C] = {0};
    sprintf(buf, "%.4f", d);

    return buf;

}


HSTR CCgiUtil::WebFloat6(HCSTRR str) {

    HDOUBLE d = HCStr::stod(str);
    return WebFloat6(d);

}


HSTR CCgiUtil::WebFloat6(HDOUBLE d) {

    HCHAR buf[HLEN2_C] = {0};
    sprintf(buf, "%.6f", d);
    return buf;

}


HSTR CCgiUtil::DoubleStr (HCSTRR str, HINT n) {
    size_t pos = str.find(".");

    if (pos == HSTR::npos) {
        return str;
    }

    HSTR str1 = str.substr(0, pos), str2;
    HINT fn = str.length() - pos;

    if (n >= fn) {

        str2 = str.substr(pos + 1, str.length() - 1 - pos);

    } else {

        str2 = str.substr(pos + 1, n);

    }

    return str1 + "." + str2;

}


HSTR CCgiUtil::DoubleStr (HDOUBLE d, HINT n) {

    HSTR str = HCStr::dtos(d);
    return DoubleStr(str, n);

}

