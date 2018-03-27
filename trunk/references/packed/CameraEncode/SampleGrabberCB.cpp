#include "stdafx.h"
#include "SampleGrabberCB.h"


CSampleGrabberCB::CSampleGrabberCB(void)
{
	m_fp_dst = NULL;
	m_pParam = NULL;
	m_pPic_in = NULL;
	m_pPic_out = NULL;
	m_pHandle = NULL;
	m_bBeginEncode = FALSE; 
	m_bEndEncode = FALSE;
	m_bFirst = TRUE;
	m_nFrameIndex = 0;
	m_sSavePath = _T("");
}


CSampleGrabberCB::~CSampleGrabberCB(void)
{
	if (NULL != m_fp_dst)fclose(m_fp_dst);
	if (NULL != m_pParam)free(m_pParam);
	if (NULL != m_pPic_in)free(m_pPic_in);
	if (NULL != m_pPic_out)free(m_pPic_out);
	if (NULL != m_pHandle)free(m_pHandle);

	m_fp_dst = NULL;
	m_pParam = NULL;
	m_pPic_in = NULL;
	m_pPic_out = NULL;
	m_pHandle = NULL;

}

ULONG STDMETHODCALLTYPE CSampleGrabberCB::AddRef() 
{ 
	return 2; 
}

ULONG STDMETHODCALLTYPE CSampleGrabberCB::Release() 
{ 
	return 1; 
}

HRESULT STDMETHODCALLTYPE CSampleGrabberCB::QueryInterface(REFIID riid, void ** ppv)
{
	if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown )
	{ 
		*ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
		return NOERROR;
	} 
        
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE CSampleGrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
	long BuffLen = pSample->GetActualDataLength();
	BYTE *pBuff;
	CTime time = CTime::GetCurrentTime();
	CString strSuffix = time.Format("%Y%m%d_%H%M%S_yxling.yuv");
	CString strSavePath = _T("");
	strSavePath.Format(_T("%s%s"), m_sSavePath, strSuffix);
	USES_CONVERSION;
	string strFullPath = W2A(strSavePath);
	pSample->GetPointer(&pBuff);
	if (m_bBeginEncode) {
		if(m_fp_dst == NULL)m_fp_dst = fopen(strFullPath.c_str(), "wb+");
		if (m_fp_dst && pSample) {
			fwrite(pBuff, 1, BuffLen, m_fp_dst);
		}
		//if(m_fp_dst != NULL)fclose(m_fp_dst);
		//m_fp_dst = NULL;

	}



	return 0;
}
        
