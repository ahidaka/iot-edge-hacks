#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <linux/limits.h> //PATH_MAX
#include <stdbool.h>

#include "typedefs.h"
#include "dpride.h"

#define SC_SIZE (16)
#define NODE_TABLE_SIZE (256)

//
// Control file node table
//
typedef struct _node_table
{
	UINT Id;
    char *Eep;
	char *Desc;
	INT SCCount;
	char **SCuts;
} NODE_TABLE;

NODE_TABLE NodeTable[NODE_TABLE_SIZE];

//
//inline bool IsTerminator(char c)
bool IsTerminator(char c)
{
	return (c == '\n' || c == '\r' || c == '\0' || c == '#');
}

CHAR *DeBlank(char *p)
{
	while(isblank(*p))
	{
		p++;
	}
	return p;
}

CHAR *CheckNext(char *p)
{
	if (IsTerminator(*p))
	{
		// Oops, terminated suddenly
		return NULL;
	}
	p = DeBlank(p);
	if (IsTerminator(*p))
	{
		// Oops, terminated suddenly again
		return NULL;
	}
	return p;
}

CHAR *GetItem(char *p, char **item)
{
	char buf[BUFSIZ];
	char *base;
	char *duped;
	char *pnext;
	int i;

	//printf("*** GetIem: p=%s\n", p); //DEBUG
	base = &buf[0];
	for(i = 0; i < BUFSIZ; i++)
	{
		if (*p == ',' || IsTerminator(*p))
			break;
		*base++ = *p++;
	}
	pnext = p + (*p == ','); // if ',', forward one char
	*base = '\0';
	duped = strdup(buf);
	if (duped == NULL)
	{
		Error("duped == NULL");
	}
	else
	{
		*item = duped;
	}
	return pnext;
}

INT DecodeLine(char *Line, uint *Id, char **Eep, char **Desc, char ***SCuts)
{
	char *p;
	char *item;
	int scCount = 0;
	char **scTable;
	int i;

	scTable = (char **) malloc(sizeof(char *) * SC_SIZE);
	if (scTable == NULL)
	{
		Error("cannot alloc scTable");
		return 0;
	}

	p = GetItem(DeBlank(Line), &item);
	if (p == NULL || IsTerminator(*p) || *p == ',')
	{
		if (item)
			free(item);
		return 0;
	}
	//printf("**0: <%s><%s>\n", item, p);  //DEBUG
	*Id = strtoul(item, NULL, 16);

	if ((p = CheckNext(p)) == NULL)
	{
		Error("cannot read EEP item");
		return 0;
	}
	p = GetItem(p, &item);
	if (p == NULL || IsTerminator(*p) || *p == ',')
	{
		if (item)
			free(item);
		return 0;
	}
	*Eep = item;

	if ((p = CheckNext(p)) == NULL)
	{
		Error("cannot read Desc item");
		if (item)
			free(item);
		return 0;
	}
	p = GetItem(p, &item);
	if (p == NULL || IsTerminator(*p) || *p == ',')
	{
		if (item)
			free(item);
		return 0;
	}
	*Desc = item;

	if ((p = CheckNext(p)) == NULL)
	{
		Error("cannot read SCut first item");
		if (item)
                        free(item);
		return 0;
	}

	for(i = 0; i < SC_SIZE; i++)
	{
		p = GetItem(p, &item);
		if (p == NULL) {
			if (item)
				free(item);
			break;
		}
		scTable[i] = item;

		if ((p = CheckNext(p)) == NULL)
		{
			//End of line
			break;
		}
	}
	*SCuts = (char **) scTable;
	scCount = i + 1;
	return scCount;
}

INT ReadCsv(char *Filename)
{
	char buf[BUFSIZ];
	FILE *fd;
	NODE_TABLE *nt;
	uint id;
	char *eep;
	char *desc;
	char **scs;
	int scCount;
	int lineCount = 0;

	nt = &NodeTable[0];

	fd = fopen(Filename, "r");
	if (fd == NULL)
	{
		Error("Open error");
		return 0;
	}
	while(TRUE)
	{
		char *rtn = fgets(buf, BUFSIZ, fd);
		if (rtn == NULL)
		{
			//printf("*fgets: EOF found\n");
			break;
		}
		scCount = DecodeLine(buf, &id, &eep, &desc, &scs);
		if (scCount > 0)
		{
			if (nt->SCuts)
			{
				//purge old shortcuts
				////free(nt->SCuts);
            }
			nt->Id = id;
			nt->Eep = eep;
			nt->Desc = desc;
			nt->SCuts = scs;
			nt->SCCount = scCount;
			nt++;
			lineCount++;
			if (lineCount >= NODE_TABLE_SIZE)
			{
				Error("Node Table overflow");
				break;
			}
		}
	}
	nt->Id = 0; //mark EOL
	fclose(fd);

	return lineCount;
}

#define EO_DATSIZ (8)
typedef struct _eodata
{
	int  Index;
	int  Id;
	char *Eep;
	char *Name;
	char *Desc;
	int  PIndex;
	int  PCount;
	char Data[EO_DATSIZ];
}
EO_DATA;

