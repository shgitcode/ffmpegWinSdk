#ifndef __H_FF_VIDEO_FRAME_TYPE_H_H
#define __H_FF_VIDEO_FRAME_TYPE_H_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <algorithm>
#include <mutex>
#include "kdvtype.h"
#include "FFVidEncoderType.h"

/* log class use mutex */


/* 视频帧数据 */
typedef struct CapEncVideoYUVFrame : public TFFVideoYUVFrame
{
	u32 m_dwUse;       // 表示有数据,采集时如果为0表示结束采集刷新

	CapEncVideoYUVFrame():m_dwUse{0}
	{
		m_dwVidWidth = { 0 };
		m_dwVidHeight = { 0 };
		m_pVidFrameY = { nullptr };
		m_pVidFrameU = { nullptr };
		m_pVidFrameV = { nullptr };
	}

	CapEncVideoYUVFrame(u32 dwWidth, u32 dwHeight):m_dwUse{ 0 }
	{
		m_dwVidWidth  = dwWidth;
		m_dwVidHeight = dwHeight;

		u32 dwSize = m_dwVidWidth * m_dwVidHeight;

		m_pVidFrameY = new u8[dwSize];
		m_pVidFrameU = new u8[dwSize>>2];
		m_pVidFrameV = new u8[dwSize>>2];

		memset(m_pVidFrameY, 0, dwSize);
		memset(m_pVidFrameU, 0, dwSize>>2);
		memset(m_pVidFrameV, 0, dwSize>>2);
	}

	CapEncVideoYUVFrame(const CapEncVideoYUVFrame &tmp)
	{
		u32 dwSize = 0;
	
		m_dwUse = tmp.m_dwUse;
		m_dwVidWidth = tmp.m_dwVidWidth;
		m_dwVidHeight = tmp.m_dwVidHeight;

		dwSize = m_dwVidWidth * m_dwVidHeight;

		m_pVidFrameY = new u8[dwSize];
		m_pVidFrameU = new u8[dwSize >> 2];
		m_pVidFrameV = new u8[dwSize >> 2];

		memcpy(m_pVidFrameY, tmp.m_pVidFrameY, dwSize);
		memcpy(m_pVidFrameU, tmp.m_pVidFrameU, dwSize >> 2);
		memcpy(m_pVidFrameV, tmp.m_pVidFrameV, dwSize >> 2);
	}

	CapEncVideoYUVFrame& operator=(const CapEncVideoYUVFrame &tmp)
	{
		u32 dwSize = 0;

		if (this != &tmp)
		{
			delete[] m_pVidFrameY;
			delete[] m_pVidFrameU;
			delete[] m_pVidFrameV;

			m_dwUse = tmp.m_dwUse;
			m_dwVidWidth = tmp.m_dwVidWidth;
			m_dwVidHeight = tmp.m_dwVidHeight;

			dwSize = m_dwVidWidth * m_dwVidHeight;

			m_pVidFrameY = new u8[dwSize];
			m_pVidFrameU = new u8[dwSize >> 2];
			m_pVidFrameV = new u8[dwSize >> 2];

			memcpy(m_pVidFrameY, tmp.m_pVidFrameY, dwSize);
			memcpy(m_pVidFrameU, tmp.m_pVidFrameU, dwSize >> 2);
			memcpy(m_pVidFrameV, tmp.m_pVidFrameV, dwSize >> 2);
		}
	
		return *this;
	}

	CapEncVideoYUVFrame(CapEncVideoYUVFrame &&tmp)
	{
		u32 dwSize = 0;

		m_dwUse = tmp.m_dwUse;
		m_dwVidWidth = tmp.m_dwVidWidth;
		m_dwVidHeight = tmp.m_dwVidHeight;
		m_pVidFrameY = tmp.m_pVidFrameY;
		m_pVidFrameU = tmp.m_pVidFrameU;
		m_pVidFrameV = tmp.m_pVidFrameV;

		tmp.m_dwUse = 0;
		tmp.m_dwVidWidth = tmp.m_dwVidHeight = 0;
		tmp.m_pVidFrameY = tmp.m_pVidFrameU = tmp.m_pVidFrameV = nullptr;
	}

	CapEncVideoYUVFrame& operator=(CapEncVideoYUVFrame &&tmp)
	{
		u32 dwSize = 0;
		u8 *pTmpY = nullptr;
		u8 *pTmpU = nullptr;
		u8 *pTmpV = nullptr;

		if (this != &tmp)
		{
			delete[] m_pVidFrameY;
			delete[] m_pVidFrameU;
			delete[] m_pVidFrameV;
		}

		m_dwUse = tmp.m_dwUse;
		m_dwVidWidth = tmp.m_dwVidWidth;
		m_dwVidHeight = tmp.m_dwVidHeight;
		m_pVidFrameY = tmp.m_pVidFrameY;
		m_pVidFrameU = tmp.m_pVidFrameU;
		m_pVidFrameV = tmp.m_pVidFrameV;

		tmp.m_dwUse = 0;
		tmp.m_dwVidWidth = tmp.m_dwVidHeight = 0;
		tmp.m_pVidFrameY = tmp.m_pVidFrameU = tmp.m_pVidFrameV = nullptr;

		return *this;
	}

	virtual ~CapEncVideoYUVFrame()
	{	
		m_dwUse = 0;
		m_dwVidWidth = m_dwVidHeight = 0;

		delete[] m_pVidFrameY;
		delete[] m_pVidFrameU;
		delete[] m_pVidFrameV;
		m_pVidFrameY = nullptr;
		m_pVidFrameU = nullptr;
		m_pVidFrameV = nullptr;
	}

}TCapEncVideoYUVFrame;

#endif