HRESULT STDMETHODCALLTYPE CSampleGrabberCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	CString str;
	str.Format(_T("\n BufferCB--lBufferSize:%ld,lWidth:%d,lHeight:%d"), BufferLen, lWidth, lHeight);
	OutputDebugString(str);

	////开始编码，也是开始录制
	//if (m_bBeginEncode)
	//{
	//	//每一帧大小
	//	ULONG nYUVLen = lWidth * lHeight + (lWidth * lHeight)/2;
	//	BYTE * yuvByte = new BYTE[nYUVLen];
	//	//先把RGB24转为YUV420
	//	RGB2YUV(pBuffer, lWidth, lHeight, yuvByte, &nYUVLen);

	//	int csp = X264_CSP_I420;
	//	int width = lWidth;
	//	int height = lHeight;
	//	int y_size = width * height;

	//	//刚开始打开要初始化一些参数
	//	if (m_bFirst)
	//	{
	//		m_bFirst = FALSE;

	//		CTime time = CTime::GetCurrentTime();
	//		CString szTime = time.Format("%Y%m%d_%H%M%S.h264");
	//		CString strSavePath = _T("");
	//		strSavePath.Format(_T("%s%s"), m_sSavePath, szTime);
	//		USES_CONVERSION;
	//		string strFullPath = W2A(strSavePath);
	//		m_fp_dst = fopen(strFullPath.c_str(), "wb");

	//		m_pParam = (x264_param_t*)malloc(sizeof(x264_param_t));

	//		//初始化，是对不正确的参数进行修改,并对各结构体参数和cabac编码,预测等需要的参数进行初始化
	//		//x264_param_default(m_pParam);

	//		//如果有编码延迟，可以这样设置就能即时编码
	//		//x264_param_default_preset(m_pParam, "fast", "zerolatency"); 

	//		m_pParam->i_width = width;
	//		m_pParam->i_height = height;
	//		m_pParam->i_csp = X264_CSP_I420;  

	//		//设置Profile，这里有5种级别（编码出来的码流规格），级别越高，清晰度越高，耗费资源越大
	//		//x264_param_apply_profile(m_pParam, x264_profile_names[5]);

	//		//x264_picture_t存储压缩编码前的像素数据
	//		m_pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	//		m_pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));

	//		//x264_picture_init(m_pPic_out);

	//		//为图像结构体x264_picture_t分配内存
	//		//x264_picture_alloc(m_pPic_in, csp, m_pParam->i_width, m_pParam->i_height);

	//		//打开编码器
	//		//m_pHandle = x264_encoder_open(m_pParam);

	//	}

	//	if (m_pPic_in == NULL || m_pPic_out == NULL || m_pHandle == NULL || m_pParam == NULL)
	//	{
	//		return 2;
	//	}

	//	int iNal = 0;

	//	//x264_nal_t存储压缩编码后的码流数据
	//	x264_nal_t* pNals = NULL;

	//	//注意写的起始位置和大小，前y_size是Y的数据，然后y_size/4是U的数据，最后y_size/4是V的数据
	//	memcpy(m_pPic_in->img.plane[0], yuvByte, y_size);						//先写Y
	//	memcpy(m_pPic_in->img.plane[1], yuvByte + y_size, y_size/4);			//再写U
	//	memcpy(m_pPic_in->img.plane[2], yuvByte + y_size + y_size/4, y_size/4); //再写V

	//	m_pPic_in->i_pts = m_nFrameIndex++; //时钟

	//	//编码一帧图像，pNals为返回的码流数据，iNal是返回的pNals中的NAL单元的数目
	//	int ret = 0;// x264_encoder_encode(m_pHandle, &pNals, &iNal, m_pPic_in, m_pPic_out);
	//	if (ret < 0)
	//	{
	//		OutputDebugString(_T("\n x264_encoder_encode err"));
	//		return 1;
	//	}

	//	//写入目标文件
	//	for (int j = 0; j < iNal; ++j)
	//	{
	//		fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, m_fp_dst);
	//	}

	//	delete[] yuvByte; //用完要释放
	//}

	////结束编码
	//if (m_bEndEncode)
	//{
	//	m_bEndEncode = FALSE;

	//	int iNal = 0;

	//	//x264_nal_t存储压缩编码后的码流数据
	//	x264_nal_t* pNals = NULL;

	//	//flush encoder 
	//	//把编码器中剩余的码流数据输出
	//	while (1)
	//	{
	//		int ret = 0;// x264_encoder_encode(m_pHandle, &pNals, &iNal, NULL, m_pPic_out);
	//		if (ret == 0)
	//		{
	//			break;
	//		}
	//		printf("Flush 1 frame.\n");
	//		for (int j = 0; j < iNal; ++j)
	//		{
	//			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, m_fp_dst);
	//		}
	//	}

	//	//释放内存
	//	//x264_picture_clean(m_pPic_in);

	//	//关闭编码器
	//	//x264_encoder_close(m_pHandle);
	//	m_pHandle = NULL;

	//	free(m_pPic_in);
	//	m_pPic_in = NULL;
	//	free(m_pPic_out);
	//	m_pPic_out = NULL;

	//	free(m_pParam);
	//	m_pParam = NULL;

	//	//关闭文件
	//	fclose(m_fp_dst);
	//	m_fp_dst = NULL;
	//	
	//	m_nFrameIndex = 0;
	//}

	return 0;
}

//rgb24转YUV420
BOOL CSampleGrabberCB::RGB2YUV(LPBYTE RgbBuf, UINT nWidth, UINT nHeight, LPBYTE yuvBuf, unsigned long *len)  
{
	if (RgbBuf == NULL)
	{
		return FALSE;
	}

    int i, j;  
    unsigned char*bufY, *bufU, *bufV, *bufRGB;
    memset(yuvBuf,0,(unsigned int )*len);  
    bufY = yuvBuf;  
    bufV = yuvBuf + nWidth * nHeight;  
    bufU = bufV + (nWidth * nHeight* 1/4);  
    *len = 0;   
    unsigned char y, u, v, r, g, b;  
    unsigned int ylen = nWidth * nHeight;  
    unsigned int ulen = (nWidth * nHeight)/4;  
    unsigned int vlen = (nWidth * nHeight)/4;   
    for (j = 0; j<nHeight;j++)  
    {  
        bufRGB = RgbBuf + nWidth * (nHeight - 1 - j) * 3 ;  
        for (i = 0;i<nWidth;i++)  
        {  
            int pos = nWidth * i + j;  
            r = *(bufRGB++);  
            g = *(bufRGB++);  
            b = *(bufRGB++);  
            y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;            
            u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;            
            v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;  
            *(bufY++) = max( 0, min(y, 255 ));  
            if (j%2==0&&i%2 ==0)  
            {  
                if (u>255)  
                {  
                    u=255;  
                }  
                if (u<0)  
                {  
                    u = 0;  
                }  
                *(bufU++) =u;  
            }  
            else  
            {  
                if (i%2==0)  
                {  
                    if (v>255)  
                    {  
                        v = 255;  
                    }  
                    if (v<0)  
                    {  
                        v = 0;  
                    }  
                    *(bufV++) =v;  
                }  
            }  
        }  
    }  
    *len = nWidth * nHeight+(nWidth * nHeight)/2; 

    return TRUE;  
}   

