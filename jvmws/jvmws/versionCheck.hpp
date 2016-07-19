#pragma once


#ifndef VERSION_CHECK_H
#define VERSION_CHECK_H


#include <jni.h>
//#include <jvmti.h>
#include <ibmjvmti.h>


#ifdef __cplusplus
extern "C"
{
#endif
	/* Create major.minor.micro version string */
	static void version_check(jint cver, jint rver);

	/* Callback for JVMTI_EVENT_VM_INIT */
	static void JNICALL vm_init(jvmtiEnv* jvmti, JNIEnv* env, jthread thread);

	static jint JNICALL cbHeapReference(
		jvmtiHeapReferenceKind reference_kind,
		const jvmtiHeapReferenceInfo* reference_info,
		jlong class_tag,
		jlong referrer_class_tag,
		jlong size,
		jlong* tag_ptr,
		jlong* referrer_tag_ptr,
		jint length,
		void* user_data);

	static jint JNICALL cbHeapIteration(
		jlong class_tag,
		jlong size,
		jlong* tag_ptr,
		jint length,
		void* user_data);

	JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_references(JNIEnv* env, jobject callerObject, jobject object);

	JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_instances(JNIEnv* env, jobject callerObject);

	/* Agent_OnLoad() is called first, we prepare for a VM_INIT event here. */
	JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved);

	/* Agent_OnUnload() is called last */
	JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* vm);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif
