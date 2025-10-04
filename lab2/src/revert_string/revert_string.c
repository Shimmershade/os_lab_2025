#include "revert_string.h"
#include <string.h>
#include <stdlib.h>

void RevertString(char *str)
{
	char *t = malloc(sizeof(char) * (strlen(str) + 1));

    int len = strlen(str);
    int i = 0;

    while (len > 0) t[i++] = str[len-- - 1];
    t[i] = '\0';

    strcpy(str, t);
	free(t);
}

