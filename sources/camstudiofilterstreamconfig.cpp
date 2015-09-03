#include <streams.h>

#include "CamStudioScreenCapture.h"
#include "Guids.h"
#include "CamStudioFilterHelper.h"

#include <wmsdkidl.h>

#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);

// sets fps, size, (etc.) maybe, or maybe just saves it away for later use...
HRESULT STDMETHODCALLTYPE CCamStudioPin::SetFormat(AM_MEDIA_TYPE *pmt)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	// I *think* it can go back and forth, then.  You can call GetStreamCaps to enumerate, then call
	// SetFormat, then later calls to GetMediaType/GetStreamCaps/EnumMediatypes will all "have" to just give this one
	// though theoretically they could also call EnumMediaTypes, then Set MediaType, and not call SetFormat
	// does flash call both? what order for flash/ffmpeg/vlc calling both?
	// LODO update msdn

	// "they" [can] call this...see msdn for SetFormat

	// NULL means reset to default type...
	if(pmt != NULL)
	{
		if(pmt->formattype != FORMAT_VideoInfo)  // FORMAT_VideoInfo == {CLSID_KsDataTypeHandlerVideo} 
			return E_FAIL;
	
		// LODO I should do more here...http://msdn.microsoft.com/en-us/library/dd319788.aspx I guess [meh]
		// LODO should fail if we're already streaming... [?]

		if(CheckMediaType((CMediaType *) pmt) != S_OK) {
			return E_FAIL; // just in case :P [FME...]
		}
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pmt->pbFormat;

		//if( pvi->bmiHeader.biWidth != m_CaptureWindow.Width() || //m_nCaptureWidth || 
			//pvi->bmiHeader.biHeight != m_CaptureWindow.Height())//m_nCaptureHeight)

		/*if( pvi->bmiHeader.biWidth != 1920 ||       // del by dongmingyi
			pvi->bmiHeader.biHeight != 1080 )
		{
			return E_INVALIDARG;
		}*/

	    

		int nCount =  sizeof(video_desc)/sizeof(MediaDescriptor);
		int m = 0;
		if(pvi->bmiHeader.biWidth == m_nSrcWidth &&  pvi->bmiHeader.biHeight == m_nSrcHeight)
		{
			 m_nCaptureWidth = pvi->bmiHeader.biWidth;  // add by dongmingyi
		     m_nCaptureHeight = pvi->bmiHeader.biHeight;
		}
		else
		{  
				for ( m = 0; m < nCount; m++)
				{ 
					if(pvi->bmiHeader.biWidth == video_desc[m].width && pvi->bmiHeader.biHeight == video_desc[m].height)
					{
						 m_nCaptureWidth = pvi->bmiHeader.biWidth;  // add by dongmingyi
		                 m_nCaptureHeight = pvi->bmiHeader.biHeight;
						break;
					}
				} 
		} 

		if(m == nCount)
			return E_INVALIDARG;
			 

		// ignore other things like cropping requests for now...

		// now save it away...for being able to re-offer it later. We could use Set MediaType but we're just being lazy and re-using m_mt for many things I guess
		m_mt = *pmt;

	}
	/**/
	IPin* pin;
	ConnectedTo(&pin);
	if(pin)
	{
		IFilterGraph *pGraph = m_pParent->GetGraph();
		HRESULT res = pGraph->Reconnect(this);
		if(res != S_OK) // LODO check first, and then just re-use the old one?
			return res; // else return early...not really sure how to handle this...since we already set m_mt...but it's a pretty rare case I think...
		// plus ours is a weird case...
	} 
	else 
	{
		// graph hasn't been built yet...
		// so we're ok with "whatever" format they pass us, we're just in the setup phase...
	}
	
	// success of some type

	/*
	if(pmt == NULL) {		
		m_bFormatAlreadySet = false;
	} else {
		m_bFormatAlreadySet = true;
	}
	*/
	return S_OK;
}

