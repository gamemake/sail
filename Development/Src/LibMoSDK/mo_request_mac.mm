#ifndef _WIN32

#include <ZionDefines.h>
#include <ZionSTL.h>
#include "mosdk.h"
#include "mo_common.h"
#import <Foundation/Foundation.h>
#include <stdio.h>

@class MOHttpRequest;

struct MOREQUEST
{
	Zion::String _result;
	FILE* _file;
	int _length;
	int _content_length;
	MOREQUESTSTATE _state;
	MOHttpRequest* request;
};

@interface MOHttpRequest : NSObject {
	MOREQUEST _struct;
	NSMutableURLRequest* _request;
	NSURLConnection* _connection;
}

- (id)init;
- (void)dealloc;
- (MOREQUEST*)getStruct;
- (bool)do:(NSString *)urlString bodyData:(NSString *)bodyString startPos:(int)startPos;
@end

@implementation MOHttpRequest

- (id)init {
	_struct._result = "";
	_struct._file = NULL;
	_struct._state = MOREQUESTSTATE_PENDING;
	_struct.request = self;
	_struct._length = 0;
	_struct._content_length = 0;
	return self;
}

- (void)dealloc {
	if(_struct._file) fclose(_struct._file);
	[super dealloc];
}

- (MOREQUEST*)getStruct {
	return &_struct;
}

- (bool)do:(NSString *)urlString bodyData:(NSString *)bodyString startPos:(int)startPos {
	NSMutableURLRequest* request = [[NSMutableURLRequest alloc]
								initWithURL: [NSURL URLWithString: urlString]
								cachePolicy: NSURLRequestReloadIgnoringLocalCacheData
								timeoutInterval: 20
							];
	if (bodyString) {
		[request setHTTPMethod:@"POST"];
		[request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
		[request setHTTPBody:[bodyString dataUsingEncoding:NSUTF8StringEncoding]];
		if(startPos>0) {
			[request setValue:[[NSString alloc] initWithFormat:@"bytes=%d-", startPos]
				forHTTPHeaderField:@"Range:bytes"];
		}
	} else {
		[request setHTTPMethod:@"GET"];
	}
	NSURLConnection* connection = [[NSURLConnection alloc]
									initWithRequest: request
									delegate: self
									startImmediately: YES
								];
	if(!connection) {
		[request release];
		return false;
	}
	_request = request;
	_connection = connection;
	return true;
}

// ====================
// Callbacks
// ====================

#pragma mark NSURLConnection delegate methods

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
	_struct._content_length = (int)[response expectedContentLength];
	if(_struct._content_length<0) _struct._content_length = 0;
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
	if(_struct._file) {
		fwrite([data bytes], 1, [data length], _struct._file);
	} else {
		_struct._result.append((const char*)[data bytes], (size_t)[data length]);
	}
	_struct._length += (int)[data length];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	_struct._state = MOREQUESTSTATE_DONE;
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	_struct._state = MOREQUESTSTATE_FAILED;
}

@end

void MOEnableDebug(bool enable)
{
}

MOREQUEST* MORequestString(const char* url, const Zion::Map<Zion::String, Zion::String>& params)
{
	Zion::String val;
	build_http_param(val, params);
	return MORequestString(url, val.c_str());
}

MOREQUEST* MODownloadFile(const char* url, const Zion::Map<Zion::String, Zion::String>& params, const char* path, bool append)
{
	Zion::String val;
	build_http_param(val, params);
	return MODownloadFile(url, val.c_str(), path, append);
}

MOREQUEST* MORequestString(const char* url, const char* postdata)
{
	MOHttpRequest* request = [[MOHttpRequest alloc] init];
	NSString* nurl = [NSString stringWithCString:url encoding:NSUTF8StringEncoding];
	NSString* nval = nil;
	if(postdata) {
		nval = [NSString stringWithCString:postdata encoding:NSUTF8StringEncoding];
	}
	bool ret = [request do:nurl bodyData:nval startPos:0];
	if(!ret) {
		[request release];
		return NULL;
	}
	return [request getStruct];
}

MOREQUEST* MODownloadFile(const char* url, const char* postdata, const char* path, bool append)
{
	FILE* file = fopen(path, append?"ab":"wb");
	if(file==NULL) return NULL;
	fseek(file, 0, SEEK_END);
	int loc = (int)ftell(file);
	if(loc<=1000) loc = 0; else loc -= 1000;
	fseek(file, loc, SEEK_SET);

	MOHttpRequest* request = [[MOHttpRequest alloc] init];
	[request getStruct]->_file = file;
	[request getStruct]->_length = loc;

	NSString* nurl = [NSString stringWithCString:url encoding:NSUTF8StringEncoding];
	NSString* nval = nil;
	if(postdata) {
		nval = [NSString stringWithCString:postdata encoding:NSUTF8StringEncoding];
	}
	bool ret = [request do:nurl bodyData:nval startPos:0];
	if(!ret) {
		[request getStruct]->_file = NULL;
		[request release];
		fclose(file);
		return NULL;
	}
	return [request getStruct];
}

void MORequestDestory(MOREQUEST* request)
{
	[request->request release];
}

int MODownloadLength(MOREQUEST* request)
{
	return request->_length;
}

int MOContentLength(MOREQUEST* request)
{
	return request->_content_length;
}

bool MORetryFileRequest(MOREQUEST* request)
{
	return false;
}

void MOCancelRequest(MOREQUEST* request)
{

}

MOREQUESTSTATE MORequestStatus(MOREQUEST* request)
{
	return request->_state;
}

const char* MORequestGetResult(MOREQUEST* request)
{
	return request->_result.c_str();
}

int MORequestGetResultLength(MOREQUEST* request)
{
	return request->_length;
}

extern "C"
void inflateReset2();

#endif
