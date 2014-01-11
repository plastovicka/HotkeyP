/*
 (C) Petr Lastovicka
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  

//#include <mmdeviceapi.h>
//#include <endpointvolume.h>
//#include <devicetopology.h>

#define DEVICE_STATE_ACTIVE      0x00000001

enum ERole {	
  eConsole,
  eMultimedia,
  eCommunications,
  ERole_enum_count
};

enum EDataFlow {
  eRender,
  eCapture,
  eAll,
  EDataFlow_enum_count
};

enum PartType {	
  Connector,
	Subunit
};

enum DataFlow {	
  In,
	Out
};

enum ConnectorType {	
  Unknown_Connector,
	Physical_Internal,
	Physical_External,
	Software_IO,
	Software_Fixed,
	Network
};

typedef interface IDeviceTopology IDeviceTopology;
typedef interface IPartsList IPartsList;


struct AUDIO_VOLUME_NOTIFICATION_DATA
{
  GUID guidEventContext;
  BOOL bMuted;
  float fMasterVolume;
  UINT nChannels;
  float afChannelVolumes[ 1 ];
};

#ifndef PROPERTYKEY_DEFINED
#define PROPERTYKEY_DEFINED
struct PROPERTYKEY {
  GUID fmtid;
  DWORD pid;
};
#endif

#undef DEFINE_PROPERTYKEY
#define DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const PROPERTYKEY DECLSPEC_SELECTANY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }

DEFINE_PROPERTYKEY(PKEY_Device_DeviceDesc, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 2);
DEFINE_PROPERTYKEY(PKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 14);
DEFINE_PROPERTYKEY(PKEY_DeviceInterface_FriendlyName,  0x026e516e, 0xb814, 0x414b, 0x83, 0xcd, 0x85, 0x6d, 0x6f, 0xef, 0x48, 0x22, 2); // DEVPROP_TYPE_STRING



#ifndef __IPropertyStore_INTERFACE_DEFINED__
#define __IPropertyStore_INTERFACE_DEFINED__
struct IPropertyStore : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetCount( 
    DWORD *cProps) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetAt( 
    DWORD iProp,
    PROPERTYKEY *pkey) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetValue( 
    const PROPERTYKEY & key,
    PROPVARIANT *pv) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetValue( 
    const PROPERTYKEY & key,
    const PROPVARIANT & propvar) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Commit( void) = 0;
};
#endif

struct IMMDevice : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE Activate( 
    REFIID iid,
    DWORD dwClsCtx,
    PROPVARIANT *pActivationParams,
    void **ppInterface) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE OpenPropertyStore( 
    DWORD stgmAccess,
    IPropertyStore **ppProperties) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetId( 
    LPWSTR *ppstrId) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetState( 
    DWORD *pdwState) = 0;
  
};

struct IMMNotificationClient : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE OnDeviceStateChanged( 
    LPCWSTR pwstrDeviceId,
    DWORD dwNewState) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE OnDeviceAdded( 
    LPCWSTR pwstrDeviceId) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE OnDeviceRemoved( 
    LPCWSTR pwstrDeviceId) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged( 
    EDataFlow flow,
    ERole role,
    LPCWSTR pwstrDefaultDeviceId) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE OnPropertyValueChanged( 
    LPCWSTR pwstrDeviceId,
    const PROPERTYKEY key) = 0;

};

struct IMMDeviceCollection : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetCount( 
    UINT *pcDevices) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE Item( 
    UINT nDevice,
    IMMDevice **ppDevice) = 0;
  
};

class DECLSPEC_UUID("BCDE0395-E52F-467C-8E3D-C4579291692E") MMDeviceEnumerator;

MIDL_INTERFACE("A95664D2-9614-4F35-A746-DE8DB63617E6")
IMMDeviceEnumerator : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE EnumAudioEndpoints( 
    EDataFlow dataFlow,
    DWORD dwStateMask,
    IMMDeviceCollection **ppDevices) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetDefaultAudioEndpoint( 
    EDataFlow dataFlow,
    ERole role,
    IMMDevice **ppEndpoint) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetDevice( 
    LPCWSTR pwstrId,
    IMMDevice **ppDevice) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE RegisterEndpointNotificationCallback( 
    IMMNotificationClient *pClient) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE UnregisterEndpointNotificationCallback( 
    IMMNotificationClient *pClient) = 0;
 
};


MIDL_INTERFACE("657804FA-D6AD-4496-8A60-352752AF4F89")
IAudioEndpointVolumeCallback : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE OnNotify( 
    AUDIO_VOLUME_NOTIFICATION_DATA *pNotify) = 0;
};

MIDL_INTERFACE("5CDF2C82-841E-4546-9722-0CF74078229A")
IAudioEndpointVolume : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE RegisterControlChangeNotify( 
    IAudioEndpointVolumeCallback *pNotify) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE UnregisterControlChangeNotify( 
    IAudioEndpointVolumeCallback *pNotify) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetChannelCount( 
    UINT *pnChannelCount) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE SetMasterVolumeLevel( 
    float fLevelDB,
    LPCGUID pguidEventContext) = 0;
    
  virtual HRESULT STDMETHODCALLTYPE SetMasterVolumeLevelScalar( 
    float fLevel,
    LPCGUID pguidEventContext) = 0;
    
  virtual HRESULT STDMETHODCALLTYPE GetMasterVolumeLevel( 
    float *pfLevelDB) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetMasterVolumeLevelScalar( 
    float *pfLevel) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE SetChannelVolumeLevel( 
    UINT nChannel,
    float fLevelDB,
    LPCGUID pguidEventContext) = 0;
    
  virtual HRESULT STDMETHODCALLTYPE SetChannelVolumeLevelScalar( 
    UINT nChannel,
    float fLevel,
    LPCGUID pguidEventContext) = 0;
    
  virtual HRESULT STDMETHODCALLTYPE GetChannelVolumeLevel( 
    UINT nChannel,
    float *pfLevelDB) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetChannelVolumeLevelScalar( 
    UINT nChannel,
    float *pfLevel) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE SetMute( 
    BOOL bMute,
    LPCGUID pguidEventContext) = 0;
    
  virtual HRESULT STDMETHODCALLTYPE GetMute( 
    BOOL *pbMute) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetVolumeStepInfo( 
    UINT *pnStep,
    UINT *pnStepCount) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE VolumeStepUp( 
    LPCGUID pguidEventContext) = 0;
    
  virtual HRESULT STDMETHODCALLTYPE VolumeStepDown( 
    LPCGUID pguidEventContext) = 0;
    
  virtual HRESULT STDMETHODCALLTYPE QueryHardwareSupport( 
    DWORD *pdwHardwareSupportMask) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetVolumeRange( 
    float *pflVolumeMindB,
    float *pflVolumeMaxdB,
    float *pflVolumeIncrementdB) = 0;
};

MIDL_INTERFACE("45d37c3f-5140-444a-ae24-400789f3cbf3")
IControlInterface : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetName(
    LPWSTR *ppwstrName) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetIID( 
    GUID *pIID) = 0;
};

MIDL_INTERFACE("A09513ED-C709-4d21-BD7B-5F34C47F3947")
IControlChangeNotify : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE OnNotify( 
    DWORD dwSenderProcessId,
    LPCGUID pguidEventContext) = 0;
};

MIDL_INTERFACE("AE2DE0E4-5BCA-4F2D-AA46-5D13F8FDB3A9")
IPart : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetName( 
    LPWSTR *ppwstrName) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetLocalId( 
    UINT *pnId) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetGlobalId( 
    LPWSTR *ppwstrGlobalId) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetPartType( 
    PartType *pPartType) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetSubType( 
    GUID *pSubType) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetControlInterfaceCount( 
    UINT *pCount) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetControlInterface( 
    UINT nIndex,
    IControlInterface **ppInterfaceDesc) = 0;

  virtual HRESULT STDMETHODCALLTYPE EnumPartsIncoming( 
    IPartsList **ppParts) = 0;

  virtual HRESULT STDMETHODCALLTYPE EnumPartsOutgoing( 
    IPartsList **ppParts) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetTopologyObject( 
    IDeviceTopology **ppTopology) = 0;

  virtual HRESULT STDMETHODCALLTYPE Activate( 
    DWORD dwClsContext,
    REFIID refiid,
    void **ppvObject) = 0;

  virtual HRESULT STDMETHODCALLTYPE RegisterControlChangeCallback( 
    REFGUID riid,
    IControlChangeNotify *pNotify) = 0;

  virtual HRESULT STDMETHODCALLTYPE UnregisterControlChangeCallback( 
    IControlChangeNotify *pNotify) = 0;

};

MIDL_INTERFACE("6DAA848C-5EB0-45CC-AEA5-998A2CDA1FFB")
IPartsList : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetCount( 
    UINT *pCount) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetPart( 
    UINT nIndex,
    IPart **ppPart) = 0;
};


MIDL_INTERFACE("9c2c4058-23f5-41de-877a-df3af236a09e")
IConnector : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetType( 
    ConnectorType *pType) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetDataFlow( 
    DataFlow *pFlow) = 0;

  virtual HRESULT STDMETHODCALLTYPE ConnectTo( 
    IConnector *pConnectTo) = 0;

  virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;

  virtual HRESULT STDMETHODCALLTYPE IsConnected( 
    BOOL *pbConnected) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetConnectedTo( 
    IConnector **ppConTo) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetConnectorIdConnectedTo( 
    LPWSTR *ppwstrConnectorId) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetDeviceIdConnectedTo( 
    LPWSTR *ppwstrDeviceId) = 0;
};

MIDL_INTERFACE("82149A85-DBA6-4487-86BB-EA8F7FEFCC71")
ISubunit : public IUnknown
{
};

MIDL_INTERFACE("2A07407E-6497-4A18-9787-32F79BD0D98F")
IDeviceTopology : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetConnectorCount( 
    UINT *pCount) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetConnector( 
    UINT nIndex,
    IConnector **ppConnector) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetSubunitCount( 
    UINT *pCount) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetSubunit( 
    UINT nIndex,
    ISubunit **ppSubunit) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetPartById( 
    UINT nId,
    IPart **ppPart) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetDeviceId( 
    LPWSTR *ppwstrDeviceId) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetSignalPath( 
    IPart *pIPartFrom,
    IPart *pIPartTo,
    BOOL bRejectMixedPaths,
    IPartsList **ppParts) = 0;

};

MIDL_INTERFACE("C2F8E001-F205-4BC9-99BC-C13B1E048CCB")
IPerChannelDbLevel : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE GetChannelCount( 
    UINT *pcChannels) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetLevelRange( 
    UINT nChannel,
    float *pfMinLevelDB,
    float *pfMaxLevelDB,
    float *pfStepping) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetLevel( 
    UINT nChannel,
    float *pfLevelDB) = 0;

  virtual HRESULT STDMETHODCALLTYPE SetLevel( 
    UINT nChannel,
    float fLevelDB,
    LPCGUID pguidEventContext) = 0;

  virtual HRESULT STDMETHODCALLTYPE SetLevelUniform( 
    float fLevelDB,
    LPCGUID pguidEventContext) = 0;

  virtual HRESULT STDMETHODCALLTYPE SetLevelAllChannels( 
    float aLevelsDB[],
    ULONG cChannels,
    LPCGUID pguidEventContext) = 0;
};

MIDL_INTERFACE("7FB7B48F-531D-44A2-BCB3-5AD5A134B3DC")
IAudioVolumeLevel : public IPerChannelDbLevel
{
};

MIDL_INTERFACE("A2B1A1D9-4DB3-425D-A2B2-BD335CB3E2E5")
IAudioBass : public IPerChannelDbLevel
{
};

MIDL_INTERFACE("0A717812-694E-4907-B74B-BAFA5CFDCA7B")
IAudioTreble : public IPerChannelDbLevel
{
};

MIDL_INTERFACE("DF45AEEA-B74A-4B6B-AFAD-2366B6AA012E")
IAudioMute : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetMute( 
      BOOL bMuted,
      LPCGUID pguidEventContext) = 0;
  
  virtual HRESULT STDMETHODCALLTYPE GetMute( 
      BOOL *pbMuted) = 0;
};
