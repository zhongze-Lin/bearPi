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

#include "agent_tiny_demo.h"
#include "E53_SF1.h"
#include "lcd.h"

#if defined WITH_AT_FRAMEWORK
#include "at_frame/at_api.h"
#endif


//#define DEFAULT_SERVER_IP "180.101.147.115" /*dianxin*/

#define DEFAULT_SERVER_IP "119.3.250.80" /*local ipv4*/


#define LWM2M_LIFE_TIME     50000

char *g_endpoint_name = "86772503599";
#ifdef WITH_DTLS

char *g_endpoint_name_s = "2019032219";
char *g_endpoint_name_iots = "2019032219";
char *g_endpoint_name_bs = "2019032219";
unsigned char g_psk_iot_value[] = {0x79,0x1e,0xb9,0x37,0x14,0x03,0x10,0x50,0x70,0xbb,0x8f,0xee,0xfc,0xa3,0xb1,0x97}; //0x33 -> 0x32
unsigned char g_psk_bs_value[] = {0x79,0x1e,0xb9,0x37,0x14,0x03,0x10,0x50,0x70,0xbb,0x8f,0xee,0xfc,0xa3,0xb1,0x97};
//unsigned char g_psk_value[16] = {0x58,0xea,0xfd,0xab,0x2f,0x38,0x4d,0x39,0x80,0x69,0x4d,0x1c,0xda,0x69,0xb0,0x43};


#endif

static void *g_phandle = NULL;
static atiny_device_info_t g_device_info;
static atiny_param_t g_atiny_params;

#define cn_buf_len    512   //may be bigger enough
static char t_report_buf[cn_buf_len];
static char s_report_buf[cn_buf_len];
static char r_report_buf[cn_buf_len];

static void HexStrToStr(char *source, char *dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;
    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);
        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;
        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;
        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return ;
}

void ack_callback(atiny_report_type_e type, int cookie, data_send_status_e status)
{
    ATINY_LOG(LOG_DEBUG, "type:%d cookie:%d status:%d\n", type, cookie, status);
}

VOID app_data_report_collection(VOID)
{
	UINT32 uwRet = LOS_OK;
	
	Init_E53_SF1();	
	while (1)
	{
        E53_SF1_Read_Data();
        printf("\r\n******************************Smoke Value is  %d\r\n", E53_SF1_Data.Smoke_Value);
				sprintf(t_report_buf + 2, "%04X", E53_SF1_Data.Smoke_Value);
        
        uwRet=LOS_TaskDelay(2000);
        if(uwRet !=LOS_OK)
        return;

	}
}
UINT32 creat_collection_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;
    UINT32 TskHandle;
    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "app_data_report_collection";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)app_data_report_collection;
    task_init_param.uwStackSize = 0x800;

    uwRet = LOS_TaskCreate(&TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
            return uwRet;
    }
    return uwRet;
}

/*lint -e550*/
void app_data_report(void)
{
    data_report_t report_data;
    int ret = 0;
    int cnt = 0;
    UINT32 msgid;
    msgid = 8;
    sprintf(t_report_buf, "%02d", msgid);
	
    report_data.buf = (uint8_t *)s_report_buf;
    report_data.callback = ack_callback;
    report_data.cookie = 0;
    report_data.len = sizeof(s_report_buf);
    report_data.type = APP_DATA;
    (void)ret;
    while(1)
    {
        report_data.cookie = cnt;
        cnt++;
        HexStrToStr(t_report_buf,s_report_buf,strlen(t_report_buf));
        ret = atiny_data_report(g_phandle, &report_data);
        ATINY_LOG(LOG_DEBUG, "data report ret: %d\n", ret);
        ret = atiny_data_change(g_phandle, DEVICE_MEMORY_FREE);
        ATINY_LOG(LOG_DEBUG, "data change ret: %d\n", ret);
        (void)LOS_TaskDelay(500 * 8);
    }
}
/*lint +e550*/

VOID reply_report_task(VOID)
{
    data_report_t reply_data;
    int ret = 0;
    int cnt = 0;

    reply_data.buf = (uint8_t *)r_report_buf;
    reply_data.callback = ack_callback;
    reply_data.cookie = 0;
    reply_data.len = sizeof(r_report_buf);
    reply_data.type = APP_DATA;
    while (1)
    {
        LOS_SemPend(reply_sem, LOS_WAIT_FOREVER);
        printf("This is reply_report_task\n");    
        HexStrToStr(s_resp_buf,r_report_buf,strlen(s_resp_buf));
        if(atiny_data_report(g_phandle, &reply_data)>=0)		//上报数据			
        {
                printf("ocean_send_reply OK!\n");                                                  
        }
        else                                                                                  
        {
                printf("ocean_send_reply Fail!\n"); 	
        }
        memset(s_resp_buf, 0, sizeof(s_resp_buf));
    }
}

