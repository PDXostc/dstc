#include "dstc.h"
#include <stdio.h>

void lambda_test(int a, char b) {
    printf("a[%d]+b[%d]=%d\n", a, b&0xFF, a+b);
    return;
}

CLIENT_FUNCTION(lambda_test, int, char)
