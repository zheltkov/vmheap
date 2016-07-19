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
	jmethodID method;
	jint count;
} Tag;


typedef struct StackLocalReference
{
	jmethodID methodId;
} StackLocalReference;


//static char* gClassName = "sun/misc/Launcher$AppClassLoader";
static char* gClassName = "org/zheltkov/heapview/Heapview";

//static char* gClassName = "com/ibm/jvm/ClassLoader";

/* Create major.minor.micro version string */
static void version_check(jint cver, jint rver)
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
static void JNICALL vm_init(jvmtiEnv* jvmti, JNIEnv* env, jthread thread)
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


/**
 * Heap API 1.1 ?
 *
 */

static jint JNICALL cbHeapReference(
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

	*tag_ptr = (jlong)(ptrdiff_t)(void*)&user_data;

	char * kind;

	switch (reference_kind)
	{
	case JVMTI_HEAP_REFERENCE_CLASS:
		kind = "JVMTI_HEAP_REFERENCE_CLASS";
		break;
	case JVMTI_HEAP_REFERENCE_FIELD:
		kind = "JVMTI_HEAP_REFERENCE_FIELD";
		break;
	case JVMTI_HEAP_REFERENCE_ARRAY_ELEMENT:
		kind = "JVMTI_HEAP_REFERENCE_ARRAY_ELEMENT";
		break;
	case JVMTI_HEAP_REFERENCE_CLASS_LOADER:
		kind = "JVMTI_HEAP_REFERENCE_CLASS_LOADER";
		break;
	case JVMTI_HEAP_REFERENCE_SIGNERS:
		kind = "JVMTI_HEAP_REFERENCE_SIGNERS";
		break;
	case JVMTI_HEAP_REFERENCE_PROTECTION_DOMAIN:
		kind = "JVMTI_HEAP_REFERENCE_PROTECTION_DOMAIN";
		break;
	case JVMTI_HEAP_REFERENCE_INTERFACE:
		kind = "JVMTI_HEAP_REFERENCE_INTERFACE";
		break;
	case JVMTI_HEAP_REFERENCE_STATIC_FIELD:
		kind = "JVMTI_HEAP_REFERENCE_STATIC_FIELD";
		break;
	case JVMTI_HEAP_REFERENCE_CONSTANT_POOL:
		kind = "JVMTI_HEAP_REFERENCE_CONSTANT_POOL";
		break;
	case JVMTI_HEAP_REFERENCE_SUPERCLASS:
		kind = "JVMTI_HEAP_REFERENCE_SUPERCLASS";
		break;
	case JVMTI_HEAP_REFERENCE_JNI_GLOBAL:
		kind = "JVMTI_HEAP_REFERENCE_JNI_GLOBAL";
		break;
	case JVMTI_HEAP_REFERENCE_SYSTEM_CLASS:
		kind = "JVMTI_HEAP_REFERENCE_SYSTEM_CLASS";
		break;
	case JVMTI_HEAP_REFERENCE_MONITOR:
		kind = "JVMTI_HEAP_REFERENCE_MONITOR";
		break;
	case JVMTI_HEAP_REFERENCE_STACK_LOCAL:
		kind = "JVMTI_HEAP_REFERENCE_STACK_LOCAL";
		break;
	case JVMTI_HEAP_REFERENCE_JNI_LOCAL:
		kind = "JVMTI_HEAP_REFERENCE_JNI_LOCAL";
		break;
	case JVMTI_HEAP_REFERENCE_THREAD:
		kind = "JVMTI_HEAP_REFERENCE_THREAD";
		break;
	case JVMTI_HEAP_REFERENCE_OTHER:
		kind = "JVMTI_HEAP_REFERENCE_OTHER";
		break;
	case jvmtiHeapReferenceKindEnsureWideEnum:
		kind = "jvmtiHeapReferenceKindEnsureWideEnum";
		break;
	default: 
		kind = "def";
		break;
	}

	stdout_message("heap1.1 cbHeapReference: %s %d %d %d\n", kind,  reference_info->array.index, reference_info->field, reference_info->stack_local.location);

	return JVMTI_VISIT_OBJECTS;
}

static jint JNICALL cbHeapIteration(
	jlong class_tag,
	jlong size,
	jlong* tag_ptr,
	jint length,
	void* user_data)
{
	auto count = static_cast<int*>(user_data);
	*count += 1;

	stdout_message("cbHeapIteration: %d, %d, %d, %d, %d\n", *count, class_tag, size, tag_ptr, length);
	auto t = new Tag();
	*tag_ptr = (ptrdiff_t)(void*)&t;

	return JVMTI_VISIT_OBJECTS;
}

static jint JNICALL cbPrimitivField(
	jvmtiHeapReferenceKind reference_kind,
	const jvmtiHeapReferenceInfo* referrer_info,
	jlong class_tag,
	jlong* tag_ptr,
	jvalue value,
	jvmtiPrimitiveType value_type,
	void* user_data)
{
	stdout_message("primitiv field\n");

	return JVMTI_VISIT_OBJECTS;
}

static jint JNICALL cbArrayPrimitiveValue(
	jlong class_tag,
	jlong size,
	jlong* tag_ptr,
	jint element_count,
	jvmtiPrimitiveType element_type,
	const void* elements,
	void* user_data)
{
	stdout_message("array primitiv value\n");

	return JVMTI_VISIT_OBJECTS;
}

static jint JNICALL cbStringPrimitiveValue(
	jlong class_tag,
	jlong size,
	jlong* tag_ptr,
	const jchar* value,
	jint value_length,
	void* user_data)
{
	stdout_message("string primitiv value\n");

	return JVMTI_VISIT_OBJECTS;
}

/**
 *   Heap 1.0 ?
 *
 **/
static jvmtiIterationControl JNICALL heabObjectCallback(jlong class_tag, jlong size, jlong* tag_ptr, void* user_data)
{
	auto count = static_cast<int*>(user_data);
	*count += 1;	

	stdout_message("obj old tag %d\n", *tag_ptr);

	auto t = new Tag();
	t->count = *count;

	*tag_ptr = (jlong)(ptrdiff_t)(void*)&t;

	stdout_message("obj new tag %d\n\n\n", *tag_ptr);
	stdout_message("Callback            Heap Object: instance count: %d; tag_ptr: %d; size: %d; class_tag: %d;\n", *count, *tag_ptr, size, class_tag);

	return JVMTI_ITERATION_CONTINUE;
}

static jvmtiIterationControl JNICALL heapRootCallback(jvmtiHeapRootKind root_kind, jlong class_tag, jlong size, jlong* tag_ptr, void* user_data)
{
	if (*tag_ptr == 0 && class_tag == 0) 
	{
		return JVMTI_ITERATION_CONTINUE;
	}

	char * root_kind_str;

	switch (root_kind)
	{
	case JVMTI_HEAP_ROOT_JNI_GLOBAL: 
		root_kind_str = "JVMTI_HEAP_ROOT_JNI_GLOBAL";  
		break;
	case JVMTI_HEAP_ROOT_SYSTEM_CLASS: 
		root_kind_str = "JVMTI_HEAP_ROOT_SYSTEM_CLASS"; 
		break;
	case JVMTI_HEAP_ROOT_MONITOR: 
		root_kind_str = "JVMTI_HEAP_ROOT_MONITOR"; 
		break;
	case JVMTI_HEAP_ROOT_STACK_LOCAL: 
		root_kind_str = "JVMTI_HEAP_ROOT_STACK_LOCAL"; 
		break;
	case JVMTI_HEAP_ROOT_JNI_LOCAL: 
		root_kind_str = "JVMTI_HEAP_ROOT_JNI_LOCAL"; 
		break;
	case JVMTI_HEAP_ROOT_THREAD:
		root_kind_str = "JVMTI_HEAP_ROOT_THREAD"; 
		break;
	case JVMTI_HEAP_ROOT_OTHER:
		root_kind_str = "JVMTI_HEAP_ROOT_OTHER"; 
		break;
	case jvmtiHeapRootKindEnsureWideEnum:
		root_kind_str = "jvmtiHeapRootKindEnsureWideEnum"; 
		break;
	default: break;
	}

	stdout_message("Callback              Heap Root: class_tag: %8d; size: %3d; tag_ptr: %d; kind: %s\n", class_tag, size, *tag_ptr, root_kind_str);
	
	return JVMTI_ITERATION_CONTINUE;
}

