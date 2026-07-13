/* ZoiteChat
 * Copyright (c) 2011-2012 Berke Viktor.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "zoitechat-plugin.h"

#define EXEC_COMMAND_MAX 8192

static zoitechat_plugin *ph;
static char name[] = "Exec";
static char desc[] = "Execute commands inside Fabulor";
static char version[] = "1.2";

static int
run_command (char *word[], char *word_eol[], void *userdata)
{
	const char commandPrefix[] = "cmd.exe /c ";
	const char *command;
	char *commandLine = NULL;
	char buffer[4096];
	DWORD dwRead = 0;
	DWORD dwLeft = 0;
	DWORD dwAvail = 0;
	DWORD error;
	time_t start;
	double timeElapsed;
	size_t commandLen;
	size_t commandLineLen;

	char *token;
	char *context = NULL;
	int announce = 0;

	HANDLE readPipe = NULL;
	HANDLE writePipe = NULL;
	STARTUPINFO sInfo; 
	PROCESS_INFORMATION pInfo; 
	SECURITY_ATTRIBUTES secattr; 

	ZeroMemory (&secattr, sizeof (secattr));
	secattr.nLength = sizeof (secattr);
	secattr.bInheritHandle = TRUE;

	timeElapsed = 0.0;

	if (strlen (word[2]) > 0)
	{
		if (!stricmp("-O", word[2]))
		{
			command = word_eol[3];
			announce = 1;
		}
		else
		{
			command = word_eol[2];
		}

		if (!command || !*command)
		{
			zoitechat_command (ph, "help exec");
			return ZOITECHAT_EAT_ZOITECHAT;
		}

		commandLen = strlen (command);
		if (commandLen > EXEC_COMMAND_MAX)
		{
			zoitechat_printf (ph, "Command is too long. Maximum length is %u characters.\n", EXEC_COMMAND_MAX);
			return ZOITECHAT_EAT_ZOITECHAT;
		}

		commandLineLen = sizeof (commandPrefix) + commandLen;
		commandLine = malloc (commandLineLen);
		if (!commandLine)
		{
			zoitechat_printf (ph, "Unable to allocate command buffer.\n");
			return ZOITECHAT_EAT_ZOITECHAT;
		}

		if (strcpy_s (commandLine, commandLineLen, commandPrefix) != 0 ||
			strcat_s (commandLine, commandLineLen, command) != 0)
		{
			zoitechat_printf (ph, "Unable to prepare command line.\n");
			free (commandLine);
			return ZOITECHAT_EAT_ZOITECHAT;
		}

		if (!CreatePipe (&readPipe, &writePipe, &secattr, 0))
		{
			error = GetLastError ();
			zoitechat_printf (ph, "Unable to create command output pipe: %lu\n", error);
			free (commandLine);
			return ZOITECHAT_EAT_ZOITECHAT;
		}

		SetHandleInformation (readPipe, HANDLE_FLAG_INHERIT, 0);

		ZeroMemory (&sInfo, sizeof (sInfo));
		ZeroMemory (&pInfo, sizeof (pInfo));
		sInfo.cb = sizeof (sInfo);
		sInfo.dwFlags = STARTF_USESTDHANDLES;
		sInfo.hStdInput = NULL;
		sInfo.hStdOutput = writePipe;
		sInfo.hStdError = writePipe;

		if (!CreateProcess (0, commandLine, 0, 0, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 0, 0, &sInfo, &pInfo))
		{
			error = GetLastError ();
			zoitechat_printf (ph, "Unable to execute command: %lu\n", error);
			CloseHandle (writePipe);
			CloseHandle (readPipe);
			free (commandLine);
			return ZOITECHAT_EAT_ZOITECHAT;
		}

		free (commandLine);
		CloseHandle (writePipe);

		start = time (0);
		while (PeekNamedPipe (readPipe, buffer, 1, &dwRead, &dwAvail, &dwLeft) && timeElapsed < 10)
		{
			if (dwRead)
			{
				if (ReadFile (readPipe, buffer, sizeof (buffer) - 1, &dwRead, NULL) && dwRead != 0 )
				{
					buffer[dwRead] = '\0';

					if (announce)
					{
						token = strtok_s (buffer, "\n", &context);
						while (token != NULL)
						{
							zoitechat_commandf (ph, "SAY %s", token);
							token = strtok_s (NULL, "\n", &context);
						}
					}
					else
						zoitechat_printf (ph, "%s", buffer);
				}
			}
			else
			{
				SleepEx (100, TRUE);
			}
			timeElapsed = difftime (time (0), start);
		}

		if (!announce)
			zoitechat_printf (ph, "\n");

		if (timeElapsed >= 10)
		{
			zoitechat_printf (ph, "Command took too much time to run, execution aborted.\n");
		}

		CloseHandle (readPipe);
		CloseHandle (pInfo.hProcess);
		CloseHandle (pInfo.hThread);
	}
	else
	{
		zoitechat_command (ph, "help exec");
	}

	return ZOITECHAT_EAT_ZOITECHAT;
}

int
zoitechat_plugin_init (zoitechat_plugin *plugin_handle, char **plugin_name, char **plugin_desc, char **plugin_version, char *arg)
{
	ph = plugin_handle;

	*plugin_name = name;
	*plugin_desc = desc;
	*plugin_version = version;

	zoitechat_hook_command (ph, "EXEC", ZOITECHAT_PRI_NORM, run_command, "Usage: /EXEC [-O] - execute commands inside Fabulor", 0);
	zoitechat_printf (ph, "%s plugin loaded\n", name);

	return 1;
}

int
zoitechat_plugin_deinit (void)
{
	zoitechat_printf (ph, "%s plugin unloaded\n", name);
	return 1;
}
