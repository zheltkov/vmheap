/*
 * Copyright (c) 2004, 2011, Oracle and/or its affiliates. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *   - Neither the name of Oracle nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This source code is provided to illustrate the usage of a given feature
 * or technique and has been deliberately simplified. Additional steps
 * required for a production-quality application, such as security checks,
 * input validation and proper error handling, might not be present in
 * this sample code.
 */


#include "agent_util.hpp"
#include "versionCheck.hpp"

/* Global agent data structure */

typedef struct
{
	/* JVMTI Environment */
	jvmtiEnv* jvmti;

	/* Data access Lock */
	jrawMonitorID lock;
} GlobalAgentData;

static GlobalAgentData* gdata;

typedef struct Tag
{
	jint jvmti_heap_reference_class;
	jint jvmti_heap_reference_field;
	jint jvmti_heap_reference_array_element;
	jint jvmti_heap_reference_class_loader;
	jint jvmti_heap_reference_signers;
	jint jvmti_heap_reference_protection_domain;
	jint jvmti_heap_reference_interface;
	jint jvmti_heap_reference_static_field;
	jint jvmti_heap_reference_constant_pool;
	jint jvmti_heap_reference_superclass;
	jint jvmti_heap_reference_jni_global;
	jint jvmti_heap_reference_system_class;
	jint jvmti_heap_reference_monitor;
	jint jvmti_heap_reference_stack_local;
	jint jvmti_heap_reference_jni_local;
	jint jvmti_heap_reference_thread;
	jint jvmti_heap_reference_other;
} Tag;


typedef struct StackLocalReference 
{
	jmethodID methodId;

} StackLocalReference;


//static char* gClassName = "sun/misc/Launcher$AppClassLoader";
static char* gClassName = "org/zheltkov/heapview/Heapview";

/* Create major.minor.micro version string */
static void
version_check(jint cver, jint rver)
{
	jint cmajor, cminor, cmicro;
	jint rmajor, rminor, rmicro;

	cmajor = (cver & JVMTI_VERSION_MASK_MAJOR) >> JVMTI_VERSION_SHIFT_MAJOR;
	cminor = (cver & JVMTI_VERSION_MASK_MINOR) >> JVMTI_VERSION_SHIFT_MINOR;
	cmicro = (cver & JVMTI_VERSION_MASK_MICRO) >> JVMTI_VERSION_SHIFT_MICRO;
	rmajor = (rver & JVMTI_VERSION_MASK_MAJOR) >> JVMTI_VERSION_SHIFT_MAJOR;
	rminor = (rver & JVMTI_VERSION_MASK_MINOR) >> JVMTI_VERSION_SHIFT_MINOR;
	rmicro = (rver & JVMTI_VERSION_MASK_MICRO) >> JVMTI_VERSION_SHIFT_MICRO;
	stdout_message("CTime JVMTI Version: %d.%d.%d (0x%08x)\n",
	               cmajor, cminor, cmicro, cver);
	stdout_message("RTime JVMTI Version: %d.%d.%d (0x%08x)\n",
	               rmajor, rminor, rmicro, rver);
	if ((cmajor > rmajor) || (cmajor == rmajor && cminor > rminor))
	{
		fatal_error("ERROR: Compile Time JVMTI and Run Time JVMTI are incompatible\n");
	}
}

/* Callback for JVMTI_EVENT_VM_INIT */
static void JNICALL
vm_init(jvmtiEnv* jvmti, JNIEnv* env, jthread thread)
{
	jvmtiError err;
	jint runtime_version;

	/* The exact JVMTI version doesn't have to match, however this
	 *  code demonstrates how you can check that the JVMTI version seen
	 *  in the jvmti.h include file matches that being supplied at runtime
	 *  by the VM.
	 */
	err = jvmti->GetVersionNumber(&runtime_version);
	check_jvmti_error(jvmti, err, "get version number");
	version_check(JVMTI_VERSION, runtime_version);
}

static jint JNICALL
followReferenceCallback(
	jvmtiHeapReferenceKind reference_kind,
	const jvmtiHeapReferenceInfo* reference_info,
	jlong class_tag,
	jlong referrer_class_tag,
	jlong size,
	jlong* tag_ptr,
	jlong* referrer_tag_ptr,
	jint length,
	void* user_data)
{
	auto t = static_cast<Tag*>(user_data);

	switch (reference_kind)
	{
	case JVMTI_HEAP_REFERENCE_CLASS:
		stdout_message("JVMTI_HEAP_REFERENCE_CLASS %d\n", reference_info);
		t->jvmti_heap_reference_class++;
		break;
	case JVMTI_HEAP_REFERENCE_FIELD:
		
		/* Referecnce field */

		stdout_message("JVMTI_HEAP_REFERENCE_FIELD %d\n", reference_info->field);
		t->jvmti_heap_reference_field++;
		break;
	case JVMTI_HEAP_REFERENCE_ARRAY_ELEMENT:
		stdout_message("JVMTI_HEAP_REFERENCE_ARRAY_ELEMENT %d\n", reference_info);
		t->jvmti_heap_reference_array_element++;
		break;
	case JVMTI_HEAP_REFERENCE_CLASS_LOADER:
		stdout_message("JVMTI_HEAP_REFERENCE_CLASS_LOADER %d\n", reference_info);
		t->jvmti_heap_reference_class_loader++;
		break;
	case JVMTI_HEAP_REFERENCE_SIGNERS:
		stdout_message("JVMTI_HEAP_REFERENCE_SIGNERS %d\n", reference_info);
		t->jvmti_heap_reference_signers++;
		break;
	case JVMTI_HEAP_REFERENCE_PROTECTION_DOMAIN:
		stdout_message("JVMTI_HEAP_REFERENCE_PROTECTION_DOMAIN %d\n", reference_info);
		t->jvmti_heap_reference_protection_domain++;
		break;
	case JVMTI_HEAP_REFERENCE_INTERFACE:
		stdout_message("JVMTI_HEAP_REFERENCE_INTERFACE %d\n", reference_info);
		t->jvmti_heap_reference_interface++;
		break;
	case JVMTI_HEAP_REFERENCE_STATIC_FIELD:

		/* Referecnce static field */

		stdout_message("JVMTI_HEAP_REFERENCE_STATIC_FIELD %d\n", reference_info->field.index);
		t->jvmti_heap_reference_static_field++;
		break;
	case JVMTI_HEAP_REFERENCE_CONSTANT_POOL:
		stdout_message("JVMTI_HEAP_REFERENCE_CONSTANT_POOL %d\n", reference_info->constant_pool.index);
		t->jvmti_heap_reference_constant_pool++;
		break;
	case JVMTI_HEAP_REFERENCE_SUPERCLASS:
		stdout_message("JVMTI_HEAP_REFERENCE_SUPERCLASS %d\n", reference_info);
		t->jvmti_heap_reference_superclass++;
		break;
	case JVMTI_HEAP_REFERENCE_JNI_GLOBAL:
		stdout_message("JVMTI_HEAP_REFERENCE_JNI_GLOBAL %d\n", reference_info);
		t->jvmti_heap_reference_jni_global++;
		break;
	case JVMTI_HEAP_REFERENCE_SYSTEM_CLASS:
		stdout_message("JVMTI_HEAP_REFERENCE_SYSTEM_CLASS %d\n", reference_info);
		t->jvmti_heap_reference_system_class++;
		break;
	case JVMTI_HEAP_REFERENCE_MONITOR:
		stdout_message("JVMTI_HEAP_REFERENCE_MONITOR %d\n", reference_info);
		t->jvmti_heap_reference_monitor++;
		break;
	case JVMTI_HEAP_REFERENCE_STACK_LOCAL:
		t->jvmti_heap_reference_stack_local++;
		stdout_message("JVMTI_HEAP_REFERENCE_STACK_LOCAL %d\n", reference_info->stack_local);;
		break;
	case JVMTI_HEAP_REFERENCE_JNI_LOCAL:
		stdout_message("JVMTI_HEAP_REFERENCE_JNI_LOCAL %s\n", reference_info->jni_local.method);
		t->jvmti_heap_reference_jni_local++;
		break;
	case JVMTI_HEAP_REFERENCE_THREAD:
		stdout_message("JVMTI_HEAP_REFERENCE_THREAD %d\n", reference_info);
		t->jvmti_heap_reference_thread++;
		break;
	case JVMTI_HEAP_REFERENCE_OTHER:
		stdout_message("JVMTI_HEAP_REFERENCE_OTHER %d\n", reference_info->other);
		break;
	}

	return JVMTI_VISIT_OBJECTS;
}