static jvmtiIterationControl JNICALL stackReferenceCallback(jvmtiHeapRootKind root_kind, jlong class_tag, jlong size, jlong* tag_ptr, jlong thread_tag, jint depth, jmethodID method, jint slot, void* user_data)
{
	if (*tag_ptr == 0 && class_tag == 0 && ((jint)method) < 0 && thread_tag == 0) 
	{
		return JVMTI_ITERATION_CONTINUE;
	}

	char * root_kind_str;

	switch (root_kind)
	{
	case JVMTI_HEAP_ROOT_JNI_GLOBAL: 
		root_kind_str = "JVMTI_HEAP_ROOT_JNI_GLOBAL";  
		break;
	case JVMTI_HEAP_ROOT_SYSTEM_CLASS: 
		root_kind_str = "JVMTI_HEAP_ROOT_SYSTEM_CLASS"; 
		break;
	case JVMTI_HEAP_ROOT_MONITOR: 
		root_kind_str = "JVMTI_HEAP_ROOT_MONITOR"; 
		break;
	case JVMTI_HEAP_ROOT_STACK_LOCAL: 
		root_kind_str = "JVMTI_HEAP_ROOT_STACK_LOCAL"; 
		break;
	case JVMTI_HEAP_ROOT_JNI_LOCAL: 
		root_kind_str = "JVMTI_HEAP_ROOT_JNI_LOCAL"; 
		break;
	case JVMTI_HEAP_ROOT_THREAD:
		root_kind_str = "JVMTI_HEAP_ROOT_THREAD"; 
		break;
	case JVMTI_HEAP_ROOT_OTHER:
		root_kind_str = "JVMTI_HEAP_ROOT_OTHER"; 
		break;
	case jvmtiHeapRootKindEnsureWideEnum:
		root_kind_str = "jvmtiHeapRootKindEnsureWideEnum"; 
		break;
	default:
		root_kind_str = "def"; 
		break;
	}

	stdout_message("Callback       Stack References: class_tag: %8d; size: %3d; tag_ptr: %8d; thread_tag: %d; depth: %2d; method: %9d; slot: %2d; kind: %s\n", class_tag, size, *tag_ptr, thread_tag, depth, method, slot, root_kind_str);
	return JVMTI_ITERATION_CONTINUE;
}

