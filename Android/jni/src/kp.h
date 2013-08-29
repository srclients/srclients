#ifndef _KP_H_
#define _KP_H_

#include "SmartRoomOntology.h"
#include <stdlib.h>
#include <errno.h>
#include <jni.h>
#include <string.h>

#define MAX_CLASS_NAME_LENGTH 20
#define ERROR_MSG_LENGTH 40


JavaVM* JVM;

individual_t *firstTimeslot;
individual_t *section;
individual_t *personProfile;

int currentTimeslotIndex;

jclass *classAgenda;
jclass *classProjector;
jclass *classKP;
jobject *agendaClassObject;
jobject *projectorClassObject;
jobject *kpClassObject;


void addTimeslotToJavaList(JNIEnv *, individual_t *, jobject);
prop_val_t* initNullProperty();
char* generateUuid(char*);
jclass getJClassObject(JNIEnv *, char *);
jfieldID getFieldID(JNIEnv *, jclass, char *, char *);
int searchPerson(individual_t *, char *, char *);
int activatePerson(individual_t *);
void subscriptionHandler(subscription_t *);
void projectorNotificationHandler(subscription_t *);
void agendaNotificationHandler(subscription_t *);
individual_t * createProfile(char *, char *);
individual_t* createPerson();
void logout();
int calculateTimeslotIndex(prop_val_t *);


#endif
