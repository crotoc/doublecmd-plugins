#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <virtual.h>
#include <string.h>
#include <time.h>
#include "wfxplugin.h"
#include "extension.h"

#define Int32x32To64(a,b) ((int64_t)(a)*(int64_t)(b))

typedef struct sAVFSDirData
{
	DIR *cur;
	char path[PATH_MAX];
} tAVFSDirData;

int gPluginNr;
tProgressProc gProgressProc;
tLogProc gLogProc;
tRequestProc gRequestProc;
tExtensionStartupInfo* gStartupInfo;

static char gAVFSPath[PATH_MAX] = "/#avfsstat";
static char gLFMPath[PATH_MAX];
static char gHistoryFile[PATH_MAX];
int gListItems = 0;

char* strlcpy(char* p, const char* p2, int maxlen)
{
	if ((int)strlen(p2) >= maxlen)
	{
		strncpy(p, p2, maxlen);
		p[maxlen] = 0;
	}
	else
		strcpy(p, p2);

	return p;
}

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	int64_t ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

bool SetFindData(DIR *cur, char *path, WIN32_FIND_DATAA *FindData)
{
	struct dirent *ent;
	struct stat buf;
	char lpath[PATH_MAX];

	ent = virt_readdir(cur);

	if (ent != NULL)
	{
		memset(FindData, 0, sizeof(WIN32_FIND_DATAA));

		snprintf(lpath, sizeof(lpath), "%s/%s", path, ent->d_name);

		if (virt_stat(lpath, &buf) == 0)
		{
			if (S_ISDIR(buf.st_mode))
				FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

			FindData->nFileSizeHigh = (buf.st_size & 0xFFFFFFFF00000000) >> 32;
			FindData->nFileSizeLow = buf.st_size & 0x00000000FFFFFFFF;

			UnixTimeToFileTime(buf.st_mtime, &FindData->ftLastWriteTime);

			FindData->dwFileAttributes |= FILE_ATTRIBUTE_UNIX_MODE;
			FindData->dwReserved0 = buf.st_mode;
		}

		strlcpy(FindData->cFileName, ent->d_name, PATH_MAX - 1);

		return true;
	}

	return false;
}

intptr_t DCPCALL DlgProc(uintptr_t pDlg, char* DlgItemName, intptr_t Msg, intptr_t wParam, intptr_t lParam)
{
	FILE *fp;
	size_t len = 0;
	ssize_t read = 0;
	int i = 0;
	bool localfile = false;
	char *path = NULL, *file = NULL, *line = NULL;

	switch (Msg)
	{
	case DN_INITDIALOG:
		gStartupInfo->SendDlgMsg(pDlg, "fneLocalFile", DM_ENABLE, (intptr_t)localfile, 0);
		gStartupInfo->SendDlgMsg(pDlg, "cmbPath", DM_ENABLE, (intptr_t)!localfile, 0);

		gListItems = 0;

		if ((fp = fopen(gHistoryFile, "r")) != NULL)
		{
			while (((read = getline(&line, &len, fp)) != -1) && gListItems < 10)
			{
				if (line[read - 1] == '\n')
					line[read - 1] = '\0';

				strlcpy(line, strdup(line), PATH_MAX);
				gStartupInfo->SendDlgMsg(pDlg, "cmbPath", DM_LISTADD, (intptr_t)line, 0);
				gListItems++;
			}

			fclose(fp);
		}

		if (gListItems == 0)
		{
			gStartupInfo->SendDlgMsg(pDlg, "cmbPath", DM_LISTADD, (intptr_t)gAVFSPath, 0);
			gListItems++;
		}

		gStartupInfo->SendDlgMsg(pDlg, "cmbPath", DM_LISTSETITEMINDEX, 0, 0);

		break;

	case DN_CLICK:
		if (strcmp(DlgItemName, "btnOK") == 0)
		{
			path = strdup((char*)gStartupInfo->SendDlgMsg(pDlg, "cmbPath", DM_GETTEXT, 0, 0));
			file = strdup((char*)gStartupInfo->SendDlgMsg(pDlg, "fneLocalFile", DM_GETTEXT, 0, 0));
			localfile = (bool*)gStartupInfo->SendDlgMsg(pDlg, "chkLocalFile", DM_GETCHECK, 0, 0);

			if (localfile && file != NULL && file[0] != '\0')
			{
				if (strrchr(file, '#') == NULL)
					snprintf(gAVFSPath, sizeof(gAVFSPath), "%s#", file);
				else
					strlcpy(gAVFSPath, file, sizeof(gAVFSPath));
			}
			else if (path != NULL && path[0] != '\0')
				strlcpy(gAVFSPath, path, sizeof(gAVFSPath));
			else
				strlcpy(gAVFSPath, "/#avfsstat", sizeof(gAVFSPath));

			if ((fp = fopen(gHistoryFile, "w")) != NULL)
			{
				fprintf(fp, "%s\n", gAVFSPath);

				for (i = 0; i < gListItems; i++)
				{
					line = strdup((char*)gStartupInfo->SendDlgMsg(pDlg, "cmbPath", DM_LISTGETITEM, i, 0));

					if (line != NULL && (strcmp(gAVFSPath, line) != 0))
						fprintf(fp, "%s\n", line);
				}

				fclose(fp);
			}

			gStartupInfo->SendDlgMsg(pDlg, DlgItemName, DM_CLOSE, 3, 0);
		}

		break;

	case DN_CHANGE:
		if (strcmp(DlgItemName, "chkLocalFile") == 0)
		{
			localfile = (bool*)gStartupInfo->SendDlgMsg(pDlg, "chkLocalFile", DM_GETCHECK, 0, 0);
			gStartupInfo->SendDlgMsg(pDlg, "fneLocalFile", DM_ENABLE, (intptr_t)localfile, 0);
			gStartupInfo->SendDlgMsg(pDlg, "cmbPath", DM_ENABLE, (intptr_t)!localfile, 0);
		}

		break;
	}

	if (file)
		free(file);

	if (path)
		free(path);

	if (line)
		free(line);

	return 0;
}

