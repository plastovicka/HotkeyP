/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#include "hdr.h"
#pragma hdrstop
#include "DropTarget.h"

//drag & drop does not work if HotkeyP is running as admin

CDropTarget::CDropTarget()
{
	refCount = 1;
	command = name = NULL;
}

HRESULT __stdcall CDropTarget::QueryInterface(REFIID iid, void** ppvObject)
{
	if (iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

ULONG __stdcall CDropTarget::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}

ULONG __stdcall CDropTarget::Release(void)
{
	LONG count = InterlockedDecrement(&refCount);
	if (count == 0){
		FreeNames();
		delete this;
	}
	return count;
}

static FORMATETC fmte = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

HRESULT __stdcall CDropTarget::DragEnter(IDataObject* pDataObject, DWORD /*grfKeyState*/, POINTL /*pt*/, DWORD* pdwEffect)
{
	if (fmte.cfFormat == 0) fmte.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	if (SUCCEEDED(pDataObject->QueryGetData(&fmte))) {
		*pdwEffect &= DROPEFFECT_LINK;
		allow = true;
	}
	else {
		*pdwEffect = DROPEFFECT_NONE;
		allow = false;
	}
	return S_OK;
}

HRESULT __stdcall CDropTarget::DragOver(DWORD /*grfKeyState*/, POINTL /*pt*/, DWORD* pdwEffect)
{
	if (!allow) *pdwEffect = DROPEFFECT_NONE;
	return S_OK;
}

HRESULT __stdcall CDropTarget::DragLeave()
{
	return S_OK;
}

static WCHAR* GetIDLname(IShellFolder* psf, LPCITEMIDLIST pidl, SHGDNF uFlags)
{
	STRRET strRet;
	if (SUCCEEDED(psf->GetDisplayNameOf(pidl, uFlags, &strRet))) {
		if (strRet.uType == STRRET_WSTR) {
			return strRet.pOleStr;
		}
	}
	return 0;
}

HRESULT __stdcall CDropTarget::Drop(IDataObject* pDataObject, DWORD /*grfKeyState*/, POINTL /*pt*/, DWORD* /*pdwEffect*/)
{
	STGMEDIUM medium;
	if (SUCCEEDED(pDataObject->GetData(&fmte, &medium))) {
		CIDA* cida = (CIDA*)GlobalLock(medium.hGlobal);
		if (cida) {
			if (cida->cidl == 1) {
				IShellFolder* desktop;
				if (SUCCEEDED(SHGetDesktopFolder(&desktop))) {
					LPITEMIDLIST idl = ILCombine((LPITEMIDLIST)(((BYTE*)cida) + cida->aoffset[0]), (LPITEMIDLIST)(((BYTE*)cida) + cida->aoffset[1]));
					LPCITEMIDLIST idl2;
					if (SUCCEEDED(SHBindToParent(idl, IID_IShellFolder, (void**)&desktop, &idl2))) {
						FreeNames();
						command = GetIDLname(desktop, idl2, SHGDN_FORPARSING);
						name = GetIDLname(desktop, idl2, SHGDN_FORADDRESSBAR);
						if(command || name) PostMessage(hWnd, WM_USER + 1427, 0, (LPARAM)this);
					}
					CoTaskMemFree(idl);
					desktop->Release();
				}
			}
			GlobalUnlock(medium.hGlobal);
		}
		ReleaseStgMedium(&medium);
	}
	return S_OK;
}

void CDropTarget::FreeNames() 
{
	CoTaskMemFree(command); 
	CoTaskMemFree(name);
	command = name = NULL;
}

CDropTarget* CDropTarget::RegisterDropWindow(HWND hwnd)
{
	if (SUCCEEDED(OleInitialize(0))) {
		CDropTarget* pDropTarget = new CDropTarget();
		pDropTarget->hWnd = hwnd;
		if (SUCCEEDED(RegisterDragDrop(hwnd, pDropTarget))) return pDropTarget;
		pDropTarget->Release();
	}
	return NULL;
}

void CDropTarget::UnregisterDropWindow()
{
	RevokeDragDrop(hWnd);
	Release();
	OleUninitialize();
}
