#include "com_example_srclient_KP.h"
#include "kp.h"
#include <android/log.h>

static JavaVM* JVM;

individual_t *firstTimeslot;
individual_t *section;


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
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Can't join Smart Space");
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
JNIEXPORT int JNICALL Java_com_example_srclient_KP_userRegistration
  (JNIEnv *env, jobject obj, jstring userName, jstring password) {

	//list_t* profileList = sslog_ss_get_individual_by_class_all(CLASS_PROFILE);
	list_t* profileList = sslog_repo_get_individual_by_classtype_all(CLASS_PROFILE->classtype);
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

	jclass class = getJClassObject(env, "Agenda");

	jmethodID methodId = (*env)->GetMethodID(env, class, "addTimeslotItemToList",
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

		prop_val_t *p_val_img = sslog_ss_get_property (person, PROPERTY_IMG);

		if(p_val_img != NULL) {
			strcpy(imgLink, (char *)p_val_img->prop_value);
		}
	}

	/* Calling Agenda's addTimeslotItemToList Java method */
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
	char *rand_chr = (char*) malloc (sizeof(char) * postfix_length);
	int rand_val;

	do {
		srand(time(NULL));
		rand_val = rand() % 1000;
		sprintf(rand_chr, "%d", rand_val);
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
JNIEXPORT int JNICALL Java_com_example_srclient_KP_getServicesInfo
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
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "Can't find ServicesMenu class");
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
JNIEXPORT int JNICALL Java_com_example_srclient_KP_initSubscription
  (JNIEnv *env, jobject obj) {


	/* Subscription to ProjectorService class */
	extern void projectorNotifHandler(subscription_t *);
	void (*pProjectorHandler)(subscription_t *) = &projectorNotifHandler;

	subscription_t *subscriptionProjector = sslog_new_subscription(true);

	individual_t *projectorService = sslog_new_individual(CLASS_PROJECTORSERVICE);

	if(projectorService == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());

	sslog_set_individual_uuid(projectorService, "projector-service");

	list_t *propListProjector = list_get_new_list();

	list_add_data(PROPERTY_CURRENTSLIDEIMG, propListProjector);
	list_add_data(PROPERTY_CURRENTSLIDENUM, propListProjector);
	list_add_data(PROPERTY_CURRENTSLIDECOUNT, propListProjector);

	if(sslog_sbcr_add_individual(subscriptionProjector, projectorService, propListProjector) != ERROR_NO)
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());

	sslog_sbcr_set_changed_handler(subscriptionProjector, pProjectorHandler);

	if(sslog_sbcr_subscribe(subscriptionProjector) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}


	/* Subscription to AgendaService class */
	extern void agendaNotifHandler(subscription_t *);
	void (*pAgendaHandler)(subscription_t *) = &agendaNotifHandler;

	subscription_t *subscriptionAgenda = sslog_new_subscription(true);

	sslog_sbcr_add_class(subscriptionAgenda, CLASS_AGENDASERVICE);
	sslog_sbcr_set_changed_handler(subscriptionAgenda, pAgendaHandler);

	if(sslog_sbcr_subscribe(subscriptionAgenda) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP", get_error_text());
		return -1;
	}

	return 0;
}


/********************************************************
 *
 * Loads presentation with `uuid` or current presentation
 *
 ********************************************************/
JNIEXPORT int JNICALL Java_com_example_srclient_KP_loadPresentation
  (JNIEnv *env, jobject obj, jstring stringUuid, jobject projectorClassObject) {

	const char *uuid = (*env) -> GetStringUTFChars(env, stringUuid, NULL);
	char *slideImg = (char *) malloc (sizeof(char) * 200);
	individual_t *projectorService;

	jclass class = getJClassObject(env, "Projector");

	jmethodID methodId = (*env)->GetMethodID(env, class, "setImageLink",
				"(Ljava/lang/String;)V");

	if(methodId == 0)
		__android_log_print(ANDROID_LOG_ERROR, "class KP", "methodId is NULL");


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
 * Todo: hanlde projector notifications
 * @param sbcr
 */
void projectorNotifHandler(subscription_t *sbcr) {

	JNIEnv *env;

	/* Gets jni environment pointer (env) */
	(*JVM)->AttachCurrentThread(JVM, &env, NULL);

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);
	individual_t *presentationSlide;

	__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "is running...");


	/* Get Projector class and it's fields */
	jclass class = getJClassObject(env, "Projector");

	jfieldID objectField = getFieldID(env, class, "thisObject", "Lcom/example/srclient/Projector;");
	jfieldID slideNumberField = getFieldID(env, class, "slideNumber", "I");
	jfieldID slideCountField = getFieldID(env, class, "slideCount", "I");


	jmethodID methodId = (*env)->GetMethodID(env, class, "setImageLink",
					"(Ljava/lang/String;)V");


	const list_t *list = sslog_sbcr_ch_get_individual_by_action(changes, ACTION_UPDATE);

	if(list == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "list is empty");

	list_head_t *list_walker = NULL;

	list_for_each(list_walker, &list->links) {
		list_t *node = list_entry(list_walker, list_t, links);
		char *uuid = (char *) node->data;

		list_t *p_list = sslog_sbcr_ch_get_property_by_uuid_all(changes, uuid);
		presentationSlide = (individual_t *)(p_list->data);
		list_free_with_nodes(p_list, NULL);
		break;
	}

	if(presentationSlide == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "presentation slide is NULL");

	/* Get current image */
	prop_val_t *p_val_slideImg = sslog_ss_get_property (presentationSlide,
			PROPERTY_CURRENTSLIDEIMG);

	if(p_val_slideImg == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "slide image is null");
	else
		(*env)->CallVoidMethod(env, objectField, methodId,
					(*env)->NewStringUTF(env, (char *)p_val_slideImg->prop_value));


	/* Get current slide number */
	prop_val_t *p_val_slideNum = sslog_ss_get_property (presentationSlide,
				PROPERTY_CURRENTSLIDENUM);

	if(p_val_slideNum == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "slide number is null");
	else
		(*env)->SetIntField(env, objectField, slideNumberField, (int)p_val_slideNum->prop_value);


	/* Get current slide count */
	prop_val_t *p_val_slideCount = sslog_ss_get_property (presentationSlide,
				PROPERTY_CURRENTSLIDECOUNT);

	if(p_val_slideCount == NULL)
		__android_log_print(ANDROID_LOG_ERROR, "Projector Handler:", "slide count is null");
	else
		(*env)->SetIntField(env, objectField, slideCountField, (int)p_val_slideCount->prop_value);

}


/**
 * Todo: hanlde agenda notifications
 * @param
 */
void agendaNotifHandler(subscription_t *sbcr) {
	__android_log_print(ANDROID_LOG_ERROR, "Agenda Handler:", "is running...");
}


/**
 * @brief Starts conference
 *
 * @return 0 in success, -1 otherwise
 */
JNIEXPORT int JNICALL Java_com_example_srclient_KP_startConference
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
JNIEXPORT int JNICALL Java_com_example_srclient_KP_endConference
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
