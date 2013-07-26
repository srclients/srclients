#include "com_example_srclient_KP.h"
#include "kp.h"
#include <android/log.h>


/**
 * @fn  JNIEXPORT jint JNICALL Java_com_example_srclient_KP_connectSmartSpace(JNIEnv *env, jobject obj, jstring hostname, jstring ip, jint port)
 *
 * @brief Function connect client to Smart Space
 *
 * @param env - pointer to JNI environment
 * @param obj - Java object
 * @param hostname - name of Smart Space
 * @param ip - ip address
 * @param port - SIB port
 *
 * @return Returns 0 in success and -1 if failed
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_connectSmartSpace
  (JNIEnv *env, jobject obj, jstring hostname, jstring ip, jint port) {

	const char *hostname_ = (*env) -> GetStringUTFChars(env, hostname, NULL);
	const char *ip_ = (*env) -> GetStringUTFChars(env, ip, NULL);
	char *uuid = generateUuid("KP");

	if(sslog_ss_init_session_with_parameters(hostname_, ip_, (int)port) != 0)
		return -1;

	register_ontology();

	if (ss_join(sslog_get_ss_info(), uuid) == -1) {
		__android_log_print(ANDROID_LOG_ERROR, "connectionSmartSpace():", "Connection error");
		return -1;
	}

	if((*env)->GetJavaVM(env, &JVM) != 0)
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Get java VM failed");

	return 0;
}


/**
 * @fn  Java_com_example_srclient_KP_disconnectSmartSpace(JNIEnv *env, jobject obj)
 *
 * @brief Function disconnect client from Smart Space
 *
 * @param env - pointer to JNI environment
 * @param obj - Java object
 */
JNIEXPORT void JNICALL Java_com_example_srclient_KP_disconnectSmartSpace
  (JNIEnv *env, jobject obj) {

	sslog_sbcr_stop_all();
	sslog_sbcr_unsubscribe_all(true);
	sslog_ss_leave_session(sslog_get_ss_info());
	sslog_repo_clean_all();
}


/**
 * @fn  Java_com_example_srclient_KP_userRegistration(JNIEnv *env, jobject obj, jstring userName, jstring password)
 *
 * @brief Register client in Smart Space
 *
 * Function takes `userName` and `password` and compares them
 * with corresponding individual's properties
 *
 * @param env - pointer to JNI environment
 * @param obj - Java object
 * @param userName - registered user name
 * @param password - user password
 *
 * @return Returns 0 in success and -1 if failed
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_userRegistration
  (JNIEnv *env, jobject obj, jstring userName, jstring password) {

	list_t* profileList = sslog_ss_get_individual_by_class_all(CLASS_PROFILE);
	int personFounded = -1;

	if(profileList == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "userRegistration():",
						"Can't find Profile individual");
		return -1;
	} else {
		list_head_t* pos = NULL;
		list_for_each(pos, &profileList->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual_t* profile = (individual_t*)(node->data);
			sslog_ss_populate_individual(profile);


			if(profile != NULL)
				personFounded = searchPerson(profile,
						(*env)->GetStringUTFChars(env, userName, NULL),
						(*env)->GetStringUTFChars(env, password, NULL));
			else {
				__android_log_print(ANDROID_LOG_ERROR, "userRegistration()",
						"Can't find individual by uuid");
				return -1;
			}

			if(personFounded == 0) {
				if(activatePerson(profile) != 0)
					return -1;
				return 0;
			}
		}
	}

	return personFounded;
}


/**
 * @fn  searchPerson(individual_t *person, char *userName, char *password)
 *
 * @brief Searches person in Smart Space
 *
 * @param person - pointer to person individual
 * @param userName - registered user name
 * @param password - user password
 *
 * @return Returns 0 in success and -1 if failed
 */
int searchPerson(individual_t *person, char *userName, char *password) {

	prop_val_t *p_val_username = sslog_ss_get_property (person, PROPERTY_USERNAME);

	if(p_val_username == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Property user name not found");
		p_val_username = initNullProperty();
	}

	prop_val_t *p_val_password = sslog_ss_get_property (person, PROPERTY_PASSWORD);

	if(p_val_password == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Property password not found");
		p_val_password = initNullProperty();
	}

	if((strcmp(userName, (char *)p_val_username->prop_value) == 0) &&
			(strcmp(password, (char *)p_val_password->prop_value) == 0)) {
		personProfile = person;
		return 0;
	}


	return -1;
}


/**
 * @fn activatePerson(individual_t *profile)
 *
 * @brief Send `newParticipantCome` notification
 *
 * If searchPerson() function ended successful then this function
 * sends notification to AgendaNotification class
 *
 * @param profile - pointer to profile individual
 *
 * @return Returns 0 in success and -1 if failed
 */
int activatePerson(individual_t *profile) {

	prop_val_t *person_prop = sslog_ss_get_property(profile, PROPERTY_PERSONINFORMATION);

	individual_t *person = (individual_t *)person_prop->prop_value;

	sslog_ss_populate_individual(person);

	if(person == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "userRegistration()",
										"person == NULL");
		return -1;
	}

	char *status = "online";

	if(sslog_ss_add_property(person, PROPERTY_STATUS, (void *)status) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}

	individual_t *agendaGui = sslog_new_individual(CLASS_AGENDANOTIFICATION);

	sslog_set_individual_uuid(agendaGui, "http://www.cs.karelia.ru/smartroom#AgendaNotification");

	if(sslog_ss_add_property(agendaGui, PROPERTY_NEWPARTICIPANTCOME, profile) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(agendaGui) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "insert failed");
		return -1;
	}

	return 0;
}


/**
 * @fn Java_com_example_srclient_KP_loadTimeslotList(JNIEnv *env, jclass clazz, jobject obj)
 *
 * @brief Gets TimeSlot individuals from Smart Space
 *
 * Gets the list of TimeSlot class individuals from Smart Space
 * processes it and fills Agenda.
 *
 * @param env - pointer to JNI environment
 * @param clazz - class keeping loadTimeSlot() function
 * @param obj - Agenda class object
 *
 * @return Returns 0 in success and -1 if failed
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_loadTimeslotList
  (JNIEnv *env, jclass clazz, jobject obj) {

	// TODO:
	// when there will be more than one section, it's enough only to
	// initialize chosen `section`

	if(obj != NULL)
		agendaClassObject = (jobject *)(*env)->NewGlobalRef(env, obj);
	else
		return -1;

	prop_val_t *propTimeslot = sslog_ss_get_property(section, PROPERTY_FIRSTTIMESLOT);

	while(propTimeslot != NULL) {
		individual_t *pTimeslot = (individual_t *) propTimeslot->prop_value;

		addTimeslotToJavaList(env, pTimeslot, obj);

		propTimeslot = sslog_ss_get_property(pTimeslot, PROPERTY_NEXTTIMESLOT);
	}

	return 0;
}


/**
 * @fn addTimeslotToJavaList(JNIEnv *env, individual_t *timeslot, jobject obj)
 *
 * @brief Fills Agenda by timeslot's properties values
 *
 * Extracts timeslot's properties and stores them into
 * Agenda timeslot list.
 *
 * @param env - pointer to JNI environment
 * @param timeslot - timeslot individual
 * @param obj - Agenda class object
 *
 * @return Returns 0 in success and -1 if failed
 */
void addTimeslotToJavaList(JNIEnv *env, individual_t *timeslot, jobject obj) {

	jmethodID methodId = (*env)->GetMethodID(env, classAgenda, "addTimeslotItemToList",
			"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
			"Ljava/lang/String;)V");

	/* Gets participant name property */
	prop_val_t *p_val_name = sslog_ss_get_property (timeslot, PROPERTY_SPEAKERNAME);
	if(p_val_name == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Property speaker name not found");
		p_val_name = initNullProperty();
	} /* Gets participant name property */


	/* Gets duration property */
	prop_val_t *p_val_duration = sslog_ss_get_property (timeslot, PROPERTY_DURATION);
	if(p_val_duration == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Property duration not found");
		p_val_duration = initNullProperty();
	} /* Gets duration property */


	/* Gets presentation title property */
	prop_val_t *p_val_pres_title = sslog_ss_get_property (timeslot, PROPERTY_PRESENTATIONTITLE);
	if(p_val_pres_title == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Property presentation title not found");
		p_val_pres_title = initNullProperty();
	} /* Gets presentation title property */


	/* Gets person link property */
	char *imgLink = (char *) malloc (sizeof(char) * 200);

	strcpy(imgLink, "absentImage");

	prop_val_t *p_val_person_link = sslog_ss_get_property (timeslot, PROPERTY_PERSONLINK);
	if(p_val_person_link != NULL) {

		individual_t *person = (individual_t *)p_val_person_link->prop_value;
		sslog_ss_populate_individual(person);

		prop_val_t *p_val_status = sslog_ss_get_property (person, PROPERTY_STATUS);

		if(p_val_status != NULL) {
			if(strcmp((char *)p_val_status->prop_value, "online") == 0) {

				prop_val_t *p_val_img = sslog_ss_get_property (person, PROPERTY_IMG);

				if(p_val_img != NULL) {
					strcpy(imgLink, (char *)p_val_img->prop_value);
				}
			}
		}
	}

	/* Calling Agenda's addTimeslotItemToList Java method */
	if(obj != NULL)
		(*env)->CallVoidMethod(env, obj, methodId,
				(*env)->NewStringUTF(env, (char *)(p_val_name->prop_value)),
				(*env)->NewStringUTF(env, (char *)(p_val_duration->prop_value)),
				(*env)->NewStringUTF(env, (char *)(p_val_pres_title->prop_value)),
				(*env)->NewStringUTF(env, imgLink));

	free(imgLink);

}


/***********************************
 *
 *  Initialize NULL property with ""
 *
 ***********************************/
prop_val_t* initNullProperty() {
	prop_val_t* property = (prop_val_t *) malloc (sizeof(prop_val_t));

	if(!property) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "malloc error");
		return NULL;
	}

	property->prop_value = "";

	return property;
}/* Initialize NULL property with "" */

/***********************
 *
 * Generates unique uuid
 *
 ***********************/
char* generateUuid(char *uuid) {
	int postfix_length = 6;
	char *result = (char*) malloc (sizeof(char) * strlen(uuid) + postfix_length);
	char *rand_chr = calloc (postfix_length + 1, sizeof(char));
	int rand_val = 0;
	int rand_length = 1, i = 0;

	for(; i < postfix_length; rand_length *= 10, i++);

	do {
		srand(time(NULL));
		rand_val = rand() % rand_length;
		if(snprintf(rand_chr, postfix_length + 1, "%d", rand_val) < 0) {
			__android_log_print(ANDROID_LOG_ERROR, "generateUuid:", "snprintf error");
			return uuid;
		}
		strcpy(result, uuid);
		strcat(result, "-");
		strcat(result, rand_chr);
	} while(sslog_ss_exists_uuid(result) == 1);

	__android_log_print(ANDROID_LOG_ERROR, "generateUuid:", result);

	free(rand_chr);

	return result;
}/* Generates unique uuid */


/**********************************
 *
 *  Gets list of available services
 *
 **********************************/
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_getServicesInfo
  (JNIEnv *env, jclass clazz, jobject menu) {

	jclass class = getJClassObject(env, "ServicesMenu");
	jfieldID agendaStateField = getFieldID(env, class, "agendaServiceState", "I");
	jfieldID projectorStateField = getFieldID(env, class, "projectorServiceState", "I");


	individual_t *agenda = (individual_t *)
			sslog_ss_get_individual_by_class_all(CLASS_AGENDA);

	if(agenda == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Agenda service not found");
		(*env)->SetIntField(env, menu, agendaStateField, 0);
	} else
		(*env)->SetIntField(env, menu, agendaStateField, 1);


	individual_t *projector = (individual_t *)
			sslog_ss_get_individual_by_class_all(CLASS_PROJECTORSERVICE);

	if(projector == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Projector service not found");
		(*env)->SetIntField(env, menu, projectorStateField, 0);
	} else
		(*env)->SetIntField(env, menu, projectorStateField, 1);

	return 0;

}/* Gets list of available services */


/*************************************************
 *
 * Returns jclass object of Java class `className`
 *
 *************************************************/
jclass getJClassObject(JNIEnv *env, char *className) {

	char *classPath = (char *) malloc (sizeof(char) * strlen("com/example/srclient/") + strlen(className));

	strcpy(classPath, "com/example/srclient/");
	strcat(classPath, className);

	jclass class = (*env)->FindClass(env, classPath);

	if(class == NULL) {
		char *error = (char *) malloc (sizeof(char) * 30);
		strcpy(error, "Can't find ");
		strcat(error, className);
		strcat(error, " class");
		__android_log_print(ANDROID_LOG_ERROR, "class KP", (char *)error);
		return NULL;
	}

	free(classPath);

	return class;
}/* Returns jclass object of Java class `className` */


/**************************************
 *
 * Returns ID of Java field `fieldName`
 *
 **************************************/
jfieldID getFieldID(JNIEnv *env, jclass class, char *fieldName, char *signature) {

	jfieldID fieldID = (*env)->GetFieldID(env, class, fieldName, signature);
	char *err_msg = (char *) malloc (sizeof(char) * ERROR_MSG_LENGTH);

	strcpy(err_msg, "Can't get ");
	strcat(err_msg, fieldName);
	strcat(err_msg, " ID");

	if(fieldID == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", err_msg);
		return NULL;
	}

	free(err_msg);

	return fieldID;
}


/**
 * @brief Initializes subscription to conference activities
 *
 * @return 0 in success, -1 if fail
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_initSubscription
  (JNIEnv *env, jobject obj) {

	jclass *classAgendaObj = getJClassObject(env, "Agenda");
	jclass *classProjectorObj = getJClassObject(env, "Projector");
	jclass *classKPObj = getJClassObject(env, "KP");
	kpClassObject = (jobject *)(*env)->NewGlobalRef(env, obj);

	classAgenda = (jclass *)(*env)->NewGlobalRef(env, classAgendaObj);
	classProjector = (jclass *)(*env)->NewGlobalRef(env, classProjectorObj);
	classKP = (jclass *)(*env)->NewGlobalRef(env, classKPObj);
	list_t *projectorServiceList;
	individual_t *projectorService;

	extern void agendaNotificationHandler(subscription_t *);
	extern void projectorNotificationHandler(subscription_t *);
	void (*pAgendaHandler)(subscription_t *) = &agendaNotificationHandler;
	void (*pProjectorHandler)(subscription_t *) = &projectorNotificationHandler;


	subscription_t *agendaSubscriptionContainer = sslog_new_subscription(true);

	/* Subscription to agenda */
	sslog_sbcr_add_class(agendaSubscriptionContainer, CLASS_AGENDAGUINOTIFICATION);

	list_t *listProperties = list_get_new_list();
	list_add_data(PROPERTY_CURRENTTIMESLOT, listProperties);

	/* Get section */
	list_t *sectionList = sslog_ss_get_individual_by_class_all(CLASS_SECTION);

	if(sectionList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &sectionList->links) {
			list_t* node = list_entry(pos, list_t, links);
			section = (individual_t*)(node->data);
			sslog_ss_populate_individual(section);
			break;
		}
	}/* Get section */


	prop_val_t *firstTimeslotProp = sslog_ss_get_property(section, PROPERTY_FIRSTTIMESLOT);

	if(firstTimeslotProp != NULL)
		firstTimeslot = (individual_t *)firstTimeslotProp->prop_value;


	if(sslog_sbcr_add_individual(agendaSubscriptionContainer, section, listProperties)
			!= ERROR_NO)
		__android_log_print(ANDROID_LOG_ERROR, "initSbcr:", "curtimeslot sbcr fail");

	sslog_sbcr_set_changed_handler(agendaSubscriptionContainer, pAgendaHandler);
	/* Subscription to agenda */


	/* Subscription to projector service */
	subscription_t *projectorSubscriptionContainer = sslog_new_subscription(true);
	projectorServiceList = sslog_ss_get_individual_by_class_all(CLASS_PROJECTORSERVICE);

	if(projectorServiceList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &projectorServiceList->links) {
			list_t* node = list_entry(pos, list_t, links);
			projectorService = (individual_t*)(node->data);
			sslog_ss_populate_individual(projectorService);
			__android_log_print(ANDROID_LOG_ERROR, "class KP", "projectorList exists");
		}

		list_t *listProperties = list_get_new_list();
		list_add_data(PROPERTY_CURRENTSLIDENUM, listProperties);
		list_add_data(PROPERTY_CURRENTSLIDECOUNT, listProperties);
		list_add_data(PROPERTY_CURRENTSLIDEIMG, listProperties);

		if(sslog_sbcr_add_individual(projectorSubscriptionContainer, projectorService, listProperties)
				!= ERROR_NO)
			__android_log_print(ANDROID_LOG_ERROR, "initSbcr:", "add indiv sbcr fail");
	}

	sslog_sbcr_set_changed_handler(projectorSubscriptionContainer, pProjectorHandler);
	/* Subscription to projector service */


	if(sslog_sbcr_subscribe(agendaSubscriptionContainer) != ERROR_NO) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}

	if(sslog_sbcr_subscribe(projectorSubscriptionContainer) != ERROR_NO) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Handles agenda changes
 */
void agendaNotificationHandler(subscription_t *sbcr) {

	/* Gets jni environment pointer (env) */
	JNIEnv *env = NULL;
	bool attached = false;

	switch((*JVM)->GetEnv(JVM, (void **)&env, JNI_VERSION_1_6)) {

		case JNI_OK:
			break;

		case JNI_EDETACHED:
			(*JVM)->AttachCurrentThread(JVM, &env, NULL);
			attached = true;
			break;

		case JNI_EVERSION:
			__android_log_print(ANDROID_LOG_ERROR, "agendaHandler:",
					"invalid jni version");
			break;
	}

	jmethodID updateAgenda = (*env)->GetMethodID(env, classAgenda, "updateAgenda", "()V");
	//jmethodID setCurTimeslot = (*env)->GetMethodID(env, classAgenda, "setCurrentTimeslot", "(I)V");

	jfieldID agendaCreated = (*env)->GetStaticFieldID(env, classAgenda,
			"agendaCreated", "I");
	int agendaIsCreated = (*env)->GetStaticIntField(env, classAgenda, agendaCreated);

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);
	list_t *list = sslog_sbcr_ch_get_individual_all(changes);

	if(list != NULL) {
		list_head_t *list_walker = NULL;

		list_for_each(list_walker, &list->links) {
			list_t *node = list_entry(list_walker, list_t, links);
			char *uuid = (char *) node->data;

			individual_t *individual = (individual_t *)
					sslog_repo_get_individual_by_uuid(uuid);

			prop_val_t *p_val_update = sslog_ss_get_property (individual,
						PROPERTY_UPDATEAGENDA);
			if(p_val_update != NULL) {
				if(agendaIsCreated == 1) {
					__android_log_print(ANDROID_LOG_ERROR, "agendaHandler:", "calling update agenda");
					(*env)->CallVoidMethod(env, agendaClassObject, updateAgenda);
					__android_log_print(ANDROID_LOG_ERROR, "agendaHandler:", "update OK");
				}
			}

			/*prop_val_t *p_val_timeslot = sslog_ss_get_property (individual,
									PROPERTY_CURRENTTIMESLOT);
			if(p_val_timeslot != NULL) {
				individual_t *curTimeslot = (individual_t *) p_val_timeslot->prop_value;
				individual_t *timeslot = firstTimeslot;
				currentTimeslotIndex = 1;

				__android_log_print(ANDROID_LOG_ERROR, "agendaHandler:", "currentTimeslot handling");

				// Get index of current timeslot
				while(strcmp(curTimeslot->uuid, timeslot->uuid) != 0) {
					prop_val_t *val = sslog_ss_get_property(timeslot, PROPERTY_NEXTTIMESLOT);

					if(val != NULL) {
						timeslot = (individual_t *) val->prop_value;
						++currentTimeslotIndex;
					} else break;
				}

				/*if(agendaIsCreated == 1)
					(*env)->CallVoidMethod(env, agendaClassObject, setCurTimeslot, currentTimeslotIndex);
				else
					break;

				__android_log_print(ANDROID_LOG_ERROR, "agendaHandler:", "timeslot ok");*/
			//}
		}
	}

	list_free_with_nodes(list, NULL);

	if(attached)
		(*JVM)->DetachCurrentThread(JVM);
}

JNIEXPORT jint JNICALL Java_com_example_srclient_KP_getCurrentTimeslotIndex
  (JNIEnv *env, jclass clazz) {
	return currentTimeslotIndex;
}

/**
 * @brief Function checks the fact that current
 * participant is a speaker.
 *
 * @return True if participant is a speaker and
 * false otherwise
 */
JNIEXPORT jboolean JNICALL Java_com_example_srclient_KP_checkSpeakerState
  (JNIEnv *env, jclass clazz) {

	individual_t *timeslot;
	prop_val_t *curValue = sslog_ss_get_property(section, PROPERTY_CURRENTTIMESLOT);

	if(curValue == NULL)
		return JNI_FALSE;

	timeslot = (individual_t *) curValue->prop_value;

	prop_val_t *personValue = sslog_ss_get_property(timeslot, PROPERTY_PERSONLINK);
	individual_t *curPerson = (individual_t *) personValue->prop_value;

	prop_val_t *person = sslog_ss_get_property(personProfile, PROPERTY_PERSONINFORMATION);
	individual_t *personInfo = (individual_t *) person->prop_value;

	if(strcmp(personInfo->uuid, curPerson->uuid) == 0)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

/********************************************************
 *
 * Loads presentation with `uuid` or current presentation
 *
 ********************************************************/
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_loadPresentation
  (JNIEnv *env, jobject obj, jobject projectorClassObj) {

	prop_val_t *p_val_slideNum = NULL;
	prop_val_t *p_val_slideCount = NULL;
	prop_val_t *p_val_slideImg = NULL;
	individual_t *projectorService = NULL;

	if(projectorClassObj == NULL)
		return -1;

	projectorClassObject = (jobject *)(*env)->NewGlobalRef(env, projectorClassObj);

	__android_log_print(ANDROID_LOG_ERROR, "loadPresentation()", "start");

	jmethodID setSlideNumId = (*env)->GetMethodID(env, classProjector,
			"setSlideNumber", "(Ljava/lang/String;)V");
	jmethodID setSlideCountId = (*env)->GetMethodID(env, classProjector,
			"setSlideCount", "(Ljava/lang/String;)V");
	jmethodID setSlideImageId = (*env)->GetMethodID(env, classProjector,
			"setSlideImage", "(Ljava/lang/String;)V");

	list_t* projectorServiceList = sslog_ss_get_individual_by_class_all(
				CLASS_PROJECTORSERVICE);

	if(projectorServiceList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &projectorServiceList->links) {
			list_t* node = list_entry(pos, list_t, links);
			projectorService = (individual_t*)(node->data);
			sslog_ss_populate_individual(projectorService);
			break;
		}

		// Set current slide img value
		p_val_slideImg = sslog_ss_get_property (projectorService,
				PROPERTY_CURRENTSLIDEIMG);
		if(p_val_slideImg != NULL)
			(*env)->CallVoidMethod(env, projectorClassObject, setSlideImageId,
					(*env)->NewStringUTF(env, (char *)p_val_slideImg->prop_value));
		else
			(*env)->CallVoidMethod(env, projectorClassObject, setSlideImageId,
					(*env)->NewStringUTF(env, ""));

		// Set current slide number value
		p_val_slideNum = sslog_ss_get_property (projectorService,
				PROPERTY_CURRENTSLIDENUM);
		if(p_val_slideNum != NULL)
			(*env)->CallVoidMethod(env, projectorClassObject, setSlideNumId,
					(*env)->NewStringUTF(env, (char *)p_val_slideNum->prop_value));

		// Set current slide count value
		p_val_slideCount = sslog_ss_get_property (projectorService,
				PROPERTY_CURRENTSLIDECOUNT);
		if(p_val_slideCount != NULL)
			(*env)->CallVoidMethod(env, projectorClassObject, setSlideCountId,
					(*env)->NewStringUTF(env, (char *)p_val_slideCount->prop_value));
	}

	__android_log_print(ANDROID_LOG_ERROR, "loadPresentation()", "DONE");

	return 0;
}/* Loads presentation with `uuid` */



/**
 * @brief Handles changes happened with `CLASS_PROJECTORNOTIFICATION`
 *
 * @param sbcr
 */
void projectorNotificationHandler(subscription_t *sbcr) {

	/* Gets jni environment pointer (env) */
	JNIEnv *env;
	bool attached = false;

	switch((*JVM)->GetEnv(JVM, (void **)&env, JNI_VERSION_1_6)) {

		case JNI_OK:
			break;

		case JNI_EDETACHED:
			__android_log_print(ANDROID_LOG_ERROR, "projHandler:", "not attached");
			(*JVM)->AttachCurrentThread(JVM, &env, NULL);
			attached = true;
			break;

		case JNI_EVERSION:
			__android_log_print(ANDROID_LOG_ERROR, "projectorHandler:",
					"invalid jni version");
			break;
	}

	jmethodID setSlideNumId = (*env)->GetMethodID(env, classProjector,
			"setSlideNumber", "(Ljava/lang/String;)V");
	jmethodID setSlideCountId = (*env)->GetMethodID(env, classProjector,
			"setSlideCount", "(Ljava/lang/String;)V");
	jmethodID setSlideImageId = (*env)->GetMethodID(env, classProjector,
			"setSlideImage", "(Ljava/lang/String;)V");

	jfieldID projectorCreated = (*env)->GetStaticFieldID(env, classProjector,
			"projectorCreated", "I");

	int projectorIsCreated = (*env)->GetStaticIntField(env,
			classProjector, projectorCreated);

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);
		list_t *list = sslog_sbcr_ch_get_individual_all(changes);

	if(projectorIsCreated != 1)
		return;

	// Handling last changes of subscribed data
	if(list != NULL) {
		list_head_t *list_walker = NULL;

		list_for_each(list_walker, &list->links) {
			list_t *node = list_entry(list_walker, list_t, links);
			char *uuid = (char *) node->data;

			individual_t *individual = (individual_t *)
					sslog_repo_get_individual_by_uuid(uuid);

			// Current slide number property
			prop_val_t *p_val_slidenum = sslog_ss_get_property (individual,
						PROPERTY_CURRENTSLIDENUM);
			if(p_val_slidenum != NULL) {
				(*env)->CallVoidMethod(env, projectorClassObject, setSlideNumId,
						(*env)->NewStringUTF(env, (char *)p_val_slidenum->prop_value));
			}

			// Current slide count property
			prop_val_t *p_val_slidecount = sslog_ss_get_property (individual,
						PROPERTY_CURRENTSLIDECOUNT);
			if(p_val_slidecount != NULL) {
				(*env)->CallVoidMethod(env, projectorClassObject, setSlideCountId,
						(*env)->NewStringUTF(env, (char *)p_val_slidecount->prop_value));
			}

			// Current slide image property
			prop_val_t *p_val_slideimg = sslog_ss_get_property (individual,
						PROPERTY_CURRENTSLIDEIMG);
			if(p_val_slideimg != NULL) {
				(*env)->CallVoidMethod(env, projectorClassObject, setSlideImageId,
						(*env)->NewStringUTF(env, (char *)p_val_slideimg->prop_value));
			}
		}
	}

	list_free_with_nodes(list, NULL);

	if(attached)
		(*JVM)->DetachCurrentThread(JVM);
}


