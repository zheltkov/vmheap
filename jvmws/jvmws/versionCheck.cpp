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

#include <vector>
#include <unordered_map>
#include <string>

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

//static char* gClassName = "sun/misc/Launcher$AppClassLoader";
static char* gClassName = "org/zheltkov/heapview/Heapview";
static std::unordered_map<long, char*> nameMap;
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

static char* getRefKind(jvmtiHeapReferenceKind reference_kind)
{
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

	return kind;
}

static Tag* pointerToTag(jlong tag_ptr)
{
	if (tag_ptr == 0)
	{
		Tag* t = new Tag();
		t->name = "new";
		t->hashCode = 0;		

		return t;
	}

	return (Tag*)(ptrdiff_t)(void*)tag_ptr;
}

static jlong tagToPointer(Tag* tag)
{
	return (jlong)(ptrdiff_t)(void*)tag;
}

static char* getObjRefKind(jvmtiObjectReferenceKind reference_kind)
{
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
	return kind;
}

static jvmtiIterationControl JNICALL heabObjectCallback(jlong class_tag, jlong size, jlong* tag_ptr, void* user_data)
{
	auto count = static_cast<int*>(user_data);
	*count += 1;	

	stdout_message("obj old tag %d\n", *tag_ptr);

	auto t = new Tag();
	t->count = *count;

	*tag_ptr = (jlong)(ptrdiff_t)(void*)t;

	stdout_message("obj new tag %d\n\n\n", *tag_ptr);
	stdout_message("Callback            Heap Object: instance count: %d; tag_ptr: %d; size: %d; class_tag: %d;\n", *count, *tag_ptr, size, class_tag);

	return JVMTI_ITERATION_CONTINUE;
}

static jvmtiIterationControl JNICALL heabObjectReferencesCallback(jvmtiObjectReferenceKind reference_kind, jlong class_tag, jlong size, jlong* tag_ptr, jlong referrer_tag, jint referrer_index, void* user_data)
{	
	if (reference_kind != JVMTI_HEAP_REFERENCE_FIELD && 
		reference_kind != JVMTI_HEAP_REFERENCE_ARRAY_ELEMENT && 
		reference_kind != JVMTI_HEAP_REFERENCE_CLASS_LOADER)
	{
		return JVMTI_ITERATION_IGNORE;
	}

	auto t = pointerToTag(*tag_ptr);
	auto rbt = pointerToTag(referrer_tag);
 	*tag_ptr = tagToPointer(t);
		
	rbt->ref_next_tags.push_back(t);
	t->ref_back_tags.push_back(rbt);
	
	auto kind = getObjRefKind(reference_kind);
	//stdout_message("Callback Heap Object References: index: %2d; referer_tag: %s; %s; tag_ptr %s\n", referrer_index, rbt->name, kind, t->name);

	if (strcmp(t->name,"new") == 0)
	{
 		//store all tags
		std::vector<jlong>* tag_ptr_list = (std::vector<jlong>*)(ptrdiff_t)(void*)user_data;
		(*tag_ptr_list).push_back(*tag_ptr);		
	}	
	return JVMTI_ITERATION_CONTINUE;
}

void callGC()
{	
	stdout_message("\n\nForce GC...\n\n");
	jvmtiError error = gdata->jvmti->ForceGarbageCollection();
	check_jvmti_error(gdata->jvmti, error, "force garbage collection");
}

char* getClassSignature(JNIEnv* env, jobject object)
{
	char* classSignature;		
	gdata->jvmti->GetClassSignature(env->GetObjectClass(object), &classSignature, nullptr);	
	return  classSignature;
}

void updateTag(JNIEnv* env, jobject object)
{
	char* objectSignature = getClassSignature(env, object);
	jint hashCode;
	gdata->jvmti->GetObjectHashCode(object, &hashCode);
	strcat(objectSignature, std::to_string((long long) hashCode).c_str());

	jlong tag_ptr;
	gdata->jvmti->GetTag(object, &tag_ptr);

	Tag* tag = (Tag*)(ptrdiff_t)(void*)tag_ptr;
	tag->name = objectSignature;	
}

static jint level;

void printRefNextTags(std::vector<Tag*> tagList)
{
	if (tagList.size() > 0)
	{
		level++;
		auto index = 1;
		stdout_message("has %d refs:\n", tagList.size());	
		for (std::vector<Tag*>::iterator it = tagList.begin(); it != tagList.end(); ++it)
		{
			stdout_message(" %s|--> %d. ",std::string(level, ' '), index++);
			printTag(*it);			
		}
		level--;
	} else
	{
		stdout_message("\n");	
	}	
}

void printTag(Tag* tag)
{
	if (tag != nullptr)
	{
		stdout_message("obj: %s ", tag->name);	
		
		if (tag->isArray && tag->value != nullptr)
		{
			//stdout_message("val: %s", tag->value);		
		}
		
 		printRefNextTags(tag->ref_next_tags);								
	}
	else
	{
		stdout_message("tag is null.");	
	}
}

