#include <streams.h>

#include "CamStudioScreenCapture.h"
#include "Guids.h"
#include "CamStudioFilterHelper.h"

#include <wmsdkidl.h>

//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CCamStudioPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
	if (guidPropSet != AMPROPSETID_Pin) 
		return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY) 
		return E_PROP_ID_UNSUPPORTED;
	// We support getting this property, but not setting it.
	if (pTypeSupport) 
		*pTypeSupport = KSPROPERTY_SUPPORT_GET; 
	return S_OK;
}

HRESULT CCamStudioPin::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
	DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{
	// Set: we don't have any specific properties to set...that we advertise yet anyway, and who would use them anyway?
	return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CCamStudioPin::Get(
	REFGUID guidPropSet,   // Which property set.
	DWORD dwPropID,        // Which property in that set.
	void *pInstanceData,   // Instance data (ignore).
	DWORD cbInstanceData,  // Size of the instance data (ignore).
	void *pPropData,       // Buffer to receive the property data.
	DWORD cbPropData,      // Size of the buffer.
	DWORD *pcbReturned     // Return the size of the property.
	)
{
	if (guidPropSet != AMPROPSETID_Pin)             
		return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY)        
		return E_PROP_ID_UNSUPPORTED;
	if (pPropData == NULL && pcbReturned == NULL)  
		return E_POINTER;
	if (pcbReturned) 
		*pcbReturned = sizeof(GUID);
	if (pPropData == NULL)          
		return S_OK; // Caller just wants to know the size. 
	if (cbPropData < sizeof(GUID))  
		return E_UNEXPECTED;// The buffer is too small.

	*(GUID *)pPropData = PIN_CATEGORY_CAPTURE; // PIN_CATEGORY_PREVIEW ?
	return S_OK;
}


