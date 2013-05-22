#ifndef _KP_H_
#define _KP_H_

#include "SmartRoomOntology.h"
#include <stdlib.h>
#include <errno.h>
#include <jni.h>

#define MAX_CLASS_NAME_LENGTH 20
#define ERROR_MSG_LENGTH 40


JavaVM* JVM;

individual_t *firstTimeslot;
individual_t *section;

jclass *classAgenda;
jclass *classProjector;
jobject *agendaClassObject;
jobject *projectorClassObject;


void addTimeslotToJavaList(JNIEnv *, individual_t *, jobject);
prop_val_t* initNullProperty();
char* generateUuid(char*);
jclass getJClassObject(JNIEnv *, char *);
jfieldID getFieldID(JNIEnv *, jclass, char *, char *);
int searchPerson(individual_t *, char *, char *);
int activatePerson(individual_t *);
void agendaGUINotifHandler(subscription_t *);
void projectorNotifHandler(subscription_t *);

#endif
