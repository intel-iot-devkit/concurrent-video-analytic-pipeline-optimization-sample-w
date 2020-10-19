/******************************************************************************\
Copyright (c) 2005-2018, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/

#include "mfx_samples_config.h"
#include "vm/strings_defs.h"
#include "time_statistics.h"
#include "sample_defs.h"
#include "sample_utils.h"
#include "mfxcommon.h"

#include "file_and_rtsp_bitstream_reader.h"


FileAndRTSPBitstreamReader::FileAndRTSPBitstreamReader()
	:CSmplBitstreamReader()
{
#ifdef RTSP_SUPPORT
	m_bIsRTSP = false;
	m_context = nullptr;
	m_rtsp_thread = nullptr;
	m_video_stream_index = 0;
	m_packets = nullptr;
	m_rtsp_status = RTSP_NOT_CONNECT;
	m_rtsp_queue_size = 100;
	m_rtsp_file = nullptr;
#endif
}

FileAndRTSPBitstreamReader::~FileAndRTSPBitstreamReader()
{
#ifdef RTSP_SUPPORT
	if (m_bIsRTSP)
	{
		if (m_packets)
		{
			delete m_packets;
			m_packets = nullptr;
		}

		if (m_rtsp_thread)
		{
			delete m_rtsp_thread;
			m_rtsp_thread = nullptr;
		}
	}
#endif
}

#ifdef RTSP_SUPPORT
void FileAndRTSPBitstreamReader::ClearRTSPQueue()
{
	if (m_packets)
	{
		while (m_packets->size() > 0)
		{
			PacketData *packet = m_packets->front();
			m_packets->pop_front();
			free(packet->data);
			packet->data = nullptr;
			delete packet;
		}
		delete m_packets;
		m_packets = nullptr;

	}
	return;
}
#endif

void FileAndRTSPBitstreamReader::Close()
{
	CSmplBitstreamReader::Close();

#ifdef RTSP_SUPPORT
	if (m_bIsRTSP)
	{
		m_rtsp_status = RTSP_STOP;
		m_rtsp_thread->join();
		ClearRTSPQueue();
		avformat_close_input(&m_context);
		avformat_free_context(m_context);
		m_bIsRTSP = false;
		if (m_rtsp_file)
		{
			fclose(m_rtsp_file);
			m_rtsp_file = nullptr;
		}
	}
#endif
}

void FileAndRTSPBitstreamReader::Reset()
{
	if (!m_bInited)
		return;
#ifdef RTSP_SUPPORT
	if (!m_bIsRTSP) {
#endif
		fseek(m_fSource, 0, SEEK_SET);
#ifdef RTSP_SUPPORT
	}
#endif
}

mfxStatus FileAndRTSPBitstreamReader::Init(const msdk_char *strFileName)
{
	mfxStatus sts = MFX_ERR_NONE;
	MSDK_CHECK_POINTER(strFileName, MFX_ERR_NULL_PTR);

	if (!msdk_strlen(strFileName))
		return MFX_ERR_NONE;

#ifdef RTSP_SUPPORT
	/* rtsp streaming */
	msdk_string rtsp_addr_header= MSDK_STRING("rtsp://");
	msdk_string input_str = msdk_string(strFileName);
	int compare_value{
		input_str.compare(0, 7, rtsp_addr_header)
	};
#endif

#ifdef RTSP_SUPPORT
	if (!compare_value)
	{
		AVDictionary *avdic = NULL;
		char option_key[] = "max_delay";
		char option_value[] = "50000000";
		char url[50];
		//strncpy(url, (char*)strFileName, 34);
		//url[34] = '\0';
		int len = WideCharToMultiByte(CP_ACP, 0, strFileName, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, strFileName, -1, url, len, NULL, NULL);
		
		av_dict_set(&avdic, option_key, option_value, 0);
		//av_dict_set(&avdic, "rtsp_transport", "tcp", 0);
		m_bIsRTSP = true;

		av_register_all();
		avformat_network_init();

		m_context = avformat_alloc_context();
		
		if (avformat_open_input(&m_context, url, NULL, &avdic) != 0) {
			msdk_printf(MSDK_STRING("Failed to RTSP address: %s\n"), input_str.c_str());
			sts = MFX_ERR_NOT_FOUND;
			goto exit;
		}

		if (avformat_find_stream_info(m_context, NULL) < 0) {
			msdk_printf(MSDK_STRING("Failed to find RTSP stream info: %s\n"), input_str.c_str());
			sts = MFX_ERR_NOT_FOUND;
			goto exit;
		}

		//search video stream
		for (unsigned int i = 0; i < m_context->nb_streams; i++) {
			if (m_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
				m_video_stream_index = i;
		}

		m_packets = new SampleQueue<PacketData *>();
		m_rtsp_status = RTSP_CONNECTED;
	}
	else {
#endif
		//open file to read input stream
		MSDK_FOPEN(m_fSource, strFileName, MSDK_STRING("rb"));
	        if (!m_fSource)
	        {
	            msdk_printf(MSDK_STRING("ERROR: Failed to open video file %s, please verify if the video file path is correct\n"), input_str.c_str());
	            return MFX_ERR_NULL_PTR;
	        }
#ifdef RTSP_SUPPORT
	}
#endif

	m_bInited = true;
	return MFX_ERR_NONE;

#ifdef RTSP_SUPPORT
	exit :
		 avformat_free_context(m_context);
		 return sts;
#endif
}

