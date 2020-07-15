/*
 (C) 2006-2007  Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#include "hdr.h"
#pragma hdrstop
#include <wincrypt.h>

bool sha1(BYTE *out, int outLen, char *in, int inLen)
{
	bool ok=false;
	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;
	if(CryptAcquireContext(&hCryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)){
		if(CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash)){
			if(CryptHashData(hHash, (BYTE*)in, inLen, 0)){
				DWORD d = outLen;
				if(CryptGetHashParam(hHash, HP_HASHVAL, out, &d, 0)){
					ok=true;
					memset(out+d, 0, outLen-d);
				}
			}
			CryptDestroyHash(hHash);
		}
		CryptReleaseContext(hCryptProv, 0);
	}
	return ok;
}

extern "C" int encrypt(BYTE *out, int outLen, char *in, int inLen, int alg)
{
	if(alg<0) alg=1;
	memset(out, 0, outLen);
	if(inLen>0){
		int i, j, x, r;
		r=x=0;
		for(i=0; i<outLen>>2; i++){
			for(j=0; j<inLen; j++){
				x=x*r+in[j];
				r=r*367413989+174680251;
			}
			((DWORD*)out)[i]=x;
		}
		if(alg==1){
			if(!sha1(out, outLen, in, inLen)) alg=0;
		}
	}
	return alg;
}
