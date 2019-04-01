// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>

//Windows Service not yet supported. 
int configureAsAService(void)
{
    printf("Disabled: %s\n", __func__);
    return 0;
}


void waitForUserInput(void)
{
	int c;
    printf("Disabled: %s\n", __func__);
    //(void)printf("Press return to exit the application. \r\n");
    (void)printf("Press 'q' to exit the application. \r\n");

	do
	{
	    c = getchar();
		//printf("Disabled: %s char=%02x<%c>\r\n", __func__, c, c);
	}
	while(c != 'q');
}
