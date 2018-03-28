#include "utils.h"


int conversion_toInt(long double a, int* b) {
	if (a > INT_MAX) {
		return -1;
	}
	*b = (int)a;
	return 0;
}

int conversion_toLong(long double a, long* b) {
	if (a > LONG_MAX) {
		return -1;
	}
	*b = (int)a;
	return 0;
}