mfxStatus FileAndRTSPBitstreamReader::ReadNextFrame(mfxBitstream *pBS)
{
	if (!m_bInited)
		return MFX_ERR_NOT_INITIALIZED;

	MSDK_CHECK_POINTER(pBS, MFX_ERR_NULL_PTR);

	mfxU32 nBytesRead = 0;
	int nLeftBufferSize = pBS->MaxLength - pBS->DataLength;

	if (pBS->DataLength > 0) {
		memmove(pBS->Data, pBS->Data + pBS->DataOffset, pBS->DataLength);
	}
	pBS->DataOffset = 0;

#ifdef RTSP_SUPPORT
	int ret = 0;

	if (m_bIsRTSP) {
		//Status should be RTSP_CONNECT or RTSP_PLAY
		if (GetRTSPStatus() == RTSP_STOP)
		{
			return MFX_ERR_NONE;
		}

		if (!m_packets)
		{
			return MFX_ERR_NULL_PTR;
		}

		if (GetRTSPStatus() == RTSP_CONNECTED)
		{
			m_rtsp_status = RTSP_PLAY;
			StartRTSPThread();
		}
		int retry_count = 0;
		while (retry_count++ < RTSP_RETRY_MAX)
		{
			PacketData *packet = m_packets->front();
			m_packets->pop_front();

			if (packet->size > 0) {

				if (nLeftBufferSize >= (int)packet->size) {
					memmove(pBS->Data + pBS->DataLength, packet->data, packet->size);
					nBytesRead += packet->size;

				}
				else
				{
					memmove(pBS->Data + pBS->DataLength, packet->data, nLeftBufferSize);
					nBytesRead += nLeftBufferSize;
					msdk_printf(MSDK_STRING("Bitstream buffer overflow! RTSP packet size: %zu, buffer size: %d\n"),
						packet->size, nLeftBufferSize);
				}
				free(packet->data);
				packet->data = nullptr;
				delete packet;
				break;
			}
		}

		if (retry_count == RTSP_RETRY_MAX)
		{
			msdk_printf(MSDK_STRING("Reach max RTSP retry count %d. Current queue size %d\n"), retry_count, m_packets->size());
			return  MFX_ERR_UNKNOWN;
		}
	}
	else
	{

#endif
	start_again:
		nBytesRead = (mfxU32)fread(pBS->Data + pBS->DataLength, 1, nLeftBufferSize, m_fSource);
		if (0 == nBytesRead)
		{
#if LOOP_INPUT
			fseek(m_fSource, 0, SEEK_SET);
			goto start_again;
#else
			return MFX_ERR_MORE_DATA;
#endif
		}

#ifdef RTSP_SUPPORT
	}
#endif

	pBS->DataLength += nBytesRead;

	return MFX_ERR_NONE;
}


#ifdef RTSP_SUPPORT
void FileAndRTSPBitstreamReader::RtspPacketReader(FileAndRTSPBitstreamReader  *ctx)
{
	int ret = 0;
	unsigned int packet_count = 0;
	unsigned int error_count = 0;
	unsigned int drop_count = 0;
	AVPacket packet = { 0 };
	int video_idx = ctx->m_video_stream_index;

	while (ret == 0)
	{
		RTSP_STATUS status = ctx->GetRTSPStatus();
		if (status != RTSP_PLAY)
			break;

		ret = av_read_frame(ctx->m_context, &packet);
		if (ret)
		{
			error_count++;
			msdk_printf(MSDK_STRING("error: av_read_frame failed %d. error count %d\n"), ret, error_count);
			if (error_count > RTSP_ERROR_MAX_COUNT)
			{
				break;
			}
			else
			{
				ret = 0;
				continue;
			}

		}

		if (packet.stream_index != video_idx)
		{
			av_free_packet(&packet);
			continue;
		}
		packet_count++;

		if (ctx->m_packets->size() > RTSP_QUEUE_MAX_SIZE)
		{
			drop_count++;
			if (drop_count % 1000 == 0)
			{
				msdk_printf(MSDK_STRING("Warning: drop rtsp packet %d\n"), drop_count);
			}
			av_free_packet(&packet);
			continue;
		}

		PacketData *pData = new PacketData;
		pData->size = packet.size;
		pData->data = malloc(pData->size);
		if (pData->data && (packet.size > 0))
		{
			memcpy(pData->data, packet.data, pData->size);
			ctx->m_packets->push_back(pData);

			/* Save the data to local file */
			if (ctx->m_rtsp_file)
			{
				fwrite(packet.data, 1, packet.size, ctx->m_rtsp_file);
			}

			av_free_packet(&packet);
		}
		else
		{
			msdk_printf(MSDK_STRING("ERROR: FileAndRTSPBitstreamReader::RtspPacketReader Out of memory!\n"));
			av_free_packet(&packet);
			if (pData->data)
				free(pData->data);

			delete pData;
			break;
		}
	}
	return;
}

void FileAndRTSPBitstreamReader::StartRTSPThread()
{
	av_read_play(m_context);
	m_rtsp_thread = new std::thread(RtspPacketReader, this);
}

void FileAndRTSPBitstreamReader::CreateRtspDumpFile(const char * rtsp_file_name)
{
	m_rtsp_file = fopen((char *)rtsp_file_name, "wb");
	if (!m_rtsp_file)
	{
		msdk_printf(MSDK_STRING("ERROR: FileAndRTSPBitstreamReader::CreateRtspDumpFile with path %s failed!\n"), m_rtsp_file);
	}
	return;
}

#endif
