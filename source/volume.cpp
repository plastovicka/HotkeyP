/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */

#include "hdr.h"
#pragma hdrstop
#include "VistaVol.h"
#include "hotkeyp.h"

class TvolumeParam
{
	IMMDevice *pEndpoint;
	HMIXEROBJ hMix;
	int mxId, whichMxId;
	int rec, record;
	TCHAR *which, *line;
	TCHAR *endpointName, *endpointNameLong;
	int value;
	int action;
	bool match;
	int popupLine;
	LPWSTR defaultAudioDeviceId;
	bool isDefaultEndpoint;
	MIXERLINE mixerLine;
	TCHAR *names[10];
	LPWSTR endpointId;

	bool IsMatch();
	void volume1();
	void volume2(DWORD controlType, int m);
	void volumeVista();
	void volumeVista1();
	void volumeVista2();
	void TvolumeParam::controls(IPart *pPartNext, LPWSTR ppwstrName);
	HRESULT part(IPart *pPartPrev, DataFlow flow);
	HRESULT conector(IConnector *pConnFrom, DataFlow flow);
	HRESULT endpoint();
	void propValue(TCHAR *&s, IPropertyStore *pProps, const PROPERTYKEY &key);
public:
	void volume(TCHAR *which1, int _value, int _action);
};

TendpointName *endpointNameList;

#define EXIT_ON_ERROR(hres)  \
	if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
	{ (punk)->Release(); (punk) = NULL; }

HRESULT conector(IConnector *pConnFrom, DataFlow flow);

const GUID KSNODETYPE_VOLUME={0x3A5ACC00L, 0xC557, 0x11D0, {0x8A, 0x2B, 0x00, 0xA0, 0xC9, 0x25, 0x5A, 0xC1}};
const GUID KSNODETYPE_TONE={0x7607E580L, 0xC557, 0x11D0, {0x8A, 0x2B, 0x00, 0xA0, 0xC9, 0x25, 0x5A, 0xC1}};
const GUID KSNODETYPE_MUTE={0x02B223C0L, 0xC557, 0x11D0, {0x8A, 0x2B, 0x00, 0xA0, 0xC9, 0x25, 0x5A, 0xC1}};
//---------------------------------------------------------------------------

int textY(int i)
{
	return 6+(fontH+9)*i;
}

int streqi(TCHAR *s1, TCHAR *s2, int len)
{
	return !_tcsnicmp(s1, s2, len) && s2[len]==0;
}

void setCurVolume(int line, int mute, int value)
{
	if(line>=0 && curVolume[line][mute]!=value){
		curVolume[line][mute]=value;
		//redraw popup window
		if(popupVolume.hWnd){
			RECT rc;
			rc.left=1;
			rc.right=popupVolume.width-1;
			rc.top=textY(line);
			rc.bottom=rc.top+fontH;
			InvalidateRect(popupVolume.hWnd, &rc, FALSE);
		}
	}
}

//get i item from the "Show these audio lines" option
bool getVolumeName(int i, TCHAR *&name, int &len, int &mxId, int &rec, bool display)
{
	TCHAR *s, *e, *eq;

	mxId=0;
	rec=0;
	s=volumeStr;
	if(!*s && i==0){
	l0:
		name = _T("Mixer");
		len=5;
		return true;
	}
	do{
		e=_tcschr(s, ',');
		if(!i--){
			//i-th item found
			if(!e) e=_tcschr(s, 0);
			//parse display name
			eq= _tcschr(s, '=');
			if(eq && eq<e){
				if(display){
					name= s;
					len= int(eq-s);
					return true;
				}
				s= eq+1;
			}
			for(;;){
				//skip spaces
				while(*s==' ') s++;
				if(e==s) goto l0;
				//parse record and sound card number
				if((*s=='r' || *s=='R') && s[1]==':'){
					rec=1;
					s+=2;
					continue;
				}
				if(*s>'0' && *s<='9' && s[1]==':'){
					mxId= *s-'1';
					s+=2;
					continue;
				}
				name=s;
				len= int(e-s);
				return true;
			}
		}
		if(!e) break;
		s=e;
		do{ s++; } while(*s==' ');
	} while(*s);
	//i is too big
	return false;
}

int volumeH()
{
	TCHAR *name;
	int n, len, mxId, rec;

	for(n=0; getVolumeName(n, name, len, mxId, rec, true); n++);
	return textY(n)-2;
}

