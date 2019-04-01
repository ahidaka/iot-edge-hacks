// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>

#include "simulated_device_cloud_upload_config_service.h"
#include "gateway.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/vector.h"
#include "azure_c_shared_utility/platform.h"

// need check
#include <signal.h>
#include <errno.h> // errno, EINTR
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "EoControl.c"

static const char version[] = "\n@ azure-iot-edge-v1-dd Version 1.00 \n";

static const int ddebug = 0;
#define DDEBUG if(ddebug > 0)
//#define DDEBUG if(0)

#define EO_DIRECTORY "/var/tmp/dpride"
#define AZURE_PID_FILE "azure.pid"

#define DOMAIN_LENGTH (256)

#define SIGENOCEAN (SIGRTMIN + 6)

typedef FILE* HANDLE;
typedef char TCHAR;
typedef int BOOL;

typedef enum dwt_statrus {
        NoEntry = 0,
        NoData = 1,
        DataExists = 2
} EventStatus;

void EoSignalAction(int signo, void (*func)(int));
void ExamineEvent(int Signum, siginfo_t *ps, void *pt);
char *EoMakePath(char *Dir, char *File);
INT EoReflesh(void);
EO_DATA *EoGetDataByIndex(int Index);

extern bool EoGetData(const char *pointName, char *data);

/////////////////////////////////////////

#define DWT_TABLE_SIZE (1024) // 0~999
#define DWT_DATA_SIZE (8)

typedef struct _dwt_table
{
    union _utag
    {
        long long ll;
        char str[DWT_DATA_SIZE];
    } key;
    EventStatus status;
    char data[DWT_DATA_SIZE];
} DWT_TABLE;

static int DFlag;

static DWT_TABLE dwt_table[DWT_TABLE_SIZE];
static int dwt_lastindex;

int dwt_index(const char *key)
{
    int index = -1;
    int i;
    DWT_TABLE *pt = &dwt_table[0];
    long long ll = 0;

    strncpy((char *)&ll, key, 7);
    for (i = 0; i < DWT_TABLE_SIZE; i++)
    {
        if (ll == pt->key.ll)		
        {
	    DDEBUG
            printf("find %d: %s=%s (%016llx)\n",
                   i, key, pt->data, pt->key.ll);
            index = i;
            break;
        }
        else if (0LL == pt->key.ll)
        {
	    DDEBUG
            printf("last %d:\n", i);
            dwt_lastindex = i;
            break;
        }
	DDEBUG
	printf("**%s: not found %d: %s(%016llx):%s(%016llx)\n",
	       __func__, i, key, ll, pt->key.str, pt->key.ll);
        pt++;
    }
    return index;
}

int PointPut(const char *key, char *data)
{
    DWT_TABLE *pt = &dwt_table[0];
    int index = dwt_index(key);
    long long ll = 0LL;
    strncpy((char *)&ll, key, 7);

    if (index == -1) //not found, add new entry
    {
        index = dwt_lastindex++;
    }
    pt = &dwt_table[index];
    pt->key.ll = ll;
    strncpy(pt->data, data, DWT_DATA_SIZE - 1);
    pt->data[DWT_DATA_SIZE - 1] = '\0';
    pt->status = DataExists;

    return index;
}

int PointGet(const char *key, char *data)
{
    DWT_TABLE *pt = &dwt_table[0];
    int index = dwt_index(key);

    DDEBUG
    printf(">>%s index=%d key=%s data=%s status=%d\r\n",
    	__func__, index, key, data, dwt_table[index].status);

    if (index != -1) //found
    {
        pt = &dwt_table[index];
        if (pt->status == DataExists)
        {
            strcpy(data, pt->data); // coution, data should have space
            pt->status = NoData;    // mark to already got the data
        }
        else
        {
            //printf("DWT %d: (%s) nodata\n", index, pt->key.str);
            index = -1;
        }
    }
    //DDEBUG
    //printf("<<%s index=%d key=%s data=%s status=%d\r\n",
    //	__func__, index, key, data, dwt_table[index].status);

    return index;
}

void PointList(void)
{
    int i;
    DWT_TABLE *pt = &dwt_table[0];

    for (i = 0; i < DWT_TABLE_SIZE; i++)
    {
        if (0LL != pt->key.ll)
        {
            printf("index %d: %s=%s (%d %016llx)\n",
                   i, pt->key.str, pt->data, pt->status, pt->key.ll);
        }
        else
            break;
        pt++;
    }
}

