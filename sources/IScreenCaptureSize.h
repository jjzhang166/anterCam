#pragma once

 
// {6B69D777-2974-41AE-84C0-05141BB01C58}
static const GUID IID_IScreenCaptureSize = 
{ 0x6b69d777, 0x2974, 0x41ae, { 0x84, 0xc0, 0x5, 0x14, 0x1b, 0xb0, 0x1c, 0x58 } };



DECLARE_INTERFACE_(IScreenCaptureSize, IUnknown)
{ 
	STDMETHOD(SetDstWidth)(UINT iDstWidth) PURE;
	STDMETHOD(SetDstHeight)(UINT iDstHeight) PURE;
	STDMETHOD_(UINT, GetDstHeight) (void) PURE;
	STDMETHOD_(UINT, GetDstWidth) (void) PURE;
	STDMETHOD_(UINT, GetSrcHeight) (void) PURE;
	STDMETHOD_(UINT, GetSrcWidth) (void) PURE; 
	STDMETHOD_(bool, IsIncludeDefaultSize) (void) PURE; 
	STDMETHOD(SetChange)(bool bChange) PURE;

	STDMETHOD(SetFrameRate)(UINT iFrameRate) PURE;
	STDMETHOD_(UINT, GetFrameRate) (void) PURE; 
};