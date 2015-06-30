#pragma once

class CScreenProp : public CBasePropertyPage
{
private:
    CScreenProp(IUnknown *pUnknown);
	virtual ~CScreenProp() { };

public:
	
	virtual HRESULT OnConnect(IUnknown *pUnknown) { return NOERROR; };
	virtual HRESULT OnDisconnect() { return NOERROR; };
	virtual HRESULT OnActivate() { return NOERROR; };
	virtual HRESULT OnDeactivate() { return NOERROR; };
	virtual HRESULT OnApplyChanges() { return NOERROR; };
	virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
private:
	//ISaturation *m_pGray; // Pointer to the filter's custom interface.
	long m_lVal ;// Store the old value, so we can revert.
	long m_lNewVal; // New value.
};