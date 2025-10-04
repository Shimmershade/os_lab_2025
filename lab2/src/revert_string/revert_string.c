#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
	char t[100];

    int len = strlen(str);
    int i = 0;

    while (len > 0) t[i++] = str[len-- - 1];
    t[i] = '\0';

    strcpy(str, t);
}