bool TvolumeParam::IsMatch()
{
	int l, len, x, r;
	TCHAR *name;

	match=false;
	if(mxId==whichMxId && rec==record){
		for(TCHAR **u=names; *u; u++){
			if(!_tcsicmp(*u, which)){ match=true; break; }
		}
	}
	for(l=0;; l++){
		if(!getVolumeName(l, name, len, x, r, false)){
			if(!match) return false; //this line is not "which" and is not visible in popup window
			popupLine=-1; //it is not displayed, but volume will be changed
			return true;
		}
		if(x==mxId && r==rec){
			for(TCHAR **u=names; *u; u++){
				if(streqi(name, *u, len)){
					popupLine=l;
					return true;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
//  Windows Vista and newer - device topology and endpoint API
//---------------------------------------------------------------------------

void TvolumeParam::controls(IPart *pPartNext, LPWSTR ppwstrName)
{
	UINT nControl=0;
	IControlInterface *pInterfaceDesc=0;
	HRESULT hr;
	UINT j, c;
	int vol;

	hr= pPartNext->GetControlInterfaceCount(&nControl);
	EXIT_ON_ERROR(hr);

	for(j=0; j<nControl; j++){
		hr= pPartNext->GetControlInterface(j, &pInterfaceDesc);
		EXIT_ON_ERROR(hr);
		IID controlIID;
		hr= pInterfaceDesc->GetIID(&controlIID);
		EXIT_ON_ERROR(hr);

		names[0]=names[1]=0;
		if(controlIID==__uuidof(IAudioBass)){
			names[0] = _T("Bass");
		}
		if(controlIID==__uuidof(IAudioTreble)){
			names[0] = _T("Treble");
		}
		if(!wcsncmp(ppwstrName, L"Wave", 4) && controlIID==__uuidof(IAudioVolumeLevel)){
			names[0] = _T("Wave");
		}
		if(names[0] && IsMatch()){
			IPerChannelDbLevel *pVolume;
			hr=pPartNext->Activate(CLSCTX_ALL, controlIID, (void**)&pVolume);
			EXIT_ON_ERROR(hr);
			UINT nChannel;
			pVolume->GetChannelCount(&nChannel);

			vol=0;
			for(c = 0; c<nChannel; c++){
				float level, l1, l2, step;
				pVolume->GetLevel(c, &level);
				pVolume->GetLevelRange(c, &l1, &l2, &step);
				float rng = l2-l1;
				int i=(int)((level-l1)*100/rng+0.5);
				if(match){
					int i0=i;
					if(action==0){ //increase or decrease volume
						i+=value;
						aminmax(i, 0, 100);
						float incr = value*rng/100;
						if(value<0){
							if(-incr<step) incr=-step;
						}
						else{
							if(incr<step) incr=step;
						}
						level+=incr;
					}
					else if(action==2){ //set volume
						i=value;
						level=i*rng/100+l1;
						if(level<0) level-=step/2;
						else level+=step/2;
					}
					if(i!=i0) pVolume->SetLevel(c, level, 0);
				}
				amin(vol, i);
			}
			setCurVolume(popupLine, 0, vol);
			pVolume->Release();
		}
		/*else if(controlIID==__uuidof(IAudioMute)){
			IAudioMute * pMute;
			hr=pPartNext->Activate(CLSCTX_ALL,controlIID,(void**)&pMute);
			EXIT_ON_ERROR(hr);
			BOOL m;
			pMute->GetMute(&m);
			///
			pMute->Release();
			}*/
		pInterfaceDesc->Release();
	}
Exit:;
}

HRESULT TvolumeParam::part(IPart *pPartPrev, DataFlow flow)
{
	HRESULT hr = S_OK;
	IConnector *pConnFrom = NULL;
	IConnector *pConnTo = NULL;
	IPart *pPartNext = NULL;
	PartType parttype;
	IPartsList *pParts=0;
	UINT pCount=0;
	UINT i;
	LPWSTR ppwstrName=0;
	GUID subType;

	// Follow downstream link to next part.
	hr = (pPartPrev->*(flow==Out ? &IPart::EnumPartsOutgoing : &IPart::EnumPartsIncoming))(&pParts);
	if(hr==0x80070490) goto Exit;
	EXIT_ON_ERROR(hr);

	hr= pPartPrev->GetName(&ppwstrName);
	EXIT_ON_ERROR(hr);

	pPartPrev->GetSubType(&subType);

	if(subType==KSNODETYPE_VOLUME || subType==KSNODETYPE_MUTE || subType==KSNODETYPE_TONE){
		controls(pPartPrev, ppwstrName);
	}
	CoTaskMemFree(ppwstrName);


	hr = pParts->GetCount(&pCount);
	EXIT_ON_ERROR(hr);
	for(i=0; i<pCount; i++){
		hr = pParts->GetPart(i, &pPartNext);
		EXIT_ON_ERROR(hr);

		hr = pPartNext->GetPartType(&parttype);
		EXIT_ON_ERROR(hr);

		if(parttype == Connector)
		{
			// We've reached the output connector that
			// lies at the end of this device topology.
			hr = pPartNext->QueryInterface(__uuidof(IConnector), (void**)&pConnFrom);
			EXIT_ON_ERROR(hr);

			conector(pConnFrom, flow);

		}
		else{

			part(pPartNext, flow);
		}
	}
Exit:
	SAFE_RELEASE(pParts);
	SAFE_RELEASE(pConnFrom);
	SAFE_RELEASE(pConnTo);
	SAFE_RELEASE(pPartNext);
	return hr;
}

HRESULT TvolumeParam::conector(IConnector *pConnFrom, DataFlow flow)
{
	HRESULT hr = S_OK;
	IConnector *pConnTo = NULL;
	DataFlow flowTo;
	IPart *pPartPrev = NULL;

	BOOL bConnected;
	hr = pConnFrom->IsConnected(&bConnected);
	EXIT_ON_ERROR(hr);

	// Does this connector connect to another device?
	if(bConnected == FALSE)
	{
		// This is the end of the data path that
		// stretches from the endpoint device to the
		// system bus or external bus. Verify that
		// the connection type is Software_IO.
		ConnectorType  connType;
		hr = pConnFrom->GetType(&connType);
		EXIT_ON_ERROR(hr);

		if(connType == Software_IO)
		{
			goto Exit;  // finished
		}
		EXIT_ON_ERROR(hr = E_FAIL)
	}

	hr = pConnFrom->GetDataFlow(&flowTo);
	EXIT_ON_ERROR(hr);
	if(flowTo==flow)
	{
		// Get the connector in the next device topology,
		// which lies on the other side of the connection.
		hr = pConnFrom->GetConnectedTo(&pConnTo);
		EXIT_ON_ERROR(hr);

		// Get the connector's IPart interface.
		hr = pConnTo->QueryInterface(__uuidof(IPart), (void**)&pPartPrev);
		EXIT_ON_ERROR(hr);
		SAFE_RELEASE(pConnTo);

		part(pPartPrev, flow);
	}
Exit:
	SAFE_RELEASE(pConnTo);
	SAFE_RELEASE(pPartPrev);
	return hr;
}

HRESULT TvolumeParam::endpoint()
{
	HRESULT hr = S_OK;
	DataFlow flow;
	IDeviceTopology *pDeviceTopology = NULL;
	IConnector *pConnFrom = NULL;

	// Get the endpoint device's IDeviceTopology interface.
	hr = pEndpoint->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDeviceTopology);
	EXIT_ON_ERROR(hr);

	// The device topology for an endpoint device always
	// contains just one connector (connector number 0).
	hr = pDeviceTopology->GetConnector(0, &pConnFrom);
	EXIT_ON_ERROR(hr);

	// Make sure that this is a capture device.
	hr = pConnFrom->GetDataFlow(&flow);
	EXIT_ON_ERROR(hr);

	conector(pConnFrom, flow);

Exit:
	SAFE_RELEASE(pDeviceTopology);
	SAFE_RELEASE(pConnFrom);
	return hr;
}


void TvolumeParam::volumeVista2()
{
	IAudioEndpointVolume *eptvol;
	if(SUCCEEDED(pEndpoint->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&eptvol))){
		float f;
		int i, i0;
		if(SUCCEEDED(eptvol->GetMasterVolumeLevelScalar(&f))){
			i = (int)(f*100+0.5);
			if(match){
				i0 = i;
				if(action==0){ //increase or decrease volume
					i+=value;
					aminmax(i, 0, 100);
				}
				else if(action==2){ //set volume
					i=value;
				}
				if(i!=i0) eptvol->SetMasterVolumeLevelScalar(i / 100.0f, NULL);
			}
			//redraw popup window
			setCurVolume(popupLine, 0, i);
		}

		BOOL b, b0;
		if(SUCCEEDED(eptvol->GetMute(&b))){
			if(match){
				b0=b;
				if(action==1){ //toggle mute
					b=!b;
				}
				else if(action==3){ //set mute
					oldMute= b;
					b= value;
				}
				if(b!=b0) eptvol->SetMute(b, NULL);
			}
			//redraw popup window
			setCurVolume(popupLine, 1, b);
		}

		eptvol->Release();
	}
}

void TvolumeParam::volumeVista1()
{
	int n=0;

	if(endpointName) names[n++]=endpointName;
	if(endpointNameLong) names[n++]=endpointNameLong;
	if(defaultAudioDeviceId!=0){
		isDefaultEndpoint = !wcscmp(endpointId, defaultAudioDeviceId);
	}
	else{
		isDefaultEndpoint = !_tcsicmp(_T("Speakers"), endpointName);
	}
	if(isDefaultEndpoint) names[n++] = _T("Mixer");
	names[n]=0;
	if(IsMatch()) volumeVista2();

	//search topology of default endpoint, but not if hotkey parameter and options are Mixer
	if(!rec && isDefaultEndpoint){
		if(!record && _tcsicmp(which, _T("Mixer")) || *volumeStr && _tcsicmp(volumeStr, _T("Mixer")))
			endpoint();
	}
}

void TvolumeParam::propValue(TCHAR *&s, IPropertyStore *pProps, const PROPERTYKEY &key)
{
	s=0;
	PROPVARIANT var;
	PropVariantInit(&var);
	if(SUCCEEDED(pProps->GetValue(key, &var))){
		cpStr(s, var.pwszVal);
	}
	PropVariantClear(&var);
}

//action: 0= Volume +/-, 1= Mute On/Off, 2= Set volume, 3= Set mute, 4= Read value and redraw if changed
void TvolumeParam::volumeVista()
{
	IMMDeviceEnumerator *pEnumerator;
	IMMDeviceCollection *pCollection;
	IPropertyStore *pProps;

	CoInitialize(0);
	if(SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator))){
		//enumerate all audio lines
		for(rec=0; rec<2; rec++){
			//get default audio line
			defaultAudioDeviceId=0;
			if(SUCCEEDED(pEnumerator->GetDefaultAudioEndpoint(rec ? eCapture : eRender, eMultimedia, &pEndpoint))){
				pEndpoint->GetId(&defaultAudioDeviceId);
				pEndpoint->Release();
			}
			if(SUCCEEDED(pEnumerator->EnumAudioEndpoints(rec ? eCapture : eRender,
					DEVICE_STATE_ACTIVE, &pCollection))){
				UINT count = 0;
				pCollection->GetCount(&count);
				for(ULONG i = 0; i < count; i++){
					if(SUCCEEDED(pCollection->Item(i, &pEndpoint))){
						//get id
						if(SUCCEEDED(pEndpoint->GetId(&endpointId))){
							//get endpoint name from cache
							endpointName=endpointNameLong=0;
							for(TendpointName *en=endpointNameList; en; en=en->nxt){
								if(!wcscmp(endpointId, en->id)){
									endpointName= en->name;
									endpointNameLong= en->nameLong;
									break;
								}
							}
							if(!endpointName){
								//get endpoint name from property store
								if(SUCCEEDED(pEndpoint->OpenPropertyStore(STGM_READ, &pProps))){
									propValue(endpointName, pProps, PKEY_Device_DeviceDesc);
									propValue(endpointNameLong, pProps, PKEY_Device_FriendlyName);
									pProps->Release();

									//add name to cache
									TendpointName *e = new TendpointName();
									e->name= endpointName;
									e->nameLong= endpointNameLong;
									e->id= new WCHAR[wcslen(endpointId)+1];
									wcscpy(e->id, endpointId);
									e->nxt= endpointNameList;
									endpointNameList= e;
								}
							}
							volumeVista1();

							CoTaskMemFree(endpointId);
						}
						pEndpoint->Release();
					}
				}
				pCollection->Release();
			}
			CoTaskMemFree(defaultAudioDeviceId);
		}
		pEnumerator->Release();
	}
	CoUninitialize();
}

//---------------------------------------------------------------------------
//  Windows XP and older - winmm.dll
//---------------------------------------------------------------------------

//action: 0= Volume +/-, 1= Mute On/Off, 2= Set volume, 3= Set mute, 4= Read value and redraw if changed
//m: 0= controlType is volume, 1= controlType is mute
void TvolumeParam::volume2(DWORD controlType, int m)
{
	int minBound, maxBound, rng, incr;
	ULONG mcu, mcu0;
	MIXERLINECONTROLS mlc;
	MIXERCONTROL mc;
	MIXERCONTROLDETAILS mcd;

	mlc.cbStruct=sizeof(MIXERLINECONTROLS);
	mlc.cControls=1;
	mlc.cbmxctrl=sizeof(MIXERCONTROL);
	mlc.pamxctrl=&mc;
	mlc.dwLineID=mixerLine.dwLineID;
	mlc.dwControlType= controlType;
	if(mixerGetLineControls(hMix, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE)==MMSYSERR_NOERROR){
		mcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
		mcd.dwControlID=mc.dwControlID;
		mcd.cChannels=1;
		mcd.cMultipleItems=0;
		mcd.cbDetails=4;
		mcd.paDetails=&mcu;
		if(mixerGetControlDetails(hMix, &mcd, MIXER_GETCONTROLDETAILSF_VALUE)==MMSYSERR_NOERROR){
			minBound= ((LONG*)&mc.Bounds)[0];
			maxBound= ((LONG*)&mc.Bounds)[1];
			rng = maxBound - minBound;
			if(match){
				mcu0=mcu;
				if(!m){
					//change volume or bass/treble
					incr= (rng<1000) ? (value*rng)/100 : value*(rng/100);
					if(action==0){
						if(incr>0 && mcu>unsigned(maxBound-incr)) mcu=maxBound;
						else if(incr<0 && mcu<unsigned(minBound-incr)) mcu=minBound;
						else mcu+=incr;
					}
					else if(action==2){
						mcu=incr+minBound;
					}
				}
				else{
					//mute
					if(action==1) mcu= !mcu;
					else if(action==3){
						oldMute= mcu0;
						mcu= value;
					}
				}
				if(mcu!=mcu0) mixerSetControlDetails(hMix, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
			}

			setCurVolume(popupLine, m, (rng<1000) ? (mcu*100)/rng : mcu/(rng/100));
		}
	}
}

//volume action for one audio line
void TvolumeParam::volume1()
{
	names[0]=mixerLine.szShortName;
	names[1]=mixerLine.szName;
	names[2]=0;
	if(mixerLine.dwComponentType==MIXERLINE_COMPONENTTYPE_DST_SPEAKERS){
		names[2]=_T("Mixer");
		names[3]=0;
	}
	if(IsMatch()){
		//audio line usually has a volume and a mute control
		volume2(MIXERCONTROL_CONTROLTYPE_VOLUME, 0);
		volume2(MIXERCONTROL_CONTROLTYPE_MUTE, 1);
	}
}

//volume action for all devices and all audio lines
void TvolumeParam::volume(TCHAR *which1, int _value, int _action)
{
	this->value=_value;
	this->action=_action;
	record=0;
	whichMxId=mxId=0;
	if(!which1) which1 = _T("");
l1:
	while(*which1==' ') which1++;
	if((*which1=='r' || *which1=='R') && which1[1]==':'){
		record=1;
		which1+=2;
		goto l1;
	}
	if(*which1>'0' && *which1<='9' && which1[1]==':'){
		whichMxId= *which1-'1';
		which1+=2;
		goto l1;
	}
	if(!*which1) which1 = _T("Mixer");
	which=which1;

	memset(curVolume, -1, sizeof(curVolume));
	if(isVista){ volumeVista(); return; }

	int numDevs= mixerGetNumDevs();
	for(mxId=0; mxId<numDevs; mxId++){
		if(mixerOpen((HMIXER*)&hMix, mxId, 0, 0, 0)==MMSYSERR_NOERROR){
			mixerLine.cbStruct=sizeof(MIXERLINE);
			for(rec=0; rec<2; rec++){
				mixerLine.dwComponentType= rec ? MIXERLINE_COMPONENTTYPE_DST_WAVEIN : MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
				if(mixerGetLineInfo(hMix, &mixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE)==MMSYSERR_NOERROR){
					//speakers volume
					volume1();
					//bass
					names[0] = _T("Bass");
					names[1]=0;
					if(IsMatch()) volume2(MIXERCONTROL_CONTROLTYPE_BASS, 0);
					//treble
					names[0] = _T("Treble");
					if(IsMatch()) volume2(MIXERCONTROL_CONTROLTYPE_TREBLE, 0);
					//source line volume
					for(int src=mixerLine.cConnections-1; src>=0; src--){
						mixerLine.dwSource=src;
						if(mixerGetLineInfo(hMix, &mixerLine, MIXER_GETLINEINFOF_SOURCE)==MMSYSERR_NOERROR){
							volume1();
						}
					}
				}
			}
			mixerClose((HMIXER)hMix);
		}
	}
}

void volume(TCHAR *which, int value, int action)
{
	TvolumeParam data;
	data.volume(which, value, action);
}

