#include <streams.h>
#include<Commctrl.h>
//#include <tchar.h>
#include "ScreenPropertyPage.h"
#include "resource.h"


//
// CreateInstance
//
// Override CClassFactory method.
// Set lpUnk to point to an IUnknown interface on a new NullIPProperties object
// Part of the COM object instantiation mechanism
//
CUnknown * WINAPI CScreenCapProp::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	CUnknown *punk = new CScreenCapProp(lpunk, phr);
	if (punk == NULL) 
	{
		*phr = E_OUTOFMEMORY;
	}
	return punk;
}


CScreenCapProp::CScreenCapProp(IUnknown *pUnknown, HRESULT *phr) : 
CBasePropertyPage(NAME("ScreenProp"), pUnknown, IDD_PROPPAGE, IDS_PROPPAGE_TITLE) 
	,m_hwnd(NULL)	//,m_pGray(0)
{ 

	m_pCapSize = NULL;
}

HRESULT CScreenCapProp::OnConnect(IUnknown *pUnk)
{
	if (pUnk == NULL)
	{
		return E_POINTER;
	}
	ASSERT(m_pCapSize == NULL);
	return pUnk->QueryInterface(IID_IScreenCaptureSize, reinterpret_cast<void**>(&m_pCapSize));
}


HRESULT CScreenCapProp::OnActivate(void)
{ 
	int nCount =  sizeof(video_desc)/sizeof(MediaDescriptor);
  
	SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(m_hwnd,IDC_COMBO2, CB_RESETCONTENT, 0, 0);

	int dstWidth = m_pCapSize->GetDstWidth();
	int dstHeight = m_pCapSize->GetDstHeight(); 
	int srcWidth = m_pCapSize->GetSrcWidth();
	int srcHeight = m_pCapSize->GetSrcHeight();

	bool bIsInclude = m_pCapSize->IsIncludeDefaultSize();

	 
	if(bIsInclude)
	{   
		for (int i = 1; i <nCount; i++)
		{
			TCHAR szSize[64];
			if( srcWidth == video_desc[i].width && srcHeight == video_desc[i].height )
			{
				wsprintf(szSize, TEXT("%d x %d  (default)\0"),m_pCapSize->GetSrcWidth(),m_pCapSize->GetSrcHeight()); 
			}
			else{
				wsprintf(szSize, TEXT("%d x %d\0"),video_desc[i].width,video_desc[i].height); 
			} 
			//  SendMessage(m_hSize, CB_SETITEMDATA, i, (LPARAM)szSize);
			SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)szSize);


			// 找到当前采集尺寸
			if( dstWidth == video_desc[i].width && dstHeight == video_desc[i].height )
			{
                 SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_SETCURSEL ,i-1, 0);
			}
		} 
	}
	else{
		TCHAR szSize[64];
		wsprintf(szSize, TEXT("%d x %d  (default)\0"),m_pCapSize->GetSrcWidth(),m_pCapSize->GetSrcHeight()); 
		SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)szSize);
		SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_SETCURSEL ,0, 0);
		for (int i = 1; i < nCount; i++)
		{
			TCHAR szSize[64];
		    wsprintf(szSize, TEXT("%d x %d\0"),video_desc[i].width,video_desc[i].height);  
			//  SendMessage(m_hSize, CB_SETITEMDATA, i, (LPARAM)szSize);
			SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)szSize);
			if( dstWidth == video_desc[i].width && dstHeight == video_desc[i].height )
			{
				SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_SETCURSEL ,i, 0);
			}
		}  
	} 

	TCHAR szColSpace [64];
	// szCompression = TEXT("RGB24\0");
	SendDlgItemMessage(m_hwnd,IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)TEXT("RGB32\0")); 
	SendDlgItemMessage(m_hwnd,IDC_COMBO2, CB_SETCURSEL , 0, 0);


	EnableWindow(GetDlgItem(m_hwnd, IDC_SPIN2), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT1), TRUE);

	int iFrameRate = m_pCapSize->GetFrameRate();
    m_nFrameRate = iFrameRate;
	//SetDlgItemInt(m_hwnd, IDC_EDIT1, iFrameRate, TRUE); 
	 HWND h = GetDlgItem(m_hwnd, IDC_EDIT1); 
	 SendMessage(GetDlgItem(m_hwnd, IDC_SPIN2), UDM_SETRANGE32, 1, 10 ); 
	 SendMessage(GetDlgItem(m_hwnd, IDC_SPIN2), UDM_SETPOS32, 0,iFrameRate ); 
	return NOERROR;
	 
}


BOOL CScreenCapProp::OnReceiveMessage(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Get windows' handles
			ASSERT (m_hwnd == NULL) ;
			m_hwnd = hwnd ;  
			break;
		}

	case WM_COMMAND:
		{ 
			if (HIWORD(wParam) == CBN_SELCHANGE) 
			{
				switch ( LOWORD(wParam))
				{ 
				case IDC_COMBO1:  
					SetDirty();
					break;  
				}
			} 
			 
		} 
	case WM_VSCROLL:
		{
			HWND  h = GetDlgItem(m_hwnd, IDC_SPIN2);   
			if( (HWND) lParam  ==  h)
			{ 
                short int nPos = (short int)HIWORD(wParam);
				m_nFrameRate = nPos;
				SetDirty();
			}
		}
	} // Switch.

	// Let the parent class handle the message.
	return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}



void CScreenCapProp::SetDirty()
{
	m_bDirty = TRUE;
	if (m_pPageSite)
	{
		m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
	}
}


// Override CBasePropertyPage method.
// Release the private interface, release the upstream pin.
HRESULT CScreenCapProp::OnDisconnect()
{
	// Release of Interface
	if (m_pCapSize == NULL)
		return E_UNEXPECTED;
	m_pCapSize->Release();
	m_pCapSize = NULL;

	return NOERROR;
} // OnDisconnect


HRESULT CScreenCapProp::OnApplyChanges()
{

	TCHAR szText[128] = {0};

	// CB_GETCURSEL 
	int cbIndex = SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_GETCURSEL , 0,0);  
	//SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_GETLBTEXT ,cbIndex, LPARAM(szText));

	bool bIsInclude = m_pCapSize->IsIncludeDefaultSize();
	int w,h;
	if(bIsInclude)
	{
		w = video_desc[cbIndex+1].width;
		h = video_desc[cbIndex+1].height;
	}
	else
	{
		if(cbIndex == 0)
		{ 
			w = m_pCapSize->GetSrcWidth();
			h = m_pCapSize->GetSrcHeight();
		}
		else{
			w = video_desc[cbIndex].width;
			h = video_desc[cbIndex].height;
		}
	}

    m_pCapSize->SetDstWidth(w);
	m_pCapSize->SetDstHeight(h);
	
	m_pCapSize->SetFrameRate(m_nFrameRate);

	m_pCapSize->SetChange(true);
	 
	 

	//cbIndex = SendDlgItemMessage(m_hwnd,IDC_COMBO2, CB_GETCURSEL , 0,0);
	//SendDlgItemMessage(m_hwnd,IDC_COMBO1, CB_GETLBTEXT ,cbIndex, LPARAM(szText)); 
	//if (_tcscmp(szText, TEXT("RGB24")) == 0 )
	 

	return NOERROR; 
}