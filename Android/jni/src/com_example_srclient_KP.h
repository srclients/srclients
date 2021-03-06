/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_example_srclient_KP */

#ifndef _Included_com_example_srclient_KP
#define _Included_com_example_srclient_KP
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_com_example_srclient_KP_connectSmartSpace
  (JNIEnv *, jobject, jstring, jstring, jint);

/*
 * Class:     com_example_srclient_KP
 * Method:    loadTimeslotList
 * Signature: (Lcom/example/srclient/Agenda;)I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_loadTimeslotList
  (JNIEnv *, jclass, jobject);

/*
 * Class:     com_example_srclient_KP
 * Method:    disconnectSmartSpace
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_example_srclient_KP_disconnectSmartSpace
  (JNIEnv *, jobject);

/*
 * Class:     com_example_srclient_KP
 * Method:    publishPersonInfo
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_publishPersonInfo
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     com_example_srclient_KP
 * Method:    getServicesInfo
 * Signature: ()[C
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_getServicesInfo
  (JNIEnv *, jclass, jobject);

/*
 * Class:     com_example_srclient_KP
 * Method:    userRegistartion
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_userRegistration
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_example_srclient_KP
 * Method:    loadPresentation
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_loadPresentation
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_example_srclient_KP
 * Method:    startConference
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_startConference
  (JNIEnv *, jobject);

/*
 * Class:     com_example_srclient_KP
 * Method:    initSubscription
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_initSubscription
  (JNIEnv *, jobject);

/*
 * Class:     com_example_srclient_KP
 * Method:    endConference
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_endConference
  (JNIEnv *, jobject);

/*
 * Class:     com_example_srclient_KP
 * Method:    getProjectorClassObject
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_example_srclient_KP_getProjectorClassObject
  (JNIEnv *, jclass, jobject);


/*
 * Class:     com_example_srclient_KP
 * Method:    showSlide
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_showSlide
  (JNIEnv *, jclass, jint);


/*
 * Class:     com_example_srclient_KP
 * Method:    endPresentation
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_endPresentation
  (JNIEnv *, jclass);

/*
 * Class:     com_example_srclient_KP
 * Method:    getCurrentTimeslotIndex
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_srclient_KP_getCurrentTimeslotIndex
  (JNIEnv *, jclass);

/*
 * Class:     com_example_srclient_KP
 * Method:    checkSpeakerState
 * Signature: ()I
 */
JNIEXPORT jboolean JNICALL Java_com_example_srclient_KP_checkSpeakerState
  (JNIEnv *, jclass);

/*
 * Class:     com_example_srclient_KP
 * Method:    getMicServiceIP
 * Signature: ()I
 */
JNIEXPORT jstring JNICALL Java_com_example_srclient_KP_getMicServiceIP
  (JNIEnv *, jclass);

/*
 * Class:     com_example_srclient_KP
 * Method:    getMicServicePort
 * Signature: ()I
 */
JNIEXPORT jstring JNICALL Java_com_example_srclient_KP_getMicServicePort
  (JNIEnv *, jclass);


/*
 * Class:     com_example_srclient_KP
 * Method:    getSpeakerName
 * Signature: ()I
 */
JNIEXPORT jstring JNICALL Java_com_example_srclient_KP_getSpeakerName
  (JNIEnv *, jclass);


#ifdef __cplusplus
}
#endif
#endif
