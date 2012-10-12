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


/*============================================================================*/
/* add test case node here */
/* To add a node, call DEFNODE and ADDNODE for the specified func. */

DEFNODE(mkbucket);

static void genTestCase()
{
	ADDNODE(mkbucket);
}

/*============================================================================*/
/* func QBox_Test_Do */

void QBox_Test_Do(QBox_Client* client)
{
	genTestCase();
	runTestCase();
	delTestCase();
}

/*============================================================================*/
/* implementation */

static TestNode* g_testCases = NULL;

static void addTestCase(const char* name, TestFunc func)
{
	TestNode* node = (TestNode*)malloc(sizeof(TestNode));

	node->name = name;
	node->func = func;
	node->next = g_testCases;

	g_testCases = node;
}

static void runTestCase()
{
	TestNode* node = g_testCases;
	const char* result = "SUCCESS";

	printf("START...\n");

	while (node != NULL) {
		printf("%s:\n", node->name);
		if ((*node->func)(client) != 0) {
			result = "FAIL";
			break;
		}
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