//
//
bool EoGetData(const char *pointName, char *data)
{
    int index;
    char dataBuffer[DWT_DATA_SIZE];  

	//printf("** >>%s <%s>\r\n", __func__, pointName);
    dataBuffer[0] = '\0';
    index = PointGet(pointName, data);
    if (index != -1 && dataBuffer[0] != '\0')
    {
        strcpy(dataBuffer, data);
        //printf("***** %s$$L:index=%d %s=%s\r\n", // DD Debug
        //    __func__, index, pointName, dataBuffer);
    }

    return(index != -1);
}
//
//
static char *PidPath;

void EoSignalAction(int signo, void (*func)(int))
{
    struct sigaction act, oact;

    if (signo == SIGENOCEAN)
    {
        act.sa_sigaction = (void (*)(int, siginfo_t *, void *))func;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO;
    }
    else
    {
        act.sa_handler = func;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_RESTART;
    }
    if (sigaction(signo, &act, &oact) < 0)
    {
        fprintf(stderr, "error at sigaction\n");
    }
}

void ExamineEvent(int Signum, siginfo_t *ps, void *pt)
{
    EO_DATA *pe;
    int index;
    int i;
    char message[6] = {">>>0\n"};

    (void) pt;
    (void) Signum;
    index = (unsigned long)ps->si_value.sival_int;
    message[3] = '0' + index;
    write(1, message, 6);

    /////
    i = 0;
    while ((pe = EoGetDataByIndex(index)) != NULL)
    {
        DDEBUG
        printf("$$E:%d: %08X %d:%s='%s'\n",
               i, pe->Id, pe->PIndex, pe->Name, pe->Data);

        //lock ?
        PointPut(pe->Name, pe->Data);
        i++;
    }
}

static void stopHandler(int sign)
{
    (void) sign;
    if (PidPath)
    {
        unlink(PidPath);
    }
    printf("Got Signal");
    exit(0);
}

//
//
//
int main(int argc, char **argv)
{
    GATEWAY_HANDLE deviceCloudUploadGateway;
    int opt;
    pid_t myPid = getpid();
    FILE *f;

    printf(version);
    printf("pid=%d\n", myPid);

    while ((opt = getopt(argc, argv, "D")) != EOF)
    {
        switch (opt)
        {
        case 'D':
            DFlag++;
            break;

        default: /* '?' */
            fprintf(stderr, "Usage: %s [-D][-d domain-name] [-p] port#\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if ((argc - optind) != 1)
    {
        printf("usage: simulated_device_cloud_upload [options...] gatewayConfigFile\n");
        printf("gatewayConfigFile is a JSON configuration file \n\n");
        printf("    options:\n");
        printf("             -D         add debug option.\n\n");
    }
    else
    {
        if (DFlag)
        {
            printf("**main()\n"); //// DEBUG
        }
        
        PidPath = EoMakePath(EO_DIRECTORY, AZURE_PID_FILE);
        f = fopen(PidPath, "w");
        if (f == NULL)
        {
            fprintf(stderr, ": cannot create pid file=%s\n",
                    PidPath);
            return 1;
        }
        fprintf(f, "%d\n", myPid);
        fclose(f);
        printf("PID=%d file=%s\n", myPid, PidPath);

        signal(SIGINT, stopHandler); /* catches ctrl-c */
        signal(SIGTERM, stopHandler); /* catches kill -15 */
        EoSignalAction(SIGENOCEAN, (void(*)(int)) ExamineEvent);

        EoReflesh(); // initial load table
        if (DFlag)
        {
            printf("**EoReflesh() end.\n"); ////DEBUG
	}	
        if (platform_init() == 0)
        {
            if (configureAsAService() != 0)
            {
		        printf("**Could not configure gateway as a service.\r\n");
                LogError("Could not configure gateway as a service.");
            }
            else
            {
				//printf("**Gateway_CreateFromJson() start.\n"); ////DEBUG
                deviceCloudUploadGateway = Gateway_CreateFromJson(argv[optind]);
                //printf("**Gateway_CreateFromJson() end.\n"); ////DEBUG
                if (deviceCloudUploadGateway == NULL)
                {
		    		printf("**Failed to create gateway\r\n");
                    LogError("Failed to create gateway");
                }
                else
                {
                    /* Wait for user input to quit. */
				    //printf("**waitForUserInput() start\r\n");
                    waitForUserInput();
				    //printf("**waitForUserInput() end\r\n");
                    Gateway_Destroy(deviceCloudUploadGateway);
		    		//printf("**Gateway_Destroy() end\r\n");
                }
            }
            platform_deinit();
            //printf("**platform_deinit() end.\n"); ////DEBUG
        }
        else
        {
            LogError("Failed to initialize the platform.");
        }
    }
    return 0;
}
