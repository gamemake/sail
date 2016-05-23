
#include "../LibBase/ZionDefines.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
#ifdef _WIN32
	(void)argc;
	(void)argv;
#else
	A_SLIST_HEADER* header = (A_SLIST_HEADER*)malloc(sizeof(A_SLIST_HEADER));
	A_SLIST_INIT(header);
	A_SLIST_ENTRY x1, x2;
	x1.Next = NULL;
	x2.Next = NULL;

	printf("%p %p %p %d\n", x1.Next, x2.Next, header->First.Next, (int)header->Count);
	printf("push %p\n", &x1);
	A_SLIST_PUSH(header, &x1);
	printf("%p %p %p %d\n", x1.Next, x2.Next, header->First.Next, (int)header->Count);
	printf("push %p\n", &x1);
	A_SLIST_PUSH(header, &x1);
	printf("%p %p %p %d\n", x1.Next, x2.Next, header->First.Next, (int)header->Count);
	printf("pop %p\n", A_SLIST_POP(header));
	printf("%p %p %p %d\n", x1.Next, x2.Next, header->First.Next, (int)header->Count);
	printf("pop %p\n", A_SLIST_POP(header));
	printf("%p %p %p %d\n", x1.Next, x2.Next, header->First.Next, (int)header->Count);

	free(header);
#endif
	return 0;
}
