#ifndef _KP_H_
#define _KP_H_

#include "SmartRoomOntology.h"
#include <stdlib.h>
#include <errno.h>

#define MAX_CLASS_NAME_LENGTH 20
#define ERROR_MSG_LENGTH 40


void addTimeslotToJavaList(JNIEnv *, individual_t *, jobject);
prop_val_t* initNullProperty();
char* generateUuid(char*);
jclass getJClassObject(JNIEnv *, char *);
jfieldID getFieldID(JNIEnv *, jclass, char *, char *);
int searchPerson(individual_t *, char *, char *);
int activatePerson(individual_t *);

#endif
