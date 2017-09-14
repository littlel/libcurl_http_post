/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/ 
/* <DESC>
 * simple HTTP POST using the easy interface
 * </DESC>
 */ 
#include <stdio.h>
#include <../curl/curl.h>


#ifdef DEBUG
#pragma comment(lib,"libcurld.lib")
#else
#pragma comment(lib,"libcurl.lib")
#endif

#include <string>
//int main(void)
//{
//  CURL *curl;
//  CURLcode res;
//
//  const char buf[] = "Expect:";
//
//
//  struct curl_httppost *formpost = NULL;
//  struct curl_httppost *lastptr = NULL;
//
//
//  /* In windows, this will init the winsock stuff */ 
//  curl_global_init(CURL_GLOBAL_ALL);
// 
//  struct curl_slist *headerlist = NULL;
//
//  curl_formadd(&formpost, &lastptr,CURLFORM_CONTENTTYPE, "application/json",CURLFORM_END);
//  /* get a curl handle */ 
//  curl = curl_easy_init();
//
//   headerlist = curl_slist_append(headerlist, buf);
//
//  if(curl) {
//    /* First set the URL that is about to receive our POST. This URL can
//       just as well be a https:// URL if that is what should receive the
//       data. */ 
//
//    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.11.152:8080/hb-spk/annexUpload/client/transCallBack");
//
//	//test
//	
//	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
//	
//	/* Now specify the POST data */ 
//	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"fileId:\":\"1111\",\"userId\":\"1111\",\"isSuccess\":\"1\",\"videoPath\":\"11111\",\"imageName\":\"1111\",\"imagePath\":\"2333\"}");
// 
//	
//
//    /* Perform the request, res will get the return code */ 
//    res = curl_easy_perform(curl);
//    /* Check for errors */ 
//    if(res != CURLE_OK)
//      fprintf(stderr, "curl_easy_perform() failed: %s\n",
//              curl_easy_strerror(res));
// 
//	curl_formfree(formpost);
//    /* always cleanup */ 
//    curl_easy_cleanup(curl);
//  }
//  curl_global_cleanup();
//  printf("\n");
//  system("pause");
//  return 0;
//}


char* U2G(const char* utf8)  
{  
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);  
	wchar_t* wstr = new wchar_t[len+1];  
	memset(wstr, 0, len+1);  
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);  
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);  
	char* str = new char[len+1];  
	memset(str, 0, len+1);  
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);  
	if(wstr) delete[] wstr;  
	return str;  
} 


/***************************************************************************** 
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码. 
 * 
 * 参数: 
 *    unic     字符的Unicode编码值 
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针 
 *    outsize  pOutput缓冲的大小 
 * 
 * 返回值: 
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 . 
 * 
 * 注意: 
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求; 
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种; 
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位) 
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小! 
 ****************************************************************************/  
int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,  int outSize)  
{  
   /* assert(pOutput != NULL);  
    assert(outSize >= 6);  */
  
    if ( unic <= 0x0000007F )  
    {  
        // * U-00000000 - U-0000007F:  0xxxxxxx  
        *pOutput     = (unic & 0x7F);  
        return 1;  
    }  
    else if ( unic >= 0x00000080 && unic <= 0x000007FF )  
    {  
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
        *(pOutput+1) = (unic & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;  
        return 2;  
    }  
    else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )  
    {  
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
        *(pOutput+2) = (unic & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;  
        return 3;  
    }  
    else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )  
    {  
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+3) = (unic & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 18) & 0x07) | 0xF0;  
        return 4;  
    }  
    else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )  
    {  
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+4) = (unic & 0x3F) | 0x80;  
        *(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 24) & 0x03) | 0xF8;  
        return 5;  
    }  
    else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )  
    {  
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+5) = (unic & 0x3F) | 0x80;  
        *(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 30) & 0x01) | 0xFC;  
        return 6;  
    }  
  
    return 0;  
}  

size_t HTTP_dataCallback(void *contents, size_t size, size_t nmemb, void *userdata) 
{   
	size_t realsize = size * nmemb;
	char* version = (char*)malloc(realsize+1);
	strcpy(version,(char*)contents);
	if(version != NULL)
	  printf("%s\n",U2G(version));
	else
		printf("NULL\n");
	return realsize;
}  

int main(int argc, char *argv[])
{
	CURL *curl;
	CURLcode res;
	struct curl_slist    *headerlist=NULL;

	curl_global_init(CURL_GLOBAL_ALL);
	char _version[1000];
	curl = curl_easy_init();
	
	if(curl) {
		
		//set http_header
		headerlist = curl_slist_append(headerlist, "Content-Type: application/json"); 
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		//set post_data
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"fileId:\":\"123456\",\"userId\":\"2333\",\"isSuccess\":\"1\",\"videoPath\":\"11111\",\"imageName\":\"1111\",\"imagePath\":\"2333\"}");

		/* 设置接收回调和回复内容变量*/
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTP_dataCallback); 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &_version);  

		//set http utl
		curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.11.152:8080/hb-spk/annexUpload/client/transCallBack");


		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
		curl_easy_strerror(res));

		curl_easy_cleanup(curl);

		//curl_formfree(formpost);
		curl_slist_free_all (headerlist);
	}
	printf("\n");
	system("pause");
	return 0;
}