char *EoMakePath(char *Dir, char *File);
INT EoReflesh(void);
EO_DATA *EoGetDataByIndex(int Index);
void EoLog(char *id, char *eep, char *msg);

static EO_DATA EoData;
static int LastIndex;
static int LastPoint;
static  FILE *eologf;

char *EoMakePath(char *Dir, char *File)
{
	char path[PATH_MAX];
	char *pathOut;

    if (File[0] == '/') {
        /* Assume absolute path */
        return(strdup(File));
    }
	strcpy(path, Dir);
	if (path[strlen(path) - 1] != '/')
	{
		strcat(path, "/");
	}
	strcat(path, File);
	pathOut = strdup(path);
	if (!pathOut)
	{
		Error("strdup() error");
	}
	return pathOut;
}

INT EoReflesh(void)
{
	int count;
	char *fname = EoMakePath(EO_DIRECTORY, EO_CONTROL_FILE);
	count = ReadCsv(fname);
	////free(fname);
	return count;
}

char *EoGetPointByIndex(int Index)
{
	NODE_TABLE *nt = &NodeTable[Index];
	char *fileName;
	int i;

	if (Index != LastIndex || LastPoint > nt->SCCount)
	{
		// renew, from start
		LastPoint = 0;
		LastIndex = Index;
	}

	i = LastPoint;
	if (i == nt->SCCount)
	{
		LastPoint++;
		return NULL;
	}
	fileName = nt->SCuts[i];
	if (fileName == NULL || fileName[0] == '\0')
	{
		printf("**%s:%d fileName is NULL\n",
		       __func__, i);
		LastPoint = 0;
		return NULL;
	}
	LastPoint++;
	return fileName;
}

EO_DATA *EoGetDataByIndex(int Index)
{
	NODE_TABLE *nt = &NodeTable[Index];
	char *fileName;
	char *p;
    FILE *f;
	char buf[BUFSIZ];
    char *bridgePath;
	EO_DATA *pe = &EoData;
	int i;

	//printf("%s: %u:%u:id=%08X eep=%s cnt=%d\n", __FUNCTION__,
	//       Index, LastPoint, nt->Id, nt->Eep, /*nt->Desc,*/ nt->SCCount);

	if (Index != LastIndex || LastPoint > nt->SCCount)
	{
		// renew, from start
		LastPoint = 0;
		LastIndex = Index;
	}

	i = LastPoint;
	if (i == nt->SCCount)
	{
		LastPoint++;
		return NULL;
	}
	fileName = nt->SCuts[i];
	if (fileName == NULL || fileName[0] == '\0')
	{
		printf("%s:%d fileName is NULL\n",
		       __func__, i);
		LastPoint = 0;
		return NULL;
	}
	bridgePath = EoMakePath(EO_DIRECTORY, fileName);

	f = fopen(bridgePath, "r");
	if (f == NULL) {
		fprintf(stderr, "Cannot open=%s\n", bridgePath);
		LastPoint = 0;
		return NULL;
	}
	while(fgets(buf, BUFSIZ, f) != NULL)
	{
		if (isdigit(buf[0]))
			break;
	}
	fclose(f);

	//printf("EoGetDataByI:%s=<%s>\r\n",
	//	bridgePath, buf);

	// omit '\r' '\n' of line end
	p = strchr(buf, '\n');
	if (p != NULL)
		*p = '\0';
	p = strchr(buf, '\r');
	if (p != NULL)
		*p = '\0';
	//
	pe->Index = Index;
	pe->Id = nt->Id;
	pe->Eep = nt->Eep;
	pe->Name = fileName;
	pe->Desc = nt->Desc;
	pe->PIndex = i;
	pe->PCount = nt->SCCount;
	strncpy(pe->Data, buf, EO_DATSIZ);

	LastPoint++;

	return pe;
}

FILE *EoLogInit(char *logname)
{
	eologf = fopen(logname, "w");
	if (eologf == NULL)
	{
		fprintf(stderr, ": cannot open logfile=%s\n", logname);
		return NULL;
	}
	return eologf;
}

void EoLog(char *id, char *eep, char *msg)
{
	time_t      timep;
	struct tm   *time_inf;
	char idBuffer[12];
	char eepBuffer[12];
	char timeBuf[64];
	char buf[BUFSIZ / 2];

	if (id)
		strcpy(idBuffer, id);
	if (eep)
		strcpy(eepBuffer, eep);

	timep = time(NULL);
	time_inf = localtime(&timep);
	strftime(timeBuf, sizeof(timeBuf), "%F %X", time_inf);
	sprintf(buf, "%s,%s,%s,%s\r\n", timeBuf, idBuffer, eepBuffer, msg);

	fwrite(buf, strlen(buf), 1, eologf);
	fflush(eologf);
	// debug view
    printf("%s,%s,%s,%s\n", timeBuf, idBuffer, eepBuffer, msg);
}
