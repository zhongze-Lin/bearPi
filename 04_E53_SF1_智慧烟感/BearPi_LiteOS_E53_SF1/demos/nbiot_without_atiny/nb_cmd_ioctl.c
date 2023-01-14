/*----------------------------------------------------------------------------
 * Copyright (c) <2016-2018>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "nb_cmd_ioctl.h"
#include "sys_init.h"

msg_sys_type bc95_net_data;

#define cn_resp_buf_len    128   //may be bigger enough
char replybuf[cn_resp_buf_len] = {0};

int32_t nb_cmd_data_ioctl(void* arg, int8_t  * buf, int32_t len)
{

	int readlen = 0;
	char tmpbuf[1064] = {0};
	char recvmid[4] = {0};
	UINT32 msglen = 0;
    if (NULL == buf || len <= 0)
    {
        AT_LOG("param invailed!");
        return -1;
    }
	sscanf((char *)buf,"\r\n+NNMI:%d,%s\r\n",&readlen,tmpbuf);

	if(tmpbuf[1] == '9')   //messageid for Smoke_Control_Beep
	{
		memcpy(recvmid,tmpbuf + 2, 4);
		memset(bc95_net_data.net_nmgr, 0, 30);
		if (readlen > 0)
		{  
			HexStrToStr((const unsigned char *)tmpbuf + 6, (unsigned char *)bc95_net_data.net_nmgr, (readlen - 3) *2 );
		}
		AT_LOG("cmd is:%s\n", bc95_net_data.net_nmgr);
		if(strcmp(bc95_net_data.net_nmgr, "ON") == 0) 
		{	
			E53SF1_Beep(ON);  				

			char* respmsgid = "0a"; //messageid for Smoke_Control_Beep reply
			memcpy(s_resp_buf, respmsgid, 2);
			memcpy(s_resp_buf + 2, recvmid, 4);
			char* reply = "0001";
			memcpy(s_resp_buf + 6, reply, 4);
			LOS_SemPost(reply_sem);				
		}
		if (strcmp(bc95_net_data.net_nmgr, "OFF") == 0) 
		{	
        	E53SF1_Beep(OFF); 

			char* respmsgid = "0a";	//messageid for Smoke_Control_Beep reply
			memcpy(s_resp_buf, respmsgid, 2);
			memcpy(s_resp_buf + 2, recvmid, 4);
			char* reply = "0000";
			memcpy(s_resp_buf + 6, reply, 4);
			LOS_SemPost(reply_sem);
		}
	}
	
/*******************************END**********************************************/
		return 0;
}

int32_t OC_cmd_match(const char *buf, char* featurestr,int len)
{

    if(strstr(buf,featurestr) != NULL)
        return 0;
    else
        return -1;
}