// get's the current format...I guess...
// or get default if they haven't called SetFormat yet...
// LODO the default, which probably we don't do yet...unless they've already called GetStreamCaps then it'll be the last index they used LOL.
HRESULT STDMETHODCALLTYPE CCamStudioPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	/*  */
	if( !m_mt.IsValid())
	{  
		VIDEOINFO *pvi = (VIDEOINFO *) m_mt.AllocFormatBuffer(sizeof(VIDEOINFO));
		if(NULL == pvi)
			return(E_OUTOFMEMORY);

		// Initialize the VideoInfo structure before configuring its members
		ZeroMemory(pvi, sizeof(VIDEOINFO)); 
		//  modify by dongmingyi
		pvi->bmiHeader.biCompression = BI_RGB;
		pvi->bmiHeader.biBitCount    = 32;//24;
		pvi->bmiHeader.biWidth      =  m_nSrcWidth; 
		pvi->bmiHeader.biHeight     =  m_nSrcHeight;  
		pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER); 
		pvi->bmiHeader.biPlanes     = 1;

		// pvi->bmiHeader.biClrUsed     = iPALETTE_COLORS;

		pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader); // calculates the size for us, after we gave it the width and everything else we already chucked into it
		
		m_mt.SetSampleSize(pvi->bmiHeader.biSizeImage); // use the above size 
		pvi->bmiHeader.biClrImportant = 0;
		pvi->AvgTimePerFrame = m_rtFrameLength; // from our config or default


		  
		 
		//if( (m_nSrcWidth == pvi->bmiHeader.biWidth) && (m_nSrcHeight == pvi->bmiHeader.biHeight))   // add by dongmingyi   如果屏幕的采集尺寸不需要resize，则帧率为10.
		//{
		//	pvi->AvgTimePerFrame  =  UNITS / 10;
		//}  
		SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
		SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

		m_mt.SetType(&MEDIATYPE_Video);
		m_mt.SetFormatType(&FORMAT_VideoInfo);
		m_mt.SetTemporalCompression(FALSE);

		// Work out the GUID for the subtype from the header info.
		if(*m_mt.Subtype() == GUID_NULL) 
		{
			const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
			m_mt.SetSubtype(&SubTypeGUID);
		}

		*ppmt = CreateMediaType(&m_mt); // windows internal method, also does copy
	}
	else if(m_bIsChange)
	{
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.pbFormat;
         pvi->bmiHeader.biWidth  = m_nCaptureWidth;
	     pvi->bmiHeader.biHeight  = m_nCaptureHeight;
	     pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader); // calculates the size for us, after we gave it the width and everything else we already chucked into it
	     pvi->AvgTimePerFrame = m_rtFrameLength;  
		
		 m_mt.SetSampleSize(pvi->bmiHeader.biSizeImage);

		 *ppmt = CreateMediaType(&m_mt);  
		 m_bIsChange = false;
	}
	else   
	{
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mt.pbFormat;
	    *ppmt = CreateMediaType(&m_mt); 
	}
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CCamStudioPin::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	int count =  sizeof(video_desc)/sizeof(MediaDescriptor);   // modify by dongmingyi

	if(m_bIncludeDefaultSize)
	{
		*piCount =  count-1;
	}
	else{ 
	    *piCount = count;
	}
	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS); // VIDEO_STREAM_CONFIG_CAPS is an MS struct
	return S_OK;
}

// returns the "range" of fps, etc. for this index
HRESULT STDMETHODCALLTYPE CCamStudioPin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	HRESULT hr = GetMediaType(iIndex, &m_mt); // ensure setup/re-use m_mt ...
	// some are indeed shared, apparently.
	if(FAILED(hr))
	{
		return hr;
	}

	*pmt = CreateMediaType(&m_mt); // a windows lib method, also does a copy for us
	if (*pmt == NULL) return E_OUTOFMEMORY;

	DECLARE_PTR(VIDEO_STREAM_CONFIG_CAPS, pvscc, pSCC);
	
	/*
	  most of these are listed as deprecated by msdn... yet some still used, apparently. odd.
	*/

 	VIDEOINFOHEADER* pInfoHead = (VIDEOINFOHEADER*)(*pmt)->pbFormat;  // add by dongmingyi

	pvscc->VideoStandard = AnalogVideo_None;
	pvscc->InputSize.cx =  pInfoHead->bmiHeader.biWidth ;//*/m_nCaptureWidth;
	pvscc->InputSize.cy =  pInfoHead->bmiHeader.biHeight; //*/m_nCaptureHeight;

	// most of these values are fakes..
	pvscc->MinCroppingSize.cx =  pInfoHead->bmiHeader.biWidth ;//*/m_nCaptureWidth;
	pvscc->MinCroppingSize.cy =  pInfoHead->bmiHeader.biHeight;//*/m_nCaptureHeight;

	pvscc->MaxCroppingSize.cx =  pInfoHead->bmiHeader.biWidth ;//*/m_nCaptureWidth;
	pvscc->MaxCroppingSize.cy =  pInfoHead->bmiHeader.biHeight;//*/m_nCaptureHeight;

	pvscc->CropGranularityX = 1;
	pvscc->CropGranularityY = 1;
	pvscc->CropAlignX = 1;
	pvscc->CropAlignY = 1;

	pvscc->MinOutputSize.cx = pInfoHead->bmiHeader.biWidth ;//m_nCaptureWidth;//1;
	pvscc->MinOutputSize.cy = pInfoHead->bmiHeader.biHeight;//m_nCaptureHeight;//1;
	pvscc->MaxOutputSize.cx = pInfoHead->bmiHeader.biWidth ;///*m_CaptureWindow.Width();//*/m_nCaptureWidth;
	pvscc->MaxOutputSize.cy = pInfoHead->bmiHeader.biHeight;///*m_CaptureWindow.Height();//*/m_nCaptureHeight;
	pvscc->OutputGranularityX = 0;//1;
	pvscc->OutputGranularityY = 0;//1;

	pvscc->StretchTapsX = 1; // We do 1 tap. I guess...
	pvscc->StretchTapsY = 1;
	pvscc->ShrinkTapsX = 1;
	pvscc->ShrinkTapsY = 1;

	pvscc->MinFrameInterval = m_rtFrameLength; // the larger default is actually the MinFrameInterval, not the max
	pvscc->MaxFrameInterval = 500000000; // 0.02 fps :) [though it could go lower, really...]

	pvscc->MinBitsPerSecond = (LONG) 1*1*8*GetFps(); // if in 8 bit mode 1x1. I guess.
	//pvscc->MaxBitsPerSecond = (LONG) /*m_nCaptureWidth *m_nCaptureHeight*/m_CaptureWindow.Width() * m_CaptureWindow.Height() *32*GetFps() + 44; // + 44 header size? + the palette?
	//pvscc->MaxBitsPerSecond = (LONG) m_nCaptureWidth *m_nCaptureHeight *32*GetFps() + 44;

	pvscc->MaxBitsPerSecond = (LONG) pInfoHead->bmiHeader.biWidth * pInfoHead->bmiHeader.biHeight *32*GetFps() + 44;  // modify by dongmingyi
	return hr;
}