static jint JNICALL
heapIterationCallback(
	jlong class_tag,
	jlong size,
	jlong* tag_ptr,
	jint length,
	void* user_data)
{
	auto count = static_cast<int*>(user_data);
	*count += 1;

	return JVMTI_VISIT_OBJECTS;
}


JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_references(JNIEnv* env, jclass thisClass)
{
	jclass componandClassLoader;
	jvmtiError error;

	stdout_message("\n\nForce GC...\n\n");
	error = gdata->jvmti->ForceGarbageCollection();
	check_jvmti_error(gdata->jvmti, error, "force garbage collection");

	componandClassLoader = env->FindClass(gClassName);
	stdout_message("Compound class loader %d\n", componandClassLoader);

	if (componandClassLoader)
	{
		jvmtiHeapCallbacks callbacks;
		(void)memset(&callbacks, 0, sizeof(callbacks));
		callbacks.heap_reference_callback = &followReferenceCallback;

		auto t = new Tag();

		error = gdata->jvmti->FollowReferences(0, componandClassLoader, nullptr, &callbacks, &t);
		check_jvmti_error(gdata->jvmti, error, "follow references");

		stdout_message("heapRefClass %d\n", t->jvmti_heap_reference_class);


		//	return t->heapRefClassLoader + t->heapRefClass + t->heapRefSystemClass + t->heapRefOther;

		//		stdout_message("tag %d\n", t->heapRefStackLocal);
	}
	return 10;
}

JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_instances(JNIEnv* env, jclass thisClass)
{
	jclass componandClassLoader;
	jvmtiError err;

	stdout_message("\n\nIncstances:\n\n");

	componandClassLoader = env->FindClass(gClassName);
	stdout_message("Compound class loader %d\n", componandClassLoader);

	auto count = 0;

	if (componandClassLoader)
	{
		jvmtiHeapCallbacks callbacks;
		(void)memset(&callbacks, 0, sizeof(callbacks));
		callbacks.heap_iteration_callback = &heapIterationCallback;

		err = gdata->jvmti->IterateThroughHeap(0, componandClassLoader, &callbacks, &count);
		check_jvmti_error(gdata->jvmti, err, "iterate through heap");
	}

	return count;
}


/* Agent_OnLoad() is called first, we prepare for a VM_INIT event here. */
JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM* vm, char* options, void* reserved)
{
	static GlobalAgentData data;
	jint rc;
	jvmtiError err;
	jvmtiEventCallbacks callbacks;
	jvmtiCapabilities capabilities;
	jvmtiEnv* jvmti;


	(void)memset(static_cast<void*>(&data), 0, sizeof(data));
	gdata = &data;

	/* Get JVMTI environment */
	rc = vm->GetEnv(reinterpret_cast<void **>(&jvmti), JVMTI_VERSION);
	if (rc != JNI_OK)
	{
		fatal_error("ERROR: Unable to create jvmtiEnv, GetEnv failed, error=%d\n", rc);
		return -1;
	}
	/* Here we save the jvmtiEnv* for Agent_OnUnload(). */
	gdata->jvmti = jvmti;

	/* Immediately after getting the jvmtiEnv* we need to ask for the
	*   capabilities this agent will need.
	*/
	(void)memset(&capabilities, 0, sizeof(capabilities));
	capabilities.can_generate_all_class_hook_events = 1;
	capabilities.can_tag_objects = 1;
	capabilities.can_generate_object_free_events = 1;
	capabilities.can_get_source_file_name = 1;
	capabilities.can_get_line_numbers = 1;
	capabilities.can_generate_vm_object_alloc_events = 1;
	err = jvmti->AddCapabilities(&capabilities);
	check_jvmti_error(jvmti, err, "Unable to get necessary JVMTI capabilities.");

	/* Set callbacks and enable event notifications */
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.VMInit = &vm_init;

	err = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
	check_jvmti_error(jvmti, err, "set event callbacks");

	err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
	check_jvmti_error(jvmti, err, "set event notify");

	return JNI_OK;
}

/* Agent_OnUnload() is called last */
JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM* vm)
{
}
