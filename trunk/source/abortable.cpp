/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License.
*/
/*
This module contains code relating to operations that can be aborted by
the user.
*/
#include "hdr.h"
#pragma hdrstop
#include "hotkeyp.h"    // for HotKey class def, executeHotKey(), and hotKeyA array
#include "resource.h"   // for resource generated identifiers

/*
pCancelCmdHotKey - pointer to HotKey object that will be executed if
user does not cancel the command.  Must be set before dialog is opened.
*/
static HotKey const * pCancelCmdHotKey = 0; // input into dialog

/*
---------------------------------------------------------------------------
Cancel Command Dialog Box Callback
This code handles the "Cancel Command" dialog box that gives the user
a chance to cancel a command before it is executed.
It operates by alerting the user of the command that will be executed
and gives the user a chance to cancel/abort the command.
Only the option to cancel is given.  The rational is as follows.
If the dialog has an [OK] and [Cancel] buttons, the user can get in the
habit of invoking the command with a HotKey and typing the keys to
automatically select [OK].  The purpose if this dialog is to prevent
the user from typing something "from habit" that could invoke a command
s/he might later regret executing.  So this dialog forces the delay
the user might need to think about whether this was the correct command
to execute or not, and if it isn't the user has a few seconds to correct
the mistake.
*/
static INT_PTR CALLBACK cntDownCallback(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM /*lParam*/)
{
	const int DurationInSeconds = 5;    // time until command executed
	const int CountPerSeconds = 2;      // rate at which progress bar is updated
	const int MaxCount = DurationInSeconds * CountPerSeconds;
	const int StepSize = -(100 / MaxCount);
	const int StepDurationInMilliSec = 1000 / CountPerSeconds;

	static int count;
	static HWND hProgressBar;

	switch(msg) {
		case WM_INITDIALOG:
			// - translate dialog to selected language
			setDlgTexts(hWndDlg, 22);
			// - fill in info related to command to execute
			if(pCancelCmdHotKey) {
				SetDlgItemText(hWndDlg, IDC_STATIC_CANCEL_CMDNOTE, pCancelCmdHotKey->note);
				SetDlgItemText(hWndDlg, IDC_STATIC_CANCEL_CMDEXE, pCancelCmdHotKey->getFullCmd().c_str());
				SetDlgItemText(hWndDlg, IDC_STATIC_CANCEL_CMDARGS, pCancelCmdHotKey->args);
			}
			// - initialize countdown progress bar
			hProgressBar = GetDlgItem(hWndDlg, IDC_PROGRESS1);
			SendMessage(hProgressBar, PBM_SETPOS, 100, 0);
			SendMessage(hProgressBar, PBM_SETSTEP, (WPARAM)StepSize, 0);
			SetTimer(hWndDlg, 1, StepDurationInMilliSec, NULL);
			count = MaxCount;
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDCANCEL:
					EndDialog(hWndDlg, IDCANCEL);
					return TRUE;
			}
			break;

		case WM_TIMER:
			SendMessage(hProgressBar, PBM_STEPIT, 0, 0);
			if(--count <= 0) EndDialog(hWndDlg, IDOK);
			return TRUE;
	}

	return FALSE; // defer action to system defaults
}

/*
delayDialogActive - for internal use by delayAndExecuteHotKey.  It is
defined in global scope so that it will be initialized on process
initialization before the delayAndExecuteHotKey function can be invoked
thus preventing possible race conditions.
*/
static LONG delayDialogActive = false;

/*
---------------------------------------------------------------------------
delayAndExecuteHotKey - checks if HotKey should prompt user before
executing, prompts user if necessary and then executes the HotKey.
*/
void delayAndExecuteHotKey(HINSTANCE instance, HWND parent, int hotKeyIndex)
{
	HotKey const * hk = &hotKeyA[hotKeyIndex];
	if((hk->ask || hk->delay) && hk->isActive())
	{
		if(hk->ask)
		{
			int j = msg1(MB_ICONQUESTION | MB_YESNO, _T("%s\r\n\n%s\r\n%s %s"),
				lng(764, "Do you want to execute this command ?"),
				hk->note,
				(hk->cmd >= 0) ? getCmdName(hk->cmd) : hk->exe,
				hk->args);
			for(int cnt = 0; cnt<40 && !GetForegroundWindow(); cnt++){
				Sleep(20);
			}
			if(j != IDYES) return;
		}
		else
		{
			// only "ask" user to execute the command if
			// 1) The HotKey object should prompt the user before executing AND
			// 2) if they HotKey is associated with a specific window, that
			//    window must be active/alive.
			// Note if a HotKey is *not* associated with a specific window,
			// it is always considered active.
			// There should only be one ask/delay dialog active at any given time.
			// Due to how keys strokes are processed by this program, it is possible
			// to get to this location multiple times; even though the dialog itself
			// is modal.  Therefore, before invoking the dialog box, ensure there
			// is only one thread of execution through this path.  Other attempts
			// to open this dialog will ignore the request to invoke the HotKey.

			// attempt to set flag to "true" but only if currently false
			LONG alreadyActive = InterlockedCompareExchange(&delayDialogActive, true, false);
			if(alreadyActive) return;

			// Note: don't attempt to set the external pCancelCmdHotKey pointer
			// until we've ensured that this function can exclusively open the
			// dialog box.
			// Provide pointer to the hotkey, that will be executed, to the dialog
			// box and invoke the dialog box.  Note: this is modal and will not
			// return until the dialog box closes.
			pCancelCmdHotKey = hk;
			INT_PTR results = DialogBox(instance, _T("CNTDOWN"), parent, cntDownCallback);
			// No protection needed here since only one thread can get in.
			delayDialogActive = false;
			if(results != IDOK) return;
		}
		if(hk != &hotKeyA[hotKeyIndex]) return;
	}

	executeHotKey(hotKeyIndex);
}