static jvmtiIterationControl JNICALL heabObjectReferencesCallback(jvmtiObjectReferenceKind reference_kind, jlong class_tag, jlong size, jlong* tag_ptr, jlong referrer_tag, jint referrer_index, void* user_data)
{

	if (class_tag == 0 && *tag_ptr == 0 && referrer_tag == 0) //if (reference_kind != JVMTI_HEAP_REFERENCE_THREAD)
	{
			return JVMTI_ITERATION_CONTINUE;
	}	
		
	//*tag_ptr = (jlong)(ptrdiff_t)(void*)&user_data;
	
	char* kind;

 	switch (reference_kind)
	{
	case JVMTI_HEAP_REFERENCE_CLASS:
		kind = "JVMTI_HEAP_REFERENCE_CLASS";
		break;
	case JVMTI_HEAP_REFERENCE_FIELD: 
		kind = "JVMTI_HEAP_REFERENCE_FIELD";
		break;
	case JVMTI_HEAP_REFERENCE_ARRAY_ELEMENT:
		kind = "JVMTI_HEAP_REFERENCE_ARRAY_ELEMENT";
		break;
	case JVMTI_HEAP_REFERENCE_CLASS_LOADER:
		kind = "JVMTI_HEAP_REFERENCE_CLASS_LOADER";
		break;
	case JVMTI_HEAP_REFERENCE_SIGNERS:
		kind = "JVMTI_HEAP_REFERENCE_SIGNERS";
		break;
	case JVMTI_HEAP_REFERENCE_PROTECTION_DOMAIN:
		kind = "JVMTI_HEAP_REFERENCE_PROTECTION_DOMAIN";
		break;
	case JVMTI_HEAP_REFERENCE_INTERFACE:
		kind = "JVMTI_HEAP_REFERENCE_INTERFACE";
		break;
	case JVMTI_HEAP_REFERENCE_STATIC_FIELD:
		kind = "JVMTI_HEAP_REFERENCE_STATIC_FIELD";
		break;
	case JVMTI_HEAP_REFERENCE_CONSTANT_POOL:
		kind = "JVMTI_HEAP_REFERENCE_CONSTANT_POOL";
		break;
	case JVMTI_HEAP_REFERENCE_SUPERCLASS:
		kind = "JVMTI_HEAP_REFERENCE_SUPERCLASS";
		break;
	case JVMTI_HEAP_REFERENCE_JNI_GLOBAL:
		kind = "JVMTI_HEAP_REFERENCE_JNI_GLOBAL";
		break;
	case JVMTI_HEAP_REFERENCE_SYSTEM_CLASS: 
		kind = "JVMTI_HEAP_REFERENCE_SYSTEM_CLASS";
		break;
	case JVMTI_HEAP_REFERENCE_MONITOR: 
		kind = "JVMTI_HEAP_REFERENCE_MONITOR";
		break;
	case JVMTI_HEAP_REFERENCE_STACK_LOCAL:
		kind = "JVMTI_HEAP_REFERENCE_STACK_LOCAL";
		break;
	case JVMTI_HEAP_REFERENCE_JNI_LOCAL:
		kind = "JVMTI_HEAP_REFERENCE_JNI_LOCAL";
		break;
	case JVMTI_HEAP_REFERENCE_THREAD:
		kind = "JVMTI_HEAP_REFERENCE_THREAD";
		break;
	case JVMTI_HEAP_REFERENCE_OTHER:
		kind = "JVMTI_HEAP_REFERENCE_OTHER";
		break;
	case jvmtiHeapReferenceKindEnsureWideEnum:
		kind = "jvmtiHeapReferenceKindEnsureWideEnum";
		break;
	default: 
		kind = "def";
		break;
	}

	stdout_message("Callback Heap Object References: index: %2d; referer_tag: %9d; class_tag %9d; tag_ptr %8d; kind: %s\n", referrer_index, referrer_tag, class_tag, *tag_ptr, kind);

	return JVMTI_ITERATION_CONTINUE;
}

/********************************************************/
void callGC()
{	
	stdout_message("\n\nForce GC...\n\n");
	jvmtiError error = gdata->jvmti->ForceGarbageCollection();
	check_jvmti_error(gdata->jvmti, error, "force garbage collection");
}

void printClassName(JNIEnv* env, jobject found_object)
{
	char *classSignature;
	jclass klass = env->GetObjectClass(found_object);
	jobjectRefType reftype = env->GetObjectRefType(found_object);		
	gdata->jvmti->GetClassSignature(klass, &classSignature, nullptr);	
	stdout_message("Class %s %d\n", classSignature, reftype);
}

JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_references(JNIEnv *env, jobject callerObject, jobject object)
{
	stdout_message("param obj %d\n", object);
	printClassName(env, object);

	callGC();
	
    auto t = new Tag();
	auto tag_ptr = reinterpret_cast<ptrdiff_t>(static_cast<void*>(&t));

	stdout_message("Set tag ptr %d\n", tag_ptr);
	gdata->jvmti->SetTag(object, tag_ptr);	

	auto count = 0;
 	gdata->jvmti->IterateOverObjectsReachableFromObject(object, &heabObjectReferencesCallback, &t);
	
	
	/** Heap 1.1 API ********************************************/
 	jvmtiHeapCallbacks callbacks;
	(void)memset(&callbacks, 0, sizeof(callbacks));		
	callbacks.heap_reference_callback = &cbHeapReference;
 	callbacks.primitive_field_callback = &cbPrimitivField;
	callbacks.array_primitive_value_callback = &cbArrayPrimitiveValue;
	callbacks.string_primitive_value_callback = &cbStringPrimitiveValue;
		
	gdata->jvmti->FollowReferences(0, env->GetObjectClass(object), object, &callbacks, (void*)tag_ptr);
	
	/***************************************/
 	jlong tags = {tag_ptr};
	jint found_count;
	jobject* found_objects;
	jlong* found_tags;

	gdata->jvmti->GetObjectsWithTags(10, &tags, &found_count, &found_objects, &found_tags);
	
	stdout_message("found count %d\n", found_count);
	for (int i = 0; i < found_count; ++i)
	{
		jobject found_object = found_objects[i];
		printClassName(env, found_object);
	}
	/***********************************************/

	return count;

}

JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_instances(JNIEnv *env, jobject callerObject)
{
	jclass klass;
	jvmtiError err;

	callGC();

	stdout_message("Incstances:\n\n");

	klass = env->FindClass(gClassName);
	stdout_message("Viewed class %s %d\n", gClassName, klass);	

	auto count = 0;

	if (klass != nullptr)
	{

		/** Heap 1.1 API **/

		jvmtiHeapCallbacks callbacks;
		(void)memset(&callbacks, 0, sizeof(callbacks));		
		callbacks.heap_reference_callback = &cbHeapReference;
		callbacks.heap_iteration_callback = &cbHeapIteration;		
		callbacks.primitive_field_callback = &cbPrimitivField;
		callbacks.array_primitive_value_callback = &cbArrayPrimitiveValue;
		callbacks.string_primitive_value_callback = &cbStringPrimitiveValue;
		

		/** Heap 1.0 API **/

		//err = gdata->jvmti->IterateOverHeap(JVMTI_HEAP_OBJECT_TAGGED, &heabObjectCallback, &count);
		//check_jvmti_error(gdata->jvmti, err, "iterate over heap");

		auto t = new Tag();
		auto tag_ptr = (ptrdiff_t)(void*)&t;

		stdout_message("tag to jklass %d\n", tag_ptr);
		gdata->jvmti->SetTag(klass, tag_ptr);

		err = gdata->jvmti->IterateOverInstancesOfClass(klass, JVMTI_HEAP_OBJECT_UNTAGGED, &heabObjectCallback, &count);
		check_jvmti_error(gdata->jvmti, err, "iterate over instances of class");

		auto tt = new Tag();
		err = gdata->jvmti->IterateOverReachableObjects(&heapRootCallback, &stackReferenceCallback, &heabObjectReferencesCallback, &tt);
		check_jvmti_error(gdata->jvmti, err, "iterate over reachable objects");	    


	}

	return count;
}


/* Agent_OnLoad() is called first, we prepare for a VM_INIT event here. */
void printCapabilities(jvmtiCapabilities capabilities)
{
	stdout_message("\n Capabilities:\n \
	can_tag_objects : %d\n\
	can_generate_field_modification_events : %d\n\
	can_generate_field_access_events : %d\n\
	can_get_bytecodes : %d\n\
	can_get_synthetic_attribute : %d\n\
	can_get_owned_monitor_info : %d\n\
	can_get_current_contended_monitor : %d\n\
	can_get_monitor_info : %d\n\
	can_pop_frame : %d\n\
	can_redefine_classes : %d\n\
	can_signal_thread : %d\n\
	can_get_source_file_name : %d\n\
	can_get_line_numbers : %d\n\
	can_get_source_debug_extension : %d\n\
	can_access_local_variables : %d\n\
	can_maintain_original_method_order : %d\n\
	can_generate_single_step_events : %d\n\
	can_generate_exception_events : %d\n\
	can_generate_frame_pop_events : %d\n\
	can_generate_breakpoint_events : %d\n\
	can_suspend : %d\n\
	can_redefine_any_class : %d\n\
	can_get_current_thread_cpu_time : %d\n\
	can_get_thread_cpu_time : %d\n\
	can_generate_method_entry_events : %d\n\
	can_generate_method_exit_events : %d\n\
	can_generate_all_class_hook_events : %d\n\
	can_generate_compiled_method_load_events : %d\n\
	can_generate_monitor_events : %d\n\
	can_generate_vm_object_alloc_events : %d\n\
	can_generate_native_method_bind_events : %d\n\
	can_generate_garbage_collection_events : %d\n\
	can_generate_object_free_events : %d\n\
	can_force_early_return : %d\n\
	can_get_owned_monitor_stack_depth_info : %d\n\
	can_get_constant_pool : %d\n\
	can_set_native_method_prefix : %d\n\
	can_retransform_classes : %d\n\
	can_retransform_any_class : %d\n\
	can_generate_resource_exhaustion_heap_events : %d\n\
	can_generate_resource_exhaustion_threads_events : %d\n\n",
	               capabilities.can_tag_objects,
	               capabilities.can_generate_field_modification_events,
	               capabilities.can_generate_field_access_events,
	               capabilities.can_get_bytecodes,
	               capabilities.can_get_synthetic_attribute,
	               capabilities.can_get_owned_monitor_info,
	               capabilities.can_get_current_contended_monitor,
	               capabilities.can_get_monitor_info,
	               capabilities.can_pop_frame,
	               capabilities.can_redefine_classes,
	               capabilities.can_signal_thread,
	               capabilities.can_get_source_file_name,
	               capabilities.can_get_line_numbers,
	               capabilities.can_get_source_debug_extension,
	               capabilities.can_access_local_variables,
	               capabilities.can_maintain_original_method_order,
	               capabilities.can_generate_single_step_events,
	               capabilities.can_generate_exception_events,
	               capabilities.can_generate_frame_pop_events,
	               capabilities.can_generate_breakpoint_events,
	               capabilities.can_suspend,
	               capabilities.can_redefine_any_class,
	               capabilities.can_get_current_thread_cpu_time,
	               capabilities.can_get_thread_cpu_time,
	               capabilities.can_generate_method_entry_events,
	               capabilities.can_generate_method_exit_events,
	               capabilities.can_generate_all_class_hook_events,
	               capabilities.can_generate_compiled_method_load_events,
	               capabilities.can_generate_monitor_events,
	               capabilities.can_generate_vm_object_alloc_events,
	               capabilities.can_generate_native_method_bind_events,
	               capabilities.can_generate_garbage_collection_events,
	               capabilities.can_generate_object_free_events,
	               capabilities.can_force_early_return,
	               capabilities.can_get_owned_monitor_stack_depth_info,
	               capabilities.can_get_constant_pool,
	               capabilities.can_set_native_method_prefix,
	               capabilities.can_retransform_classes,
	               capabilities.can_retransform_any_class,
	               capabilities.can_generate_resource_exhaustion_heap_events,
	               capabilities.can_generate_resource_exhaustion_threads_events);
}

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
	jvmti->GetCapabilities(&capabilities);
	capabilities.can_tag_objects = 1;
	capabilities.can_generate_all_class_hook_events = 1;
	capabilities.can_generate_object_free_events = 1;
	capabilities.can_get_source_file_name = 1;
	capabilities.can_get_line_numbers = 1;
	capabilities.can_generate_vm_object_alloc_events = 1;
	capabilities.can_generate_field_access_events = 1;
	err = jvmti->AddCapabilities(&capabilities);

	//printCapabilities(capabilities);

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
