#include "common.h"


bool check_phone(HUIBASE::HSTR& phone)
{
	if (phone.empty())
		return false;
	if (not (phone.length() > 4 && phone.length() < 20))
		return false;

	HUIBASE::HCSTR cstr = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*#";
	return phone.find_first_not_of(cstr) == HUIBASE::HSTR::npos ? true : false;
}