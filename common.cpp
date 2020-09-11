#include "common.h"

bool CL::isPowerOf2(int i)
{
	return i > 0 && !(i & (i - 1));
}