static void ShowAVFSPathDlg(void)
{
	if (access(gLFMPath, F_OK) != 0)
		gRequestProc(gPluginNr, RT_TargetDir, "AVFS", "Enter AVFS path:", gAVFSPath, sizeof(gAVFSPath) - 1);
	else
		gStartupInfo->DialogBoxLFMFile(gLFMPath, DlgProc);
}

int DCPCALL FsInit(int PluginNr, tProgressProc pProgressProc, tLogProc pLogProc, tRequestProc pRequestProc)
{
	gPluginNr = PluginNr;
	gProgressProc = pProgressProc;
	gLogProc = pLogProc;
	gRequestProc = pRequestProc;
	return 0;
}

HANDLE DCPCALL FsFindFirst(char* Path, WIN32_FIND_DATAA *FindData)
{
	tAVFSDirData *dirdata;
	dirdata = malloc(sizeof(tAVFSDirData));
	memset(dirdata, 0, sizeof(tAVFSDirData));

	snprintf(dirdata->path, sizeof(dirdata->path), "%s%s", gAVFSPath, Path);
	dirdata->cur = virt_opendir(dirdata->path);

	if (dirdata->cur != NULL && SetFindData(dirdata->cur, dirdata->path, FindData) == true)
		return (HANDLE)dirdata;

	return (HANDLE)(-1);
}

BOOL DCPCALL FsFindNext(HANDLE Hdl, WIN32_FIND_DATAA *FindData)
{
	tAVFSDirData *dirdata = (tAVFSDirData*)Hdl;

	return SetFindData(dirdata->cur, dirdata->path, FindData);
}

int DCPCALL FsFindClose(HANDLE Hdl)
{
	tAVFSDirData *dirdata = (tAVFSDirData*)Hdl;

	if (dirdata->cur != NULL)
		virt_closedir(dirdata->cur);

	return 0;
}

int DCPCALL FsGetFile(char* RemoteName, char* LocalName, int CopyFlags, RemoteInfoStruct* ri)
{
	int ifd, ofd, done;
	ssize_t len, total = 0;
	char rpath[PATH_MAX];
	char buff[8192];
	int result = FS_FILE_OK;

	if ((CopyFlags == 0) && (access(LocalName, F_OK) == 0))
		return FS_FILE_EXISTS;

	snprintf(rpath, sizeof(rpath), "%s%s", gAVFSPath, RemoteName);

	if (gProgressProc(gPluginNr, rpath, LocalName, 0) == 1)
		return FS_FILE_USERABORT;

	ifd = virt_open(rpath, O_RDONLY, 0);

	if (ifd == -1)
		return FS_FILE_READERROR;

	ofd = open(LocalName, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

	if (ofd > -1)
	{
		size_t rsize = ((int64_t)ri->SizeHigh << 32) | ri->SizeLow;

		while ((len = virt_read(ifd, buff, sizeof(buff))) > 0)
		{
			if (write(ofd, buff, len) == -1)
				result = FS_FILE_WRITEERROR;

			total += len;

			if (rsize > 0)
				done = total * 100 / rsize;
			else
				done = 0;

			if (done > 100)
				done = 100;

			if (gProgressProc(gPluginNr, rpath, LocalName, done) == 1)
			{
				result = FS_FILE_USERABORT;
				break;
			}
		}

		close(ofd);
	}
	else
		result = FS_FILE_WRITEERROR;

	if (ri->Attr > 0)
		chmod(LocalName, ri->Attr);

	virt_close(ifd);

	return result;
}

int DCPCALL FsExecuteFile(HWND MainWin, char* RemoteName, char* Verb)
{
	if (strcmp(Verb, "open") == 0)
		return FS_EXEC_YOURSELF;

	return FS_EXEC_OK;
}

void DCPCALL FsStatusInfo(char* RemoteDir, int InfoStartEnd, int InfoOperation)
{
	if (strcmp(RemoteDir, "/") == 0)
	{
		if (InfoStartEnd == FS_STATUS_START && InfoOperation == FS_STATUS_OP_LIST)
			ShowAVFSPathDlg();
	}
}

void DCPCALL FsSetDefaultParams(FsDefaultParamStruct* dps)
{
	Dl_info dlinfo;
	const char* lfm_name = "dialog.lfm";

	strlcpy(gHistoryFile, dps->DefaultIniName, PATH_MAX);
	char *pos = strrchr(gHistoryFile, '/');

	if (pos)
		strcpy(pos + 1, "AVFSRecent.txt");

	memset(&dlinfo, 0, sizeof(dlinfo));

	if (dladdr(lfm_name, &dlinfo) != 0)
	{
		strlcpy(gLFMPath, dlinfo.dli_fname, PATH_MAX);
		pos = strrchr(gLFMPath, '/');

		if (pos)
			strcpy(pos + 1, lfm_name);
	}
}

void DCPCALL FsGetDefRootName(char* DefRootName, int maxlen)
{
	strlcpy(DefRootName, "AVFS", maxlen - 1);
}

void DCPCALL ExtensionInitialize(tExtensionStartupInfo* StartupInfo)
{
	gStartupInfo = malloc(sizeof(tExtensionStartupInfo));
	memcpy(gStartupInfo, StartupInfo, sizeof(tExtensionStartupInfo));
}

void DCPCALL ExtensionFinalize(void* Reserved)
{
	free(gStartupInfo);
}
