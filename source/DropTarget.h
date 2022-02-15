/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef DropTargetH
#define DropTargetH

class CDropTarget : public IDropTarget
{
public:
	static CDropTarget* RegisterDropWindow(HWND hwnd);
	void UnregisterDropWindow();

	// IUnknown implementation
	HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
	ULONG	__stdcall AddRef(void);
	ULONG	__stdcall Release(void);

	// IDropTarget implementation
	HRESULT __stdcall DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	HRESULT __stdcall DragLeave(void);
	HRESULT __stdcall Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

	WCHAR *command, *name;
	void FreeNames();
private:
	CDropTarget();
	LONG refCount;
	HWND hWnd;
	bool allow;
};

#endif
