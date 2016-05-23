/*
#define _CRT_SECURE_NO_WARNINGS

#include "xxjson/main.cpp"
#include "xxjson/memorystat.cpp"
#include "xxjson/jsoncpptest.cpp"
*/

#include <ZionBase.h>
#include <FastJson.h>
#include <stdio.h>
#include <uv.h>

static const char* test_cases[] =
{
	"{}",
	"[]",
	"{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":{\"n\":1}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}",
	"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]",
	"[-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966,-70.260009999999966]",
/*
	"[]",
	"[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]",
	"[\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"8a0f7438-1228-4401-aed7-b93e0c05f312\"]",
	"[0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10,0.1E-10]",
	"\"8a0f7438-1228-4401-aed7-b93e0c05f312\"",
	"{\"_name\":\"kkk\",\"_uuid\":\"8a0f7438-1228-4401-aed7-b93e0c05f312\",\"v1\":0,\"v2\":0,\"v3\":0.0}",
	"0",
	"[0]",
	"{\"name\":0}",
	"{\"name\":\"0\"}",
	"\"ABCDEFG\"",
	"\"\\uABCD\"",
	"-0.1E10",
	"0.1E10",
	"-0.1E-10",
	"0.1E-10",
*/
};

#define RUN_TIMES		10000000

bool test_file(const char* filename)
{
	static char buf[99*1024*1024];
	FILE* fp = fopen(filename, "rt");
	if(!fp) return false;

	Zion::String retval;
	while(!feof(fp))
	{
		char* txt = fgets(buf, sizeof(buf)-10, fp);
		if(txt) retval.append(txt);
	}
	fclose(fp);
	{
		Zion::JsonValue node;
		node.Parse(retval.c_str(), retval.c_str()+retval.size());
		retval = node.Stringify();
	}

	printf("%s\n", filename);

	uint64_t start, end;
	start = uv_hrtime();
	for(unsigned int t=0; t<10000; t++)
	{
		Zion::JsonValue node;
		if (!node.Parse(retval.c_str(), retval.c_str()+retval.size()))
		{
			printf("ERROR S, %s\n", filename);
			return false;
		}
	}
	end = uv_hrtime();

	printf("case [S]: %fs\n", (float)(end-start)/1000000000);
	return true;
}

int main(int argc, char* argv[])
{
	Zion::JsonValue o;
	Zion::String vv = o.Stringify();

	for(_U32 i=0; i<sizeof(test_cases)/sizeof(test_cases[0]); i++)
	{
		uint64_t start, end;
		printf("%s\n", test_cases[i]);

		start = uv_hrtime();
		for(_U32 t=0; t<RUN_TIMES; t++)
		{
			Zion::JsonValue node;
			if(!node.Parse(test_cases[i], NULL))
			{
				printf("ERROR F, %s\n", test_cases[i]);
				break;
			}
		}
		end = uv_hrtime();
		printf("case [F]: %fs\n", (float)(end-start)/1000000000);
	}

	test_file("E:\\OpenSource\\nativejson-benchmark\\data\\citm_catalog.json");
	test_file("E:\\OpenSource\\nativejson-benchmark\\data\\canada.json");
	test_file("E:\\OpenSource\\nativejson-benchmark\\data\\twitter.json");

	return 0;
}
