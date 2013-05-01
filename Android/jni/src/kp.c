#include "com_example_srclient_KP.h"
#include "kp.h"
#include <android/log.h>


/*********************
 *
 * Joining Smart Space
 *
 *********************/
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

	return 0;
}/* Joining Smart Space */


/*********************
 *
 * Leaving Smart Space
 *
 *********************/
JNIEXPORT void JNICALL Java_com_example_srclient_KP_disconnectSmartSpace
  (JNIEnv *env, jobject obj) {

	sslog_ss_leave_session(sslog_get_ss_info());
	sslog_repo_clean_all();
}/* Leaving Smart Space */


/******************************
 *
 * Register user in Smart Space
 *
 ******************************/
JNIEXPORT int JNICALL Java_com_example_srclient_KP_userRegistration
  (JNIEnv *env, jobject obj, jstring userName, jstring password) {

	list_t* profileList = sslog_ss_get_individual_by_class_all(CLASS_PROFILE);
	int personFounded = -1;

	if(profileList == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP",
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
				__android_log_print(ANDROID_LOG_ERROR, "class KP",
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
}/* Register user in Smart Space */


/********************************************
 *
 * Search person by `username` and `password`
 *
 ********************************************/
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
}/* Search person by `username` and `password` */


/**************************************************
 *
 * Send to SS `newParticipantCome`
 *
 **************************************************/
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
}/* Send to SS `newParticipantCome` */


/***************************************
 *
 * Initialization of Timeslot for Agenda
 *
 ***************************************/
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


/*****************************************
 *
 * Addition individual info to java object
 *
 *****************************************/
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


/***************************************************
 *
 * Initializes subscription to conference activities
 *
 ***************************************************/
int initSubscription() {

	subscription_t *subscription = sslog_new_subscription(true);

	list_t *propertiesList = list_get_new_list();
	list_add_data(PROPERTY_NEWPARTICIPANTCOME, propertiesList);

	//sslog_sbcr_add_individual(subscription, );

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

				// TODO: slideImg empty !!!
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


