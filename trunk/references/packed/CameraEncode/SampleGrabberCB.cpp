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

	////��ʼ���룬Ҳ�ǿ�ʼ¼��
	//if (m_bBeginEncode)
	//{
	//	//ÿһ֡��С
	//	ULONG nYUVLen = lWidth * lHeight + (lWidth * lHeight)/2;
	//	BYTE * yuvByte = new BYTE[nYUVLen];
	//	//�Ȱ�RGB24תΪYUV420
	//	RGB2YUV(pBuffer, lWidth, lHeight, yuvByte, &nYUVLen);

	//	int csp = X264_CSP_I420;
	//	int width = lWidth;
	//	int height = lHeight;
	//	int y_size = width * height;

	//	//�տ�ʼ��Ҫ��ʼ��һЩ����
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

	//		//��ʼ�����ǶԲ���ȷ�Ĳ��������޸�,���Ը��ṹ�������cabac����,Ԥ�����Ҫ�Ĳ������г�ʼ��
	//		//x264_param_default(m_pParam);

	//		//����б����ӳ٣������������þ��ܼ�ʱ����
	//		//x264_param_default_preset(m_pParam, "fast", "zerolatency"); 

	//		m_pParam->i_width = width;
	//		m_pParam->i_height = height;
	//		m_pParam->i_csp = X264_CSP_I420;  

	//		//����Profile��������5�ּ��𣨱��������������񣩣�����Խ�ߣ�������Խ�ߣ��ķ���ԴԽ��
	//		//x264_param_apply_profile(m_pParam, x264_profile_names[5]);

	//		//x264_picture_t�洢ѹ������ǰ����������
	//		m_pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	//		m_pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));

	//		//x264_picture_init(m_pPic_out);

	//		//Ϊͼ��ṹ��x264_picture_t�����ڴ�
	//		//x264_picture_alloc(m_pPic_in, csp, m_pParam->i_width, m_pParam->i_height);

	//		//�򿪱�����
	//		//m_pHandle = x264_encoder_open(m_pParam);

	//	}

	//	if (m_pPic_in == NULL || m_pPic_out == NULL || m_pHandle == NULL || m_pParam == NULL)
	//	{
	//		return 2;
	//	}

	//	int iNal = 0;

	//	//x264_nal_t�洢ѹ����������������
	//	x264_nal_t* pNals = NULL;

	//	//ע��д����ʼλ�úʹ�С��ǰy_size��Y�����ݣ�Ȼ��y_size/4��U�����ݣ����y_size/4��V������
	//	memcpy(m_pPic_in->img.plane[0], yuvByte, y_size);						//��дY
	//	memcpy(m_pPic_in->img.plane[1], yuvByte + y_size, y_size/4);			//��дU
	//	memcpy(m_pPic_in->img.plane[2], yuvByte + y_size + y_size/4, y_size/4); //��дV

	//	m_pPic_in->i_pts = m_nFrameIndex++; //ʱ��

	//	//����һ֡ͼ��pNalsΪ���ص��������ݣ�iNal�Ƿ��ص�pNals�е�NAL��Ԫ����Ŀ
	//	int ret = 0;// x264_encoder_encode(m_pHandle, &pNals, &iNal, m_pPic_in, m_pPic_out);
	//	if (ret < 0)
	//	{
	//		OutputDebugString(_T("\n x264_encoder_encode err"));
	//		return 1;
	//	}

	//	//д��Ŀ���ļ�
	//	for (int j = 0; j < iNal; ++j)
	//	{
	//		fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, m_fp_dst);
	//	}

	//	delete[] yuvByte; //����Ҫ�ͷ�
	//}

	////��������
	//if (m_bEndEncode)
	//{
	//	m_bEndEncode = FALSE;

	//	int iNal = 0;

	//	//x264_nal_t�洢ѹ����������������
	//	x264_nal_t* pNals = NULL;

	//	//flush encoder 
	//	//�ѱ�������ʣ��������������
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

	//	//�ͷ��ڴ�
	//	//x264_picture_clean(m_pPic_in);

	//	//�رձ�����
	//	//x264_encoder_close(m_pHandle);
	//	m_pHandle = NULL;

	//	free(m_pPic_in);
	//	m_pPic_in = NULL;
	//	free(m_pPic_out);
	//	m_pPic_out = NULL;

	//	free(m_pParam);
	//	m_pParam = NULL;

	//	//�ر��ļ�
	//	fclose(m_fp_dst);
	//	m_fp_dst = NULL;
	//	
	//	m_nFrameIndex = 0;
	//}

	return 0;
}

//rgb24תYUV420
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

