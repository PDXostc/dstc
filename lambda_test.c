#include <stdio.h>
#include "dstc.h"

void lambda_test(int a, char b) {
    printf("a[%d]+b[%d]=%d\n", a, b&0xFF, a+b);
    return;
}

DSTC_SERVER(lambda_test, int, char)


