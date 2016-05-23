#ifndef _WIN32

#include <ZionDefines.h>
#include <ZionSTL.h>
#include "mosdk.h"

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#include <net/if.h>
#import <Foundation/Foundation.h>
/*
#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSProcessInfo.h>
*/
#import <CommonCrypto/CommonDigest.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#import <UIKit/UIDevice.h>
#endif

static char g_AppName[1000] = "UNKNOWN_APP";
static char g_UDID[1000] = "UNKNOWN_UDID";
static char g_OSName[1000] = "APPLE";
static char g_ResourcePath[1000] = "";
static const char* get_random_deviceid(char* addr);
static bool save_to_local(const char* addr);
static bool load_from_local(char* addr, int len);

void MOInit(const char* appname)
{
	strcpy(g_AppName, appname);

    MOGetDeviceUDID();
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    char type[100], version[100];
    strcpy(type, [[UIDevice currentDevice].model UTF8String]);
    strcpy(version, [[UIDevice currentDevice].systemVersion UTF8String]);
    if(strstr(type, "iPhone"))
    {
        sprintf(g_OSName, "IOS_iPhone_%s", version);
    }
    else if(strstr(type, "iPod")) {
        sprintf(g_OSName, "IOS_iPod_%s", version);
    }
    else if(strstr(type, "iPad")) {
        sprintf(g_OSName, "IOS_iPad_%s", version);
    }
    else {
        sprintf(g_OSName, "IOS_unknown_%s", version);
    }
#else
	sprintf(g_OSName, "OSX_%s", "aaa");//[[UIDevice currentDevice].systemVersion UTF8String]);
#endif

	NSString *respath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@""];
	strcat(g_ResourcePath, [respath UTF8String]);
}

void MOFini()
{

}

const char* MOGetDeviceUDID()
{
    if(strcmp(g_UDID,"UNKNOWN_UDID") == 0){
        if(!load_from_local(g_UDID, sizeof(g_UDID)))
	    {
            if(get_random_deviceid(g_UDID))
	        {
		        save_to_local(g_UDID);
	        }
	    }
    }
	return g_UDID;
}

const char* MOGetAppName()
{
	return g_AppName;
}

const char* MOGetOSName()
{
	return g_OSName;
}

const char* MoGetResourcePath()
{
	return g_ResourcePath;
}

const char* get_random_deviceid(char* addr)
{
    unsigned char result[16];
    const char *cStr = [[[NSProcessInfo processInfo] globallyUniqueString] UTF8String];
    CC_MD5(cStr, (CC_LONG)strlen(cStr), result);
	sprintf(addr, "rand%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            result[0], result[1], result[2], result[3], 
            result[4], result[5], result[6], result[7],
            result[8], result[9], result[10], result[11],
            result[12], result[13], result[14], result[15]
            );
	return addr;
}

bool save_to_local(const char* addr)
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    char path[1000];
    sprintf(path, "%s/mo_udid.txt", [documentsDirectory UTF8String]);
    FILE* fp = fopen(path, "wt");
    if(fp==NULL) return false;
    fputs(addr, fp);
    fclose(fp);
	return true;
}

bool load_from_local(char* addr, int len)
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    char path[1000];
    sprintf(path, "%s/mo_udid.txt", [documentsDirectory UTF8String]);
    FILE* fp = fopen(path, "rt");
    if(fp==NULL) return false;
    addr = fgets(addr, len, fp);
    fclose(fp);
	return addr!=NULL;
}

#endif
