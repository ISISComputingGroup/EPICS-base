#include <windows.h>
#include <dbghelp.h>
#define epicsExportSharedSymbols
#include "epicsStackTracePvt.h"
#include "epicsStackTrace.h"

#define MAX_SYM_SIZE 255

int epicsFindAddr(void *addr, epicsSymbol *sym_p)
{
	static int first_call = 1;
	static HANDLE process = NULL;
	DWORD64 displacement64 = 0;
	DWORD displacement = 0;
	SYMBOL_INFO *symbol;
	IMAGEHLP_LINE64 line;
	if (first_call)
	{
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);
		process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);
		first_call = 0;
	}
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + (MAX_SYM_SIZE + 1) * sizeof(char), 1);
	symbol->MaxNameLen = MAX_SYM_SIZE;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	if (!SymFromAddr(process, (DWORD64)addr, &displacement64, symbol))
	{
		sym_p->s_nam = 0;
		sym_p->s_val = 0;
		sym_p->f_nam = 0;
		free(symbol);
		return -1;
	}
	sym_p->s_nam = strdup(symbol->Name);
	sym_p->s_val = (void*)symbol->Address;
	if (SymGetLineFromAddr64(process, (DWORD64)addr, &displacement, &line))
	{
		sym_p->f_nam = strdup(line.FileName);
	}
	else
	{
		sym_p->f_nam = 0;
	}
	free(symbol);
	return 0;
}

int epicsFindAddrGetFeatures(void)
{
	return  EPICS_STACKTRACE_LCL_SYMBOLS
		| EPICS_STACKTRACE_GBL_SYMBOLS
		| EPICS_STACKTRACE_DYN_SYMBOLS;
}
