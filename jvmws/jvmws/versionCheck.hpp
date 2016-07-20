#pragma once


#ifndef VERSION_CHECK_H
#define VERSION_CHECK_H

#include <jni.h>
#include <ibmjvmti.h>


#ifdef __cplusplus
extern "C"
{
#endif	

	/* find references */
	JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_references(JNIEnv* env, jobject callerObject, jobject object);

	/* find instances */
	JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_instances(JNIEnv* env, jobject callerObject);

	/* Agent_OnLoad() is called first, we prepare for a VM_INIT event here. */
	JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved);

	/* Agent_OnUnload() is called last */
	JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* vm);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif
