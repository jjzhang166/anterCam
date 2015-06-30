#include <streams.h>
#include "ScreenPropertyPage.h"
//#include "resource.h"
CScreenProp::CScreenProp(IUnknown *pUnknown) : 
CBasePropertyPage(NAME("ScreenProp"), pUnknown, IDD_PROPPAGE, IDS_PROPPAGE_TITLE) 
//,m_pGray(0)
{ 
}


BOOL CScreenProp::OnReceiveMessage(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
	/*
switch (uMsg)
{
case WM_COMMAND:
if (LOWORD(wParam) == IDC_DEFAULT)
{
// User clicked the 'Revert to Default' button.
m_lNewVal = SATURATION_DEFAULT;
m_pGray->SetSaturation(m_lNewVal); 
// Update the slider control.
SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETPOS, 1,
m_lNewVal);
SetDirty();
return (LRESULT) 1;
}
break; 
case WM_HSCROLL:
{
// User moved the slider.
switch(LOWORD(wParam))
{
case TB_PAGEDOWN:
case SB_THUMBTRACK:
case TB_PAGEUP:
m_lNewVal = SendDlgItemMessage(m_Dlg, IDC_SLIDER1,
TBM_GETPOS, 0, 0);
m_pGray->SetSaturation(m_lNewVal);
SetDirty();
}
return (LRESULT) 1;
}
} // Switch.
*/
// Let the parent class handle the message.
return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}