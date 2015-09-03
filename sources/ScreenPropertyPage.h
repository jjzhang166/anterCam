#pragma once
#include "IScreenCaptureSize.h"
#include "CamStudioScreenCapture.h"
class CScreenCapProp : public CBasePropertyPage
{
private:
    CScreenCapProp(IUnknown *pUnknown, HRESULT *phr);
	virtual ~CScreenCapProp() { };
 
public:
	 static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    DECLARE_IUNKNOWN;
	virtual HRESULT OnConnect(IUnknown *pUnknown);
	virtual HRESULT OnDisconnect();
	virtual HRESULT OnActivate();
	virtual HRESULT OnDeactivate() { return NOERROR; };
	virtual HRESULT OnApplyChanges();
	virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	void SetDirty();
private:
	IScreenCaptureSize *m_pCapSize; // Pointer to the filter's custom interface.
	int m_lVal ;// Store the old value, so we can revert.
	int m_lNewVal; // New value.

	  //HWND      m_hSize;  

     HWND                m_hwnd ;                    //  property page HWND

  	UINT m_nSrcWidth;
	UINT m_nSrcHeight;

	UINT m_nFrameRate;
};