/**
 * @brief Starts conference
 *
 * @return 0 in success, -1 otherwise
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_startConference
  (JNIEnv *env, jobject obj) {

	individual_t *individual = sslog_new_individual(CLASS_AGENDANOTIFICATION);
	sslog_set_individual_uuid(individual, "http://www.cs.karelia.ru/smartroom#AgendaNotification");

	if(sslog_ss_add_property(individual, PROPERTY_STARTCONFERENCE, firstTimeslot) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "startConference()", get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "startConference()", get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Ends conference
 *
 * @return 0 in success, -1 otherwise
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_endConference
  (JNIEnv *env, jobject obj) {

	individual_t *individual = sslog_new_individual(CLASS_AGENDANOTIFICATION);
	sslog_set_individual_uuid(individual, "http://www.cs.karelia.ru/smartroom#AgendaGuiNotification");

	if(sslog_ss_add_property(individual, PROPERTY_ENDCONFERENCE, firstTimeslot) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "endConference()", get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "endConference()", get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Changes current presentation slide
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_showSlide
  (JNIEnv *env, jclass clazz, jint slideNumber) {

	char *number = calloc (20, sizeof(char));
	sprintf(number, "%d", (int)slideNumber);

	individual_t *individual = sslog_new_individual(CLASS_PROJECTORNOTIFICATION);
	sslog_set_individual_uuid(individual, "http://www.cs.karelia.ru/smartroom#ProjectorNotification");

	if(sslog_ss_add_property(individual, PROPERTY_SHOWSLIDE,  (void *)number) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "showSlide()", get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "showSlide()", get_error_text());
		return -1;
	}

	free(number);

	return 0;
}

/**
 * @brief Ends current presentation
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_endPresentation
  (JNIEnv *env, jclass clazz) {

	prop_val_t *curPresentation = NULL;
	individual_t *projectorService = NULL;
	list_t* projectorServiceList = sslog_ss_get_individual_by_class_all(
			CLASS_PROJECTORSERVICE);

	if(projectorServiceList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &projectorServiceList->links) {
			list_t* node = list_entry(pos, list_t, links);
			projectorService = (individual_t*)(node->data);
			sslog_ss_populate_individual(projectorService);
		}
	}

	/* TODO: when ProjectorService's property CURRENTPRESENTATION
	 * will not be NULL then we should use the following code
	 *
	 * if(projectorService != NULL)
		curPresentation = sslog_ss_get_property(projectorService, PROPERTY_CURRENTPRESENTATION);
	else
		return -1;

	if(curPresentation == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", "current presentation prop = NULL");
		return -1;
	}
	individual_t *presentation = (individual_t *)curPresentation->prop_value;



	if(sslog_ss_add_property(projectorNotif, PROPERTY_ENDPRESENTATION,  presentation) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", get_error_text());
		return -1;
	}*/

	// Get current timeslot
	prop_val_t *curTimeslotProp = sslog_ss_get_property(section, PROPERTY_CURRENTTIMESLOT);
	if(curTimeslotProp == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", "curTimeslotProp is NULL");
		return -1;
	}
	individual_t *curTimeslot = (individual_t *)curTimeslotProp->prop_value;

	// Get person of this timeslot
	prop_val_t *personLinkProp = sslog_ss_get_property(curTimeslot, PROPERTY_PERSONLINK);
	if(personLinkProp == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", "personLinkProp is NULL");
		return -1;
	}
	individual_t *person = (individual_t *)personLinkProp->prop_value;

	// Get person's presentation
	prop_val_t *presentationProp = sslog_ss_get_property(person, PROPERTY_PRESENTS);
	if(presentationProp == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", "presentationProp is NULL");
		return -1;
	}
	individual_t *presentation = (individual_t *)presentationProp->prop_value;

	individual_t *projectorNotif = sslog_new_individual(CLASS_PROJECTORNOTIFICATION);
	sslog_set_individual_uuid(projectorNotif, "http://www.cs.karelia.ru/smartroom#ProjectorNotification");

	individual_t *agendaNotif = sslog_new_individual(CLASS_AGENDANOTIFICATION);
	sslog_set_individual_uuid(agendaNotif, "http://www.cs.karelia.ru/smartroom#AgendaNotification");

	//if(presentation != NULL) {
		if(sslog_ss_add_property(projectorNotif, PROPERTY_ENDPRESENTATION, presentation) != 0 ) {
			__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", get_error_text());
			return -1;
		}

		if(sslog_ss_add_property(agendaNotif, PROPERTY_ENDPRESENTATION, curTimeslot) != 0 ) {
			__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", get_error_text());
			return -1;
		}

		if(sslog_ss_insert_individual(projectorNotif) != 0) {
			__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", get_error_text());
			return -1;
		}

		if(sslog_ss_insert_individual(agendaNotif) != 0) {
			__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", get_error_text());
			return -1;
		}
	//}

	return 0;
}
