/*
 ============================================================================
 Name        : qbox_test.c
 Author      : Jiang Wen Long
 Version     : 1.0.0.0
 Copyright   : 2012 Shanghai Qiniu Information Technologies Co., Ltd.
 Description : QBOX TEST
 ============================================================================
 */

#include "qbox_test.h"

#include <stdarg.h>
#include <stdio.h>

/*============================================================================*/
/* type define */

typedef int (*TestFunc)(QBox_Client*);

typedef struct _TestNode {
	const char* name;
	TestFunc func;
	struct _TestNode* next;
}TestNode;

#define DEFNODE(func)	\
	extern int func(QBox_Client*)

#define ADDNODE(func)	\
	addTestCase(#func, func)

static void addTestCase(const char* name, TestFunc func);

/*============================================================================*/
/* add test case node here */
/* To add a node, call DEFNODE and ADDNODE for the specified func. */

DEFNODE(mkbucket);
DEFNODE(mogrifyurl);
DEFNODE(saveas);

static void genTestCase()
{
	ADDNODE(mkbucket);
	ADDNODE(mogrifyurl);
	ADDNODE(saveas);
}

/*============================================================================*/
/* implementation */

static TestNode* g_testCases = NULL;

static void addTestCase(const char* name, TestFunc func)
{
	TestNode* tail = g_testCases;
	TestNode* node = (TestNode*)malloc(sizeof(TestNode));

	node->name = name;
	node->func = func;
	node->next = NULL;
	
	if (g_testCases == NULL) {
		g_testCases = node;
	} else {
		while (tail->next != NULL) {
			tail = tail->next;
		}
		tail->next = node;
	}
}

static void runTestCase(QBox_Client* client)
{
	TestNode* node = g_testCases;
	const char* result = "SUCCESS";

	printf("START...\n\n");

	while (node != NULL) {
		printf("%s testing...\n", node->name);
		if ((*node->func)(client) != 0) {
			result = "FAIL";
			break;
		}
		printf("%s ok!\n\n", node->name);
		node = node->next;
	}

	printf("%s!\n", result);

}

static void delTestCase()
{
	while (g_testCases != NULL) {
		free(g_testCases);
		g_testCases = g_testCases->next;
	}
}

/*============================================================================*/
/* func QBT_Do */

void QBT_Do(QBox_Client* client)
{
	genTestCase();
	runTestCase(client);
	delTestCase();
}

/*============================================================================*/
/* func _QBT_Printfln */

int _QBT_Printfln(const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	vprintf(fmt, args);
	printf("\n");

	va_end(args);

	return -1;
}