UINT32 creat_reply_report_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;
    UINT32 TskHandle;

    task_init_param.usTaskPrio = 1;
    task_init_param.pcName = "reply_report_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)reply_report_task;
    task_init_param.uwStackSize = 0x800;
    uwRet = LOS_TaskCreate(&TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

UINT32 creat_report_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;
    UINT32 TskHandle;

    task_init_param.usTaskPrio = 1;
    task_init_param.pcName = "app_data_report";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)app_data_report;
    task_init_param.uwStackSize = 0x800;

    uwRet = LOS_TaskCreate(&TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;

}


void agent_tiny_entry(void)
{
    UINT32 uwRet = LOS_OK;
    atiny_param_t *atiny_params;
    atiny_security_param_t  *iot_security_param = NULL;
    atiny_security_param_t  *bs_security_param = NULL;

    atiny_device_info_t *device_info = &g_device_info;


#ifdef WITH_DTLS
    device_info->endpoint_name = g_endpoint_name_s;
#else
    device_info->endpoint_name = g_endpoint_name;
#endif
#ifdef CONFIG_FEATURE_FOTA
    device_info->manufacturer = "Lwm2mFota";
    device_info->dev_type = "Lwm2mFota";
#else
    device_info->manufacturer = "Agent_Tiny";
#endif
    atiny_params = &g_atiny_params;
    atiny_params->server_params.binding = "UQ";
    //atiny_params->server_params.life_time = LWM2M_LIFE_TIME;
    atiny_params->server_params.life_time = 20;
    atiny_params->server_params.storing_cnt = 3;

    atiny_params->server_params.bootstrap_mode = BOOTSTRAP_FACTORY;
    atiny_params->server_params.hold_off_time = 10;

    //pay attention: index 0 for iot server, index 1 for bootstrap server.
    iot_security_param = &(atiny_params->security_params[0]);
    bs_security_param = &(atiny_params->security_params[1]);


    iot_security_param->server_ip = DEFAULT_SERVER_IP;
    bs_security_param->server_ip  = DEFAULT_SERVER_IP;


#ifdef WITH_DTLS
    iot_security_param->server_port = "5684";
    bs_security_param->server_port = "5684";

    iot_security_param->psk_Id = g_endpoint_name_iots;
    iot_security_param->psk = (char *)g_psk_iot_value;
    iot_security_param->psk_len = sizeof(g_psk_iot_value);

    bs_security_param->psk_Id = g_endpoint_name_bs;
    bs_security_param->psk = (char *)g_psk_bs_value;
    bs_security_param->psk_len = sizeof(g_psk_bs_value);
#else
    iot_security_param->server_port = "5683";
    bs_security_param->server_port = "5683";

    iot_security_param->psk_Id = NULL;
    iot_security_param->psk = NULL;
    iot_security_param->psk_len = 0;

    bs_security_param->psk_Id = NULL;
    bs_security_param->psk = NULL;
    bs_security_param->psk_len = 0;
#endif
    if(ATINY_OK != atiny_init(atiny_params, &g_phandle))
    {
        return;
    }
    LCD_Clear(WHITE);		   
    POINT_COLOR = RED;	
    LCD_ShowString(40, 10, 200, 16, 24, "IoTCluB BearPi");
    LCD_ShowString(50, 50, 200, 16, 24, "E53_IA1_Demo");
    LCD_ShowString(10, 100, 200, 16, 16, "NCDP_IP:");
    LCD_ShowString(80, 100, 200, 16, 16, DEFAULT_SERVER_IP);
    LCD_ShowString(10, 150, 200, 16, 16, "NCDP_PORT:");
    #ifdef WITH_DTLS
    LCD_ShowString(100, 150, 200, 16, 16, "5684");
    #else
    LCD_ShowString(100, 150, 200, 16, 16, "5683");
    #endif	
    uwRet = creat_collection_task();
    if (uwRet != LOS_OK)
    {
        return ;
    }
		
    uwRet = creat_report_task();
    if(LOS_OK != uwRet)
    {
        return;
    }

		uwRet = LOS_SemCreate(0,&reply_sem);
		if (uwRet != LOS_OK)
    {
        return ;
    }
		
		uwRet = creat_reply_report_task();
    if (uwRet != LOS_OK)
    {
        return ;
    }
		
    (void)atiny_bind(device_info, g_phandle);
}





