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
				activatePerson(profile);
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

	if(sslog_ss_update_property(person, PROPERTY_STATUS->name, "offline", "online") == -1)
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());

	individual_t *agendaGui = sslog_new_individual(CLASS_AGENDANOTIFICATION);

	sslog_set_individual_uuid(agendaGui, generateUuid("agenda-notification"));

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

	list_t* timeslots = sslog_ss_get_individual_by_class_all(CLASS_TIMESLOT);
	agendaClassObject = (jobject *)(*env)->NewGlobalRef(env, obj);

	if (timeslots == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP",
				"Can't find Timeslot individual");
		return -1;

	} else {
		list_head_t* pos = NULL;
		list_for_each(pos, &timeslots->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual_t* timeslot = (individual_t*)(node->data);
			sslog_ss_populate_individual(timeslot);

			individual_t* individual =
					sslog_repo_get_individual_by_uuid(timeslot->uuid);


			if(individual != NULL)
				addTimeslotToJavaList(env, individual, obj);
			else {
				__android_log_print(ANDROID_LOG_ERROR, "class KP", "Can't find individual by uuid");
				return -1;
			}
		}
	}

	return 0;
} /* Initialization of Timeslot for Agenda */


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

	if(methodId == 0)
	{
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "No such method");
		return;
	}


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

	strcpy(imgLink, "http://upload.wikimedia.org/wikipedia/commons/3/36/Bonhomme_crystal_marron.png");

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

}/* Addition individual info to java object */


/*******************************************
 *
 * TODO: Loads participant's info from smart space
 *
 *******************************************/
int loadPersonInfo(void *property) {
	individual_t *person = (individual_t*)property;

	sslog_ss_populate_individual(person);
	if(person == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Can't populate person individual");
		return -1;
	}

	prop_val_t *p_val_name = sslog_ss_get_property (person, PROPERTY_NAME);
	if(p_val_name == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Property name not found");
		p_val_name = initNullProperty();
	}

	return 0;
}/* Loads participant's info from smart space */


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
	int postfix_length = 3;
	char *tmp = (char*) malloc (sizeof(char) * strlen(uuid) + postfix_length);
	char *rand_chr = (char*) calloc (postfix_length, sizeof(char));
	int rand_val;

	do {
		srand(time(NULL));
		rand_val = rand() % 1000;
		snprintf(rand_chr, strlen(rand_chr), "%d", rand_val);
		strcpy(tmp, uuid);
		strcat(tmp, "-");
		strcat(tmp, rand_chr);
	} while(sslog_ss_exists_uuid(tmp) == 1);

	free(rand_chr);

	return tmp;
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


	individual_t *agenda = sslog_ss_get_individual_by_class_all(CLASS_AGENDA);

	if(agenda == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Agenda service not found");
		(*env)->SetIntField(env, menu, agendaStateField, 0);
	} else
		(*env)->SetIntField(env, menu, agendaStateField, 1);


	individual_t *projector = sslog_ss_get_individual_by_class_all(CLASS_PROJECTORSERVICE);

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

	classAgenda = (jclass *)(*env)->NewGlobalRef(env, classAgendaObj);
	classProjector = (jclass *)(*env)->NewGlobalRef(env, classProjectorObj);


	/* Subscription to ProjectorService class */
	extern void projectorNotifHandler(subscription_t *);
	void (*pProjectorHandler)(subscription_t *) = &projectorNotifHandler;

	subscription_t *subscriptionProjector = sslog_new_subscription(true);

	sslog_sbcr_add_class(subscriptionProjector, CLASS_PROJECTORSERVICE);
	sslog_sbcr_set_changed_handler(subscriptionProjector, pProjectorHandler);

	if(sslog_sbcr_subscribe(subscriptionProjector) != ERROR_NO) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}


	/* Subscription to AgendaGuiNotif class */
	extern void agendaGUINotifHandler(subscription_t *);
	void (*pAgendaGuiHandler)(subscription_t *) = &agendaGUINotifHandler;

	subscription_t *subscriptionAgendaGui = sslog_new_subscription(true);

	sslog_sbcr_add_class(subscriptionAgendaGui, CLASS_AGENDAGUINOTIFICATION);
	sslog_sbcr_set_changed_handler(subscriptionAgendaGui, pAgendaGuiHandler);

	if(sslog_sbcr_subscribe(subscriptionAgendaGui) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Handles agenda changes
 */
void agendaGUINotifHandler(subscription_t *sbcr) {
	__android_log_print(ANDROID_LOG_ERROR, "agendaGUINotifHandler():", "IS RUNNING...");

	/* Gets jni environment pointer (env) */
	JNIEnv *env = NULL;
	(*JVM)->AttachCurrentThread(JVM, &env, NULL);

	jmethodID methodId = (*env)->GetMethodID(env, classAgenda, "updateAgenda", "()V");

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);

	/* All updated individuals */
	const list_t *list = sslog_sbcr_ch_get_individual_by_action(changes, ACTION_INSERT);

	if(list != NULL) {
		list_head_t *list_walker = NULL;

		/* For every updated individual  */
		list_for_each(list_walker, &list->links) {
			list_t *node = list_entry(list_walker, list_t, links);
			char *uuid = (char *) node->data;

			/* All updated properties with individual's `uuid` */
			list_t *p_list = sslog_sbcr_ch_get_property_by_uuid(changes, uuid, ACTION_INSERT);
			list_head_t *list_walker_prop = NULL;

			if(p_list == NULL) {
				__android_log_print(ANDROID_LOG_ERROR, "AgendaGui Handler:", "p_list is empty");
				continue;
			}

			/* `updateAgenda()` class Agenda */
			if(changes != NULL)
				(*env)->CallVoidMethod(env, agendaClassObject, methodId);

			/* For every changed property */
			list_for_each(list_walker_prop, &p_list->links) {
				__android_log_print(ANDROID_LOG_ERROR, "AgendaGui Handler:", "here it is");
				list_t *node_prop = list_entry(list_walker_prop, list_t, links);
				property_changes_data_t *ch_prop;

				ch_prop = (property_changes_data_t *)(node_prop->data);

				//__android_log_print(ANDROID_LOG_ERROR, "AgendaGui Handler:", ch_prop->property->name);

				if(strcmp(ch_prop->property->name, PROPERTY_UPDATEAGENDA->name) == 0)
					(*env)->CallVoidMethod(env, agendaClassObject, methodId);
			}

			list_free_with_nodes(p_list, NULL);
			break;
		}
	} else
		__android_log_print(ANDROID_LOG_ERROR, "AgendaGui Handler:", "insert list is empty");
}


/********************************************************
 *
 * Loads presentation with `uuid` or current presentation
 *
 ********************************************************/
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_loadPresentation
  (JNIEnv *env, jobject obj, jstring stringUuid, jobject projectorClassObject) {

	const char *uuid = (*env) -> GetStringUTFChars(env, stringUuid, NULL);
	char *slideImg = (char *) malloc (sizeof(char) * 200);
	individual_t *projectorService;

	jmethodID methodId = (*env)->GetMethodID(env, classProjector, "setImageLink",
				"(Ljava/lang/String;)V");

	if(methodId == 0)
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "methodId is NULL");

	// TODO: add extraction of `currentSlideCount` `currentSlideNum`

	/* load current presentation */
	if(strcmp(uuid, "") == 0) {
		list_t* projectorServiceList = sslog_ss_get_individual_by_class_all(CLASS_PROJECTORSERVICE);

			if(projectorServiceList != NULL) {
				list_head_t* pos = NULL;
				list_for_each(pos, &projectorServiceList->links) {
					list_t* node = list_entry(pos, list_t, links);
					projectorService = (individual_t*)(node->data);
					sslog_ss_populate_individual(projectorService);
					break;
				}

				prop_val_t *p_val_slideImg = sslog_ss_get_property (projectorService,
						PROPERTY_CURRENTSLIDEIMG);
				if(p_val_slideImg != NULL) {
					strcpy(slideImg, (char *)p_val_slideImg->prop_value);
				} else {
					__android_log_print(ANDROID_LOG_ERROR, "class KP", "slide image empty");
				}
			}
	} else {
		/* load presentation by `uuid` */
		// TODO
	}


	(*env)->CallVoidMethod(env, projectorClassObject, methodId,
			(*env)->NewStringUTF(env, slideImg));


	free(slideImg);

	return 0;
}/* Loads presentation with `uuid` */



/**
 * @brief Handles changes happened with `CLASS_PROJECTORNOTIFICATION`
 *
 * @param sbcr
 */
void projectorNotifHandler(subscription_t *sbcr) {

	JNIEnv *env;
	// TODO: start from here !!!
	/* Gets jni environment pointer (env) */
	int res = (*JVM)->AttachCurrentThread(JVM, &env, NULL);
	if(res < 0) {
		__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "Attach failed");
	}

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);

	__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "is running...");


	if(classProjector != NULL) {
		/* Get Projector class fields */
		jfieldID slideNumberField = getFieldID(env, classProjector, "slideNumber", "I");
		jfieldID slideCountField = getFieldID(env, classProjector, "slideCount", "I");
	}

	jmethodID methodId = (*env)->GetMethodID(env, classProjector, "setImageLink",
					"(Ljava/lang/String;)V");


	/* All updated individuals */
	const list_t *list = sslog_sbcr_ch_get_individual_by_action(changes, ACTION_INSERT);
	if(list == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "INSERT list is empty");

	list_head_t *list_walker = NULL;


}

/**
 * @brief Gets existing projector class object
 **/
JNIEXPORT void JNICALL Java_com_example_srclient_KP_getProjectorClassObject
  (JNIEnv *env, jclass clazz, jobject obj) {

	projectorClassObject = (jobject *)(*env)->NewGlobalRef(env, obj);
}


/**
 * @brief Starts conference
 *
 * @return 0 in success, -1 otherwise
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_startConference
  (JNIEnv *env, jobject obj) {

	list_t *sectionList = sslog_ss_get_individual_by_class_all(CLASS_SECTION);

	if(sectionList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &sectionList->links) {
			list_t* node = list_entry(pos, list_t, links);
			section = (individual_t*)(node->data);
			sslog_ss_populate_individual(section);
			break;
		}
	}

	if(section == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "startConference()", "Section is NULL");

	prop_val_t *firstTimeslotProp = sslog_ss_get_property(section, PROPERTY_FIRSTTIMESLOT);

	if(firstTimeslotProp != NULL)
		firstTimeslot = (individual_t *)firstTimeslotProp->prop_value;

	individual_t *individual = sslog_new_individual(CLASS_AGENDANOTIFICATION);
	sslog_set_individual_uuid(individual, generateUuid("agenda-gui-notification"));

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
	sslog_set_individual_uuid(individual, generateUuid("agenda-gui-notification"));

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