void printObject(JNIEnv* env, jobject object)
{
	jlong tag_ptr;
	gdata->jvmti->GetTag(object, &tag_ptr);
	Tag* tag = (Tag*)(ptrdiff_t)(void*)tag_ptr;

	level = 1;
	printTag(tag);	
}

jlong setTag(Tag* t, jobject object)
{	
	auto tag_ptr = reinterpret_cast<ptrdiff_t>(static_cast<void*>(t));
	gdata->jvmti->SetTag(object, tag_ptr);

	stdout_message("Set tag ptr %d to obj: %d %s\n", tag_ptr, object, t->name);

	return tag_ptr;
}

//----------------------------------------------------------
jlong addNewTag(jobject object, JNIEnv *env)
{	
	char * signature = getClassSignature(env, object);
	jint hashCode;
	gdata->jvmti->GetObjectHashCode(object, &hashCode);
	strcat(signature, std::to_string((long long) hashCode).c_str());
 
 	auto t = new Tag();
	t->name = signature;

	return setTag(t, object);
}

jlong addNewTag(char* className, JNIEnv *env)
{	
	jclass klass = env->FindClass(className);
	
	auto t = new Tag();
	t->name = className;

	return setTag(t, klass);
}
//----------------------------------------------------------
void getAllTaggedObjects(JNIEnv* env, std::vector<jlong> tag_ptr_list, jint * out_count, jobject** out_objects, jlong** out_tags, jint level)
{
	if (tag_ptr_list.size() > 0) {
		jint found_count = 0;
		jlong* tags = &tag_ptr_list[0];
		jobject* found_objects;
		jlong* found_tags;

		gdata->jvmti->GetObjectsWithTags(tag_ptr_list.size(), tags, &found_count, &found_objects, &found_tags);
	
		stdout_message("%s found count %d\n", std::string(level, ' '), found_count);

		for (auto i = 0; i < found_count; ++i)
		{
			jobject found_object = found_objects[i];
			Tag* found_tag = pointerToTag(found_tags[i]);

			updateTag(env, found_object);		

     		found_tag->isArray = false;
			jboolean isArrayClass;
			gdata->jvmti->IsArrayClass(env->GetObjectClass(found_object), &isArrayClass);
			
			if (isArrayClass)
			{					
				if (strstr(found_tag->name, "[C") != nullptr)
				{
					jint lenght = env->GetArrayLength((jcharArray)found_object);
					char* buf = new char[lenght + 1];
					jboolean isCopy;
					jchar* value = env->GetCharArrayElements((jcharArray)found_object, &isCopy);
					for (auto j = 0; j < lenght; ++j){ buf[j] = value[j];}
					buf[lenght] = '\0';				
				
					found_tag->isArray = true;
					found_tag->value = buf;
					found_tag->value[lenght] = '\0';

					stdout_message("val:%s\n", found_tag->value);
				}				
			}						
		}

		/*
		for (auto i = 0; i < found_count; ++i)
		{
			jobject found_object = found_objects[i];
			printObject(env, found_object);
		}
		*/
	
		*out_count = found_count;
		*out_objects = found_objects;
		*out_tags = found_tags;
	}
}

void iterateOverObjects(JNIEnv* env, jobject object, jint level)
{
	std::vector<jlong> tag_ptr_list;

	stdout_message("%s tag list size  %d\n", std::string(level, ' '),  tag_ptr_list.size());
	gdata->jvmti->IterateOverObjectsReachableFromObject(object, &heabObjectReferencesCallback, (void*)&tag_ptr_list);
	stdout_message("%s tag list size  %d\n", std::string(level, ' '), tag_ptr_list.size());

	jint found_count = 0;
	jobject* found_objects;
	jlong* found_tags;

	getAllTaggedObjects(env, tag_ptr_list, &found_count, &found_objects, &found_tags, level);
}

JNIEXPORT jint JNICALL Java_org_zheltkov_heapview_Heapview_references(JNIEnv *env, jobject callerObject, jobject object)
{
	stdout_message("param obj %d\n", object);
	
	callGC();
		
	std::vector<jlong> tag_ptr_list;	
	
	jlong t = addNewTag(object, env);			
	tag_ptr_list.push_back(t);
	
	jint level = 0;
	iterateOverObjects(env, object, level);

	stdout_message("\n");
	printObject(env, object);
	
	return 0;

}

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
		auto t = new Tag();
		auto tag_ptr = (ptrdiff_t)(void*)&t;

		stdout_message("tag to jklass %d\n", tag_ptr);
		gdata->jvmti->SetTag(klass, tag_ptr);

		err = gdata->jvmti->IterateOverInstancesOfClass(klass, JVMTI_HEAP_OBJECT_UNTAGGED, &heabObjectCallback, &count);
		check_jvmti_error(gdata->jvmti, err, "iterate over instances of class");
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