enum FourCC { FOURCC_NONE = 0, FOURCC_I420 = 100, FOURCC_YUY2 = 101, FOURCC_RGB32 = 102 };// from http://www.conaito.com/docus/voip-video-evo-sdk-capi/group__videocapture.html
//
// GetMediaType
//
// Prefer 5 formats - 8, 16 (*2), 24 or 32 bits per pixel
//
// Prefered types should be ordered by quality, with zero as highest quality.
// Therefore, iPosition =
//      0    Return a 24bit mediatype "as the default" since I guessed it might be faster though who knows
//      1    Return a 24bit mediatype
//      2    Return 16bit RGB565
//      3    Return a 16bit mediatype (rgb555)
//      4    Return 8 bit palettised format
//      >4   Invalid
// except that we changed the orderings a bit...
//
HRESULT CCamStudioPin::GetMediaType(int iPosition, CMediaType *pmt) // AM_MEDIA_TYPE basically == CMediaType
{
	CheckPointer(pmt, E_POINTER);
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	if(m_bFormatAlreadySet) {
		// you can only have one option, buddy, if setFormat already called. (see SetFormat's msdn)
		if(iPosition != 0)
			return E_INVALIDARG;
		VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();

		// Set() copies these in for us pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader); // calculates the size for us, after we gave it the width and everything else we already chucked into it
		// pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);
		// nobody uses sample size anyway :P

		pmt->Set(m_mt);
		VIDEOINFOHEADER *pVih1 = (VIDEOINFOHEADER*) m_mt.pbFormat;
		VIDEOINFO *pviHere = (VIDEOINFO  *) pmt->pbFormat;
		return S_OK;
	}

	// do we ever even get past here? hmm

	if(iPosition < 0)
		return E_INVALIDARG;

	// Have we run out of types?
	int size =  sizeof(video_desc)/sizeof(MediaDescriptor);
	if( m_bIncludeDefaultSize)
	{
		size -= 1;
	}

	if(iPosition >= size )
		return VFW_S_NO_MORE_ITEMS;

	VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
	if(NULL == pvi)
		return(E_OUTOFMEMORY);

	// Initialize the VideoInfo structure before configuring its members
	ZeroMemory(pvi, sizeof(VIDEOINFO));

	/*
	if(iPosition == 0) {
		// pass it our "preferred" which is 24 bits, since 16 is "poor quality" (really, it is), and I...think/guess that 24 is faster overall.
		iPosition = 1;//2;

		// 32 -> 24 (2): getdibits took 2.251ms
		// 32 -> 32 (1): getdibits took 2.916ms
		// except those particular numbers might be misleading in terms of total speed...hmm...
	}
	*/
	//  modify by dongmingyi
	pvi->bmiHeader.biCompression = BI_RGB;//video_desc[iPosition].biCompression;// BI_RGB;
	pvi->bmiHeader.biBitCount    = 32;//24;//video_desc[iPosition].biBitCount;// 32;
/**/

	if(m_bIncludeDefaultSize)
	{
		pvi->bmiHeader.biWidth      = video_desc[iPosition+1].width;// m_nCaptureWidth;
		pvi->bmiHeader.biHeight     = video_desc[iPosition+1].height;//m_nCaptureHeight;
	}
	else
	{
		if( iPosition == 0  )
		{
			pvi->bmiHeader.biWidth      =  m_nSrcWidth;// m_nCaptureWidth;
			pvi->bmiHeader.biHeight     =  m_nSrcHeight;//m_nCaptureHeight;
		}
		else
		{
			pvi->bmiHeader.biWidth      = video_desc[iPosition].width;// m_nCaptureWidth;
			pvi->bmiHeader.biHeight     = video_desc[iPosition].height;//m_nCaptureHeight;
		}
	}
 
   // pvi->bmiHeader.biClrUsed     = iPALETTE_COLORS;
/* 
	switch(iPosition)
	{

	case 1: 
		{
			// 32bit format

			// Since we use RGB888 (the default for 32 bit), there is
			// no reason to use BI_BITFIELDS to specify the RGB
			// masks [sometimes even if you don't have enough bits you don't need to anyway?]
			// Also, not everything supports BI_BITFIELDS ...
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount    = 32;
			pvi->bmiHeader.biWidth      =  m_nCaptureWidth;
			pvi->bmiHeader.biHeight     = m_nCaptureHeight;
			break;
		}


	case 2:

		{   // Return our 24bit format, same as above comments
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	 = 24;
			pvi->bmiHeader.biWidth      =  m_nCaptureWidth;
			pvi->bmiHeader.biHeight     =  m_nCaptureHeight;
			break;
		}

	case 3:
		{
			// 16 bit per pixel RGB565 BI_BITFIELDS

			// Place the RGB masks as the first 3 doublewords in the palette area
			for(int i = 0; i < 3; i++)
				pvi->TrueColorInfo.dwBitMasks[i] = bits565[i];

			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount    = 16;
			pvi->bmiHeader.biWidth      =  m_nCaptureWidth;
			pvi->bmiHeader.biHeight     =  m_nCaptureHeight;
			break;
		}


	case 4:
		{	// 16 bits per pixel RGB555

			// Place the RGB masks as the first 3 doublewords in the palette area
			for(int i = 0; i < 3; i++)
				pvi->TrueColorInfo.dwBitMasks[i] = bits555[i];

			// LODO ??? need? not need? BI_BITFIELDS? Or is this the default so we don't need it? Or do we need a different type that doesn't specify BI_BITFIELDS?
			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount    = 16;
			pvi->bmiHeader.biWidth      =  m_nCaptureWidth;
			pvi->bmiHeader.biHeight     = m_nCaptureHeight;
			break;
		}

	case 5:
		{   // 8 bit palettised

			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount    = 8;
			pvi->bmiHeader.biClrUsed     = iPALETTE_COLORS;
			pvi->bmiHeader.biWidth      =  m_nCaptureWidth;
			pvi->bmiHeader.biHeight     =  m_nCaptureHeight;
			break;
		}
	case 6: 
		{
			// 32bit format

			// Since we use RGB888 (the default for 32 bit), there is
			// no reason to use BI_BITFIELDS to specify the RGB
			// masks [sometimes even if you don't have enough bits you don't need to anyway?]
			// Also, not everything supports BI_BITFIELDS ...
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount    = 32;
			pvi->bmiHeader.biWidth      = 1280;
			pvi->bmiHeader.biHeight     = 720;
			break;
		}

		   
		case 6:
		{ // the i420 freak-o added just for FME's benefit...
		pvi->bmiHeader.biCompression = 0x30323449; // => ASCII "I420" is apparently right here...
		pvi->bmiHeader.biBitCount    = 12;
		//pvi->bmiHeader.biSizeImage = (  m_CaptureWindow.Width() * m_CaptureWindow.Height() *3)/2;
		pvi->bmiHeader.biSizeImage = (m_nCaptureWidth*m_nCaptureHeight *3)/2; 
		pmt->SetSubtype(&WMMEDIASUBTYPE_I420);
		break;
		}
		
	}*/

	// Now adjust some parameters that are the same for all formats
	pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER); 
	pvi->bmiHeader.biPlanes     = 1;
	pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader); // calculates the size for us, after we gave it the width and everything else we already chucked into it
	pmt->SetSampleSize(pvi->bmiHeader.biSizeImage); // use the above size

	pvi->bmiHeader.biClrImportant = 0;
	pvi->AvgTimePerFrame = m_rtFrameLength; // from our config or default
 
	if( (m_nSrcWidth == pvi->bmiHeader.biWidth) && (m_nSrcHeight == pvi->bmiHeader.biHeight))   // add by dongmingyi   如果屏幕的采集尺寸不需要resize，则帧率为10.
	{
		pvi->AvgTimePerFrame  =  UNITS / 10;
	} 

	SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
	SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

	pmt->SetType(&MEDIATYPE_Video);
	pmt->SetFormatType(&FORMAT_VideoInfo);
	pmt->SetTemporalCompression(FALSE);

	// Work out the GUID for the subtype from the header info.
	if(*pmt->Subtype() == GUID_NULL) 
	{
		const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
		pmt->SetSubtype(&SubTypeGUID);
	}

	return NOERROR;

} // GetMediaType

