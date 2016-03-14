/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License.
*/
#include "hdr.h"
#pragma hdrstop
#include <cstdlib>  // for getenv (or _dupenv_s)
#include "hotkeyp.h"

/*
getEnv is intended to be a simple wrapper around the std::getenv function
that uses std::string instead of plain TCHAR *.
It was written mainly so std::getenv could be replaced with other
functions, because on Microsoft Windows, Microsoft's version of this
function is not secure for some reason.  Microsoft recommends using
their _dupenv_s function but that function is only available when
linking with the Microsoft C run-time libraries.
*/
// original version using the portable std::getenv function
static const tstring getEnv(const tstring pVar)
{
	if(pVar==_T("HotkeyP")){
		TCHAR fn[256];
		getExeDir(fn, _T(""));
		return fn;
	}
	const TCHAR * val = _tgetenv(pVar.c_str());
	return val ? val : _T("");
}

// Microsoft specific version that uses Microsoft's _dupenv_s function
//static const std::string getEnv( const std::string pVar )
//{
//    std::string results;
//    TCHAR * pValue = 0;
//    std::size_t len = 0;
//    errno_t err = _dupenv_s( & pValue, & len, pVar.c_str() );
//    if ( !err ) results.insert( 0, pValue, len );
//    std::free( pValue );
//    return results;
//}


/*
===========================================================================
Expand variables enclosed in percent-signs with values from environment.

For example:
If "SystemRoot" is an variable defined in the environment with a value of
"C:\Windows" and the input string is "%SystemRoot%\write.exe" the output
of this function would be "C:\Windows\write.exe".

If the variable name between the percent signs is not defined in the
environment, it is replaced with an empty string.
Two consecutive percent signs (i.e. "%%") are replaced with a single
percent sign.
*/
tstring ExpandVars(tstring s)
{
	std::string::size_type idxEnd = s.find('%');
	if(idxEnd == std::string::npos) return s;

	std::string::size_type idxBeg = 0;
	tstring ret;
	do
	{
		ret += s.substr(idxBeg, idxEnd - idxBeg);
		idxBeg = idxEnd + 1;
		// find second %
		idxEnd = s.find('%', idxBeg);
		if(idxEnd == std::string::npos) break;
		// extract var
		tstring var = s.substr(idxBeg, idxEnd - idxBeg);
		idxBeg = idxEnd + 1;
		if(var.empty())
			ret += '%';
		else
		{
			ret += getEnv(var);
		}
		// find first %
		idxEnd = s.find('%', idxBeg);
	} while(idxEnd != std::string::npos);

	return ret + s.substr(idxBeg);
}

//-------------------------------------------------------------------------

static DWORD processIntegrityLevel=0xffff;

DWORD GetProcessIntegrityLevel()
{
	if(processIntegrityLevel==0xffff)
	{
		processIntegrityLevel=0;
		HANDLE hToken;
		if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			DWORD tokenSize = 0;
			GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &tokenSize);
			if(tokenSize>0)
			{
				void *token = operator new(tokenSize);
				if(GetTokenInformation(hToken, TokenIntegrityLevel, token, tokenSize, &tokenSize))
				{
					processIntegrityLevel = *GetSidSubAuthority(*(void**)token, 0);
				}
				operator delete(token);
			}
			CloseHandle(hToken);
		}
	}
	return processIntegrityLevel;
}

bool isElevated()
{
	return GetProcessIntegrityLevel() > SECURITY_MANDATORY_MEDIUM_RID;
}

BOOL CreateMediumIntegrityProcess(LPWSTR exe, DWORD creationFlags, LPCWSTR dir, STARTUPINFOW *si, PROCESS_INFORMATION *pi)
{
	static TCreateProcessWithTokenW pCreateProcessWithTokenW;
	if(!pCreateProcessWithTokenW){
		pCreateProcessWithTokenW = (TCreateProcessWithTokenW)GetProcAddress(GetModuleHandleA("Advapi32"), "CreateProcessWithTokenW");
		if(!pCreateProcessWithTokenW) return FALSE;
	}
	BOOL result = FALSE;
	DWORD pid = 0;
	GetWindowThreadProcessId(GetShellWindow(), &pid);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if(hProcess){
		HANDLE hToken, hToken2;
		if(OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hToken)){
			if(DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, 0, SecurityImpersonation, TokenPrimary, &hToken2)){
				result = pCreateProcessWithTokenW(hToken2, 0, NULL, exe, creationFlags, NULL, dir, si, pi);
				CloseHandle(hToken2);
			}
			CloseHandle(hToken);
		}
		CloseHandle(hProcess);
	}
	return result;
}

#ifndef UNICODE
BOOL CreateMediumIntegrityProcess(LPSTR exe, DWORD creationFlags, LPCSTR dir, STARTUPINFO *si, PROCESS_INFORMATION *pi)
{
	convertT2W(exe, exew);
	convertT2W(dir, dirw);
	return CreateMediumIntegrityProcess(exew, creationFlags, dirw, reinterpret_cast<STARTUPINFOW*>(si), pi);
}
#endif
