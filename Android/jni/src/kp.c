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
	logout();
	sslog_sbcr_unsubscribe_all(true);
	sslog_ss_leave_session(sslog_get_ss_info());
	sslog_repo_clean_all();
}

/**
 * @brief Logging out user from Smart Space
 */
void logout() {
	prop_val_t *person_prop = sslog_ss_get_property(personProfile, PROPERTY_PERSONINFORMATION);
	individual_t *person;

	char *online_status = "online";
	char *offline_status = "offline";

	if(person_prop == NULL)
		return;

	person = (individual_t *)person_prop->prop_value;

	if(person == NULL)
		return;

	sslog_ss_update_property(person, PROPERTY_STATUS->name, (void *)online_status,
			(void *)offline_status);

	/*individual_t *agendaGui = sslog_new_individual(CLASS_AGENDAGUINOTIFICATION);

	sslog_set_individual_uuid(agendaGui,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));

	if(sslog_ss_add_property(agendaGui, PROPERTY_UPDATEAGENDA, firstTimeslot) != 0 )
		return;

	if(sslog_ss_insert_individual(agendaGui) != 0)
		return;*/
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
 * @return Returns 0 in success, 1 if username exists and -1 if failed
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_userRegistration
  (JNIEnv *env, jobject obj, jstring userName, jstring password) {

	list_t* profileList = sslog_ss_get_individual_by_class_all(CLASS_PROFILE);
	individual_t *profile = NULL;
	int personFounded = -1;

	if(profileList != NULL) {

		list_head_t* pos = NULL;
		list_for_each(pos, &profileList->links) {
			list_t* node = list_entry(pos, list_t, links);
			profile = (individual_t*)(node->data);
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

	if(usernameExists((*env)->GetStringUTFChars(env, userName, NULL)) == 0)
		return 1;

	profile = createProfile((*env)->GetStringUTFChars(env, userName, NULL),
			(*env)->GetStringUTFChars(env, password, NULL));

	if(profile == NULL)
		return -1;

	if(activatePerson(profile) != 0)
		return -1;

	return 0;
}

/**
 * @brief Creates new profile in Smart Space
 * @param userName
 * @param password
 * @return 0 in success and -1 otherwise
 */
individual_t* createProfile(char *username, char *password) {

	individual_t *profile = sslog_new_individual(CLASS_PROFILE);

	sslog_set_individual_uuid(profile, generateUuid("Profile"));

	if(sslog_ss_add_property(profile, PROPERTY_USERNAME, (void *)username) == -1)
		return NULL;

	if(sslog_ss_add_property(profile, PROPERTY_PASSWORD, (void *)password) == -1)
		return NULL;

	if(sslog_ss_add_property(profile, PROPERTY_PERSONINFORMATION, createPerson()) == -1)
			return NULL;

	if(sslog_ss_insert_individual(profile) == -1)
		return NULL;

	__android_log_print(ANDROID_LOG_INFO, "createProfile()", "Profile created");

	personProfile = profile;

	return profile;
}

/**
 * @brief Creates empty individual for person data
 * @return pointer to individual, NULL otherwise
 */
individual_t* createPerson() {

	individual_t *person = sslog_new_individual(CLASS_PERSON);

	if(person == NULL)
		return NULL;

	sslog_set_individual_uuid(person, generateUuid("Person"));

	return person;
}

/**
 * @brief Checks an existence of username in Smart Space
 *
 * @param username
 * @return 0 if exists and -1 otherwise
 */
int usernameExists(char *username) {
	list_t* profileList = sslog_ss_get_individual_by_class_all(CLASS_PROFILE);

	if(profileList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &profileList->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual_t* profile = (individual_t*)(node->data);
			sslog_ss_populate_individual(profile);

			prop_val_t *login = sslog_ss_get_property(profile, PROPERTY_USERNAME);

			if(login != NULL)
				if(strcmp(username, (char *)login->prop_value) == 0)
					return 0;
		}
	}

	return -1;
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
int searchPerson(individual_t *profile, char *userName, char *password) {

	prop_val_t *p_val_username = sslog_ss_get_property (profile, PROPERTY_USERNAME);

	if(p_val_username == NULL)
		return -1;

	prop_val_t *p_val_password = sslog_ss_get_property (profile, PROPERTY_PASSWORD);

	if(p_val_password == NULL)
		return -1;

	if((strcmp(userName, (char *)p_val_username->prop_value) == 0) &&
			(strcmp(password, (char *)p_val_password->prop_value) == 0)) {
		personProfile = profile;

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

	char *online_status = "online";
	char *offline_status = "offline";

	prop_val_t *person_prop = sslog_ss_get_property(profile, PROPERTY_PERSONINFORMATION);

	individual_t *person;

	if(person_prop == NULL)
		return -1;

	person = (individual_t *)person_prop->prop_value;

	sslog_ss_populate_individual(person);

	if(person == NULL)
		return -1;

	// if property does not exists
	if(sslog_ss_get_property(person, PROPERTY_STATUS) == NULL) {

		if(sslog_ss_add_property(person, PROPERTY_STATUS, (void *)online_status) != 0) {
			__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
			return -1;
		}

	} else {
		sslog_ss_update_property(person, PROPERTY_STATUS->name, (void *)offline_status,
				(void *)online_status);
	}

	individual_t *agendaGui = sslog_new_individual(CLASS_AGENDANOTIFICATION);

	sslog_set_individual_uuid(agendaGui,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));

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
	int counter = 0, attempts = 3;

	if(obj != NULL)
		agendaClassObject = (jobject *)(*env)->NewGlobalRef(env, obj);
	else
		return -1;

	prop_val_t *propTimeslot = sslog_ss_get_property(section, PROPERTY_FIRSTTIMESLOT);
	for(; counter < attempts; counter++) {
		if(propTimeslot == NULL)
			propTimeslot = sslog_ss_get_property(section, PROPERTY_FIRSTTIMESLOT);
		else
			break;
	}

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
			"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

	int counter = 0, attempts = 3;
	prop_val_t *p_val_name = sslog_ss_get_property (timeslot, PROPERTY_SPEAKERNAME);
	prop_val_t *p_val_pres_title = sslog_ss_get_property (timeslot, PROPERTY_PRESENTATIONTITLE);

	for(; counter < attempts; counter++) {

		if(p_val_name == NULL)
			p_val_name = sslog_ss_get_property (timeslot, PROPERTY_SPEAKERNAME);

		if(p_val_pres_title == NULL)
			p_val_pres_title = sslog_ss_get_property (timeslot, PROPERTY_PRESENTATIONTITLE);

		if((p_val_name != NULL)	&& (p_val_pres_title != NULL))
			break;
	}


	/* Gets person link property */
	char *imgLink = (char *) malloc (sizeof(char) * 200);

	strcpy(imgLink, "absentImage");

	prop_val_t *p_val_person_link = sslog_ss_get_property (timeslot, PROPERTY_PERSONLINK);
	if(p_val_person_link != NULL) {

		individual_t *person = (individual_t *)p_val_person_link->prop_value;

		prop_val_t *p_val_img = sslog_ss_get_property (person, PROPERTY_IMG);

		if(p_val_img != NULL) {
			strcpy(imgLink, (char *)p_val_img->prop_value);
		}
	}

	/* Calling Agenda's addTimeslotItemToList Java method */
	if(obj != NULL)
		(*env)->CallVoidMethod(env, obj, methodId,
				(*env)->NewStringUTF(env, (char *)(p_val_name->prop_value)),
				(*env)->NewStringUTF(env, (char *)(p_val_pres_title->prop_value)),
				(*env)->NewStringUTF(env, imgLink));

	free(imgLink);
}


/***********************
 *
 * Generates unique uuid
 *
 ***********************/
char* generateUuid(char *uuid) {

	int rand_val = 0, rand_length = 1, i = 0, postfix_length = 6;
	char *result = (char*) malloc (sizeof(char) * strlen(uuid) + postfix_length + 2);

	for(; i < postfix_length; rand_length *= 10, i++);

	do {
		srand(time(NULL));
		rand_val = rand() % rand_length;
		sprintf(result, "%s-%d", uuid, rand_val);
	} while(sslog_ss_exists_uuid(result) == 1);

	__android_log_print(ANDROID_LOG_ERROR, "uuid gen", result);

	return result;
}


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
		}

		list_t *listProperties = list_get_new_list();
		list_add_data(PROPERTY_CURRENTSLIDENUM, listProperties);

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
	bool attached = JNI_FALSE;

	switch((*JVM)->GetEnv(JVM, (void **)&env, JNI_VERSION_1_6)) {

		case JNI_OK:
			break;

		case JNI_EDETACHED:
			(*JVM)->AttachCurrentThread(JVM, &env, NULL);
			attached = JNI_TRUE;
			break;

		case JNI_EVERSION:
			__android_log_print(ANDROID_LOG_ERROR, "agendaHandler:",
					"invalid jni version");
			break;
	}

	jmethodID updateAgenda = (*env)->GetMethodID(env, classAgenda, "updateAgenda", "()V");
	jmethodID setCurTimeslot = (*env)->GetMethodID(env, classAgenda, "setCurrentTimeslot", "(I)V");

	jfieldID agendaCreated = (*env)->GetStaticFieldID(env, classAgenda,
			"agendaCreated", "I");
	int agendaIsCreated = (*env)->GetStaticIntField(env, classAgenda, agendaCreated);

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);
	list_t *list = sslog_sbcr_ch_get_individual_all(changes);

	if(agendaIsCreated != 1) {
		if(attached)
			(*JVM)->DetachCurrentThread(JVM);

		return;
	}

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
				if(agendaIsCreated == 1)
					(*env)->CallVoidMethod(env, agendaClassObject, updateAgenda);
			}

			prop_val_t *p_val_timeslot = sslog_ss_get_property (individual,
									PROPERTY_CURRENTTIMESLOT);
			if(p_val_timeslot != NULL) {

				calculateTimeslotIndex(p_val_timeslot);

				if(agendaIsCreated == 1)
					(*env)->CallVoidMethod(env, agendaClassObject, updateAgenda);
				else
					break;
			}
		}
	}

	list_free_with_nodes(list, NULL);

	if(attached)
		(*JVM)->DetachCurrentThread(JVM);
}


JNIEXPORT jint JNICALL Java_com_example_srclient_KP_getCurrentTimeslotIndex
  (JNIEnv *env, jclass clazz) {
	return calculateTimeslotIndex(NULL);
}

/**
 * @brief Calculates an index number of current timeslot
 */
int calculateTimeslotIndex(prop_val_t *propTimeslot) {

	individual_t *curTimeslot;
	individual_t *timeslot = firstTimeslot;
	currentTimeslotIndex = 1;

	if(propTimeslot != NULL)
		curTimeslot = (individual_t *) propTimeslot->prop_value;

	else {
		propTimeslot = sslog_ss_get_property (section,
				PROPERTY_CURRENTTIMESLOT);

		if(propTimeslot != NULL)
			curTimeslot = (individual_t *) propTimeslot->prop_value;
		else
			return -1;
	}

	// Get index of current timeslot
	while(strcmp(curTimeslot->uuid, timeslot->uuid) != 0) {
		prop_val_t *val = sslog_ss_get_property(timeslot, PROPERTY_NEXTTIMESLOT);

		if(val != NULL) {
			timeslot = (individual_t *) val->prop_value;
			++currentTimeslotIndex;
		} else return -1;
	}

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
}



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
	jmethodID updateProjector = (*env)->GetMethodID(env, classProjector, "updateProjector", "()I");

	jfieldID projectorCreated = (*env)->GetStaticFieldID(env, classProjector,
			"projectorCreated", "I");

	int projectorIsCreated = (*env)->GetStaticIntField(env,
			classProjector, projectorCreated);

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);
		list_t *list = sslog_sbcr_ch_get_individual_all(changes);

	if(projectorIsCreated != 1) {
		if(attached)
			(*JVM)->DetachCurrentThread(JVM);

		return;
	}

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

	(*env)->CallIntMethod(env, projectorClassObject, updateProjector);

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

	int ret_val = 0;
	individual_t *individual = sslog_new_individual(CLASS_AGENDANOTIFICATION);

	if(individual == NULL)
		return -1;

	ret_val = sslog_set_individual_uuid(individual,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));
	__android_log_print(ANDROID_LOG_ERROR, "startConference()", "here4");

	if(ret_val != ERROR_NO)
		return -1;

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

	int ret_val = 0;
	individual_t *individual = sslog_new_individual(CLASS_AGENDANOTIFICATION);

	if(individual == NULL)
		return -1;

	ret_val = sslog_set_individual_uuid(individual,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));

	if(ret_val != ERROR_NO)
		return -1;

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

	int ret_val = 0;
	char number[] = "0";
	sprintf(number, "%d", (int)slideNumber);

	individual_t *individual = sslog_new_individual(CLASS_PROJECTORNOTIFICATION);
	if(individual == NULL)
		return -1;

	ret_val = sslog_set_individual_uuid(individual,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));

	if(ret_val != ERROR_NO)
		return -1;

	if(sslog_ss_add_property(individual, PROPERTY_SHOWSLIDE,  (void *)number) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "showSlide()", get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "showSlide()", get_error_text());
		return -1;
	}

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
	individual_t *agendaNotif = sslog_new_individual(CLASS_AGENDANOTIFICATION);

	//if(presentation != NULL) {
		sslog_set_individual_uuid(projectorNotif,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));

		if(sslog_ss_add_property(projectorNotif, PROPERTY_ENDPRESENTATION, presentation) != 0 ) {
			__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", get_error_text());
			return -1;
		}

		if(sslog_ss_insert_individual(projectorNotif) != 0) {
			__android_log_print(ANDROID_LOG_ERROR, "endPresentation()", get_error_text());
			return -1;
		}

		sslog_set_individual_uuid(agendaNotif,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));

		if(sslog_ss_add_property(agendaNotif, PROPERTY_ENDPRESENTATION, curTimeslot) != 0 ) {
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

JNIEXPORT jstring JNICALL Java_com_example_srclient_KP_getMicServiceIP
  (JNIEnv *env, jclass clazz) {

	list_t *list = sslog_ss_get_individual_by_class_all(CLASS_MICROPHONESERVICE);
	individual_t *individual;

	if(list != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &list->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual = (individual_t*)(node->data);
			sslog_ss_populate_individual(individual);
		}
	} else
		return NULL;

	prop_val_t *ip_value = sslog_ss_get_property(individual, PROPERTY_IP);

	if(ip_value == NULL)
		return NULL;

	return (*env)->NewStringUTF(env, (char *)ip_value->prop_value);
}

JNIEXPORT jstring JNICALL Java_com_example_srclient_KP_getMicServicePort
  (JNIEnv *env, jclass clazz) {

	list_t *list = sslog_ss_get_individual_by_class_all(CLASS_MICROPHONESERVICE);
	individual_t *individual;

	if(list != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &list->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual = (individual_t*)(node->data);
			sslog_ss_populate_individual(individual);
		}
	} else
		return NULL;

	prop_val_t *port_value = sslog_ss_get_property(individual, PROPERTY_PORT);

	if(port_value == NULL)
		return NULL;

	return (*env)->NewStringUTF(env, (char *)port_value->prop_value);
}

/**
 * @brief Returns current speaker name
 */
JNIEXPORT jstring JNICALL Java_com_example_srclient_KP_getSpeakerName
  (JNIEnv *env, jclass clazz) {

	prop_val_t *curTimeslotProp = sslog_ss_get_property(section, PROPERTY_CURRENTTIMESLOT);

	if(curTimeslotProp == NULL)
		return "";

	individual_t *curTimeslot = (individual_t *)curTimeslotProp->prop_value;

	prop_val_t *personLinkProp = sslog_ss_get_property(curTimeslot, PROPERTY_PERSONLINK);
	if(personLinkProp == NULL)
		return -1;

	individual_t *person = (individual_t *)personLinkProp->prop_value;
	prop_val_t *personName = sslog_ss_get_property(person, PROPERTY_NAME);

	if(personName != NULL)
		return (*env)->NewStringUTF(env, (char *)personName->prop_value);

	else
		return "";
}
