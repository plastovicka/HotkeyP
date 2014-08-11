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
that uses std::string instead of plain char *.
It was written mainly so std::getenv could be replaced with other
functions, because on Microsoft Windows, Microsoft's version of this
function is not secure for some reason.  Microsoft recommends using
their _dupenv_s function but that function is only available when
linking with the Microsoft C run-time libraries.
*/
// original version using the portable std::getenv function
static const std::string getEnv(const std::string pVar)
{
	if(pVar=="HotkeyP"){
		char fn[256];
		getExeDir(fn, "");
		return fn;
	}
	const char * val = getenv(pVar.c_str());
	return val ? val : "";
}

// Microsoft specific version that uses Microsoft's _dupenv_s function
//static const std::string getEnv( const std::string pVar )
//{
//    std::string results;
//    char * pValue = 0;
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
std::string ExpandVars(std::string s)
{
	std::string::size_type idxEnd = s.find('%');
	if(idxEnd == std::string::npos) return s;

	std::string::size_type idxBeg = 0;
	std::string ret;
	do
	{
		ret += s.substr(idxBeg, idxEnd - idxBeg);
		idxBeg = idxEnd + 1;
		// find second %
		idxEnd = s.find('%', idxBeg);
		if(idxEnd == std::string::npos) break;
		// extract var
		std::string var = s.substr(idxBeg, idxEnd - idxBeg);
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
