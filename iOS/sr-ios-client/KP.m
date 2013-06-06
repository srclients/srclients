//
//  KP.m
//  sr-ios-client
//
//  Created by Alexander on 21.04.13.
//  Copyright (c) 2013 alexkataev. All rights reserved.
//

#import "KP.h"
//#import "generic.h"
//#import "ss_multisib.h"

//#import "kp_error.h"


@implementation KP

-(BOOL)isInternetAvailableNow {
    
    Boolean isInternetAvailable = FALSE;
    
    NetworkStatus netStatus = [[Reachability reachabilityForInternetConnection] currentReachabilityStatus];
    
    switch (netStatus) {
        case NotReachable: {
            isInternetAvailable = FALSE;
            break;
        }
            
        case ReachableViaWWAN: {
            //label.text = @"Reachable WWAN";
            break;
        }
        case ReachableViaWiFi: {
            isInternetAvailable = TRUE;
            break;
        }
    }
    
    return isInternetAvailable;
}

//Connect to SmartSpace
-(int)connectToSmartSpaceOnIp:(char *)ip
                       ToPort:(int)port
                 WithHostname:(char *)hostname {
    
    if([self isInternetAvailableNow] == FALSE) {
        NSLog(@"No wi-fi connection!");
        return -1;
    }

    const char *uuid = [self generateUuid:"KP"];

    if(sslog_ss_init_session_with_parameters(hostname, ip, port) != 0) {
        NSLog(@"Connection problem");
        return -1;
    }
    register_ontology();

    if (ss_join(sslog_get_ss_info(), (char *)uuid) == -1) {
        NSLog(@"Can't join to Smart Space");
        return -1;
    }

    return 0;
}

//Disconnect from SmartSpace
-(int)disconnectFromSmartSpace {
    
    sslog_sbcr_stop_all();
	sslog_sbcr_unsubscribe_all(true);
	sslog_ss_leave_session(sslog_get_ss_info());
	sslog_repo_clean_all();
    
    NSLog(@"Succesfully disconnected");
    
    return 0;
}

//Register user in SmartSpace
-(int)userRegistrationWithUserName:(NSString *)userName andWithPassword:(NSString *)password {
    
    list_t* profileList = sslog_ss_get_individual_by_class_all(CLASS_PROFILE);
	int personFounded = -1;
    
	if(profileList == NULL) {
		NSLog(@"Can't find Profile individual");
		return -1;
	} else {
		list_head_t* pos = NULL;
		list_for_each(pos, &profileList->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual_t* profile = (individual_t*)(node->data);
			sslog_ss_populate_individual(profile);
            
            
			if(profile != NULL)
				personFounded = [self searchPersonByIndividual:profile withName:userName andPassword:password];
			else {
				NSLog(@"Can't find individual by uuid");
				return -1;
			}
            
			if(personFounded == 0) {
				if([self activatePersonWithIndividual:profile] != 0)
					return -1;
				return 0;
			}
		}
	}
    
	return personFounded;
}

//Search person by `username` and `password`
-(int)searchPersonByIndividual:(individual_t *)person withName:(NSString *)userName andPassword:(NSString *)password {
    
    prop_val_t *p_val_username = sslog_ss_get_property (person, PROPERTY_USERNAME);
    
	if(p_val_username == NULL) {
		NSLog(@"Property user name not found");
		p_val_username = [self initNullProperty];
	}
    
	prop_val_t *p_val_password = sslog_ss_get_property (person, PROPERTY_PASSWORD);
    
	if(p_val_password == NULL) {
		NSLog(@"Property password not found");
		p_val_password = [self initNullProperty];
	}
    
	NSString* prop_value_nsstring = [NSString stringWithFormat:@"%@", p_val_username->prop_value];
    NSString* prop_password_nsstring = [NSString stringWithFormat:@"%@", p_val_password->prop_value];
    
    if([userName isEqualToString:prop_value_nsstring] && [password isEqualToString:prop_password_nsstring]) {
        return 0;
    }
    
    
	return -1;
}


//Send to SS `newParticipantCome`
-(int)activatePersonWithIndividual:(individual_t *)profile {

    prop_val_t *person_prop = sslog_ss_get_property(profile, PROPERTY_PERSONINFORMATION);
    
	individual_t *person = (individual_t *)person_prop->prop_value;
    
	sslog_ss_populate_individual(person);
    
	/* If user has already entered SS */
	prop_val_t *person_status = sslog_ss_get_property(person, PROPERTY_STATUS);
	if(strcmp(person_status->prop_value, "online") == 0)
		return -1;
    
	if(sslog_ss_update_property(person, PROPERTY_STATUS->name, "offline", "online") == -1)
		NSLog(@"update _status_ error");
    
	individual_t *agendaGui = sslog_new_individual(CLASS_AGENDANOTIFICATION);
    
    sslog_set_individual_uuid(agendaGui, [self generateUuid:"agenda-notification"]);
    
    
	if(sslog_ss_add_property(agendaGui, PROPERTY_NEWPARTICIPANTCOME, profile) != 0 ) {
		NSLog(@"%s", get_error_text());
		return -1;
	}
    
	if(sslog_ss_insert_individual(agendaGui) != 0) {
		NSLog(@"insert failed");
		return -1;
	}
    
	return 0;
}


//Initialize NULL property with ""
-(prop_val_t *)initNullProperty {
    
    prop_val_t* property = (prop_val_t *) malloc (sizeof(prop_val_t));
    
	if(!property) {
		NSLog(@"malloc error");
		return NULL;
	}
    
	property->prop_value = "";
    
	return property;
}

//Generates unique uuid
//-(char*) generateUuid:(char *)uuid {
//	int postfix_length = 3;
//	char *tmp = (char*) malloc (sizeof(char) * strlen(uuid) + postfix_length);
//	char *rand_chr = (char*) malloc (sizeof(char) * postfix_length);
//	int rand_val;
//    
//	do {
//		srand(time(NULL));
//		rand_val = rand() % 1000;
//		sprintf(rand_chr, "%d", rand_val);
//		strcpy(tmp, uuid);
//		strcat(tmp, "-");
//		strcat(tmp, rand_chr);
//	} while(sslog_ss_exists_uuid(tmp) == 1);
//    
//	free(rand_chr);
//    
//	return tmp;
//}

//Generate UUID
-(const char*)generateUuid:(char *)uuid {

    // Returns a UUID
    CFUUIDRef uuid_ = CFUUIDCreate(kCFAllocatorDefault);
    NSString *uuidStr = (__bridge_transfer NSString *)CFUUIDCreateString(kCFAllocatorDefault, uuid_);
    //CFRelease(uuid);

    uuidStr = [@"KP-" stringByAppendingString:uuidStr];

    //NSLog(uuidStr);

    return [uuidStr UTF8String];
}

//Gets list of available services
-(NSMutableArray *)getServicesInfo {
    
	individual_t *agenda = sslog_ss_get_individual_by_class_all(CLASS_AGENDA);
    
    NSMutableArray *servicesStatus;
    servicesStatus = [[NSMutableArray alloc] init];
    
	if(agenda == NULL) {
		NSLog(@"Agenda service not found");
        [servicesStatus addObject:@"Agenda"];
	}

	individual_t *projector = sslog_ss_get_individual_by_class_all(CLASS_PROJECTORSERVICE);
    
	if(projector == NULL) {
		NSLog(@"Projector service not found");
        [servicesStatus addObject:@"Projector"];
	}
    
	return servicesStatus;
}

////Initializes subscription to conference activities
//-(int)initSubscription {
//    
//    subscription_t *subscription = sslog_new_subscription(true);
//    
//	list_t *propertiesList = list_get_new_list();
//	list_add_data(PROPERTY_NEWPARTICIPANTCOME, propertiesList);
//    
//	//sslog_sbcr_add_individual(subscription, (individual_t *)person, propertiesList );
//    
//	return 0;
//}
//
////TODO: Loads participant's info from smart space
//-(int)loadPersonInfoWithProperty:(void *)property {
//	individual_t *person = (individual_t*)property;
//    
//	sslog_ss_populate_individual(person);
//	if(person == NULL) {
//		NSLog(@"Can't populate person individual");
//		return -1;
//	}
//    
//	prop_val_t *p_val_name = sslog_ss_get_property(person, PROPERTY_NAME);
//	if(p_val_name == NULL) {
//		NSLog(@"Property name not found");
//		p_val_name = [self initNullProperty];
//	}
//    
//	return 0;
//}
//
////Initialization of Timeslot for Agenda
//-(int)loadTimeslotList {
//    
//	list_t* timeslots = sslog_ss_get_individual_by_class_all(CLASS_TIMESLOT);
//    
//	if (timeslots == NULL) {
//		NSLog(@"Can't find Timeslot individual");
//		return -1;
//        
//	} else {
//		list_head_t* pos = NULL;
//		list_for_each(pos, &timeslots->links) {
//			list_t* node = list_entry(pos, list_t, links);
//			individual_t* timeslot = (individual_t*)(node->data);
//			sslog_ss_populate_individual(timeslot);
//            
//			individual_t* individual = (individual_t *)sslog_repo_get_individual_by_uuid(timeslot->uuid);
//            
//			if(individual != NULL)
//				[self addTimeslotToListWithIndividual:individual];
//			else {
//				NSLog(@"Can't find individual by uuid");
//				return -1;
//			}
//		}
//	}
//    
//	return 0;
//}
//
////Addition individual info to java object
//-(void)addTimeslotToListWithIndividual:(individual_t *)timeslot {
//    
//	/* Gets participant name property */
//	prop_val_t *p_val_name = sslog_ss_get_property (timeslot, PROPERTY_SPEAKERNAME);
//	if(p_val_name == NULL) {
//		NSLog(@"Property speaker name not found");
//		p_val_name = [self initNullProperty];
//	} /* Gets participant name property */
//    
//    
//	/* Gets duration property */
//	prop_val_t *p_val_duration = sslog_ss_get_property (timeslot, PROPERTY_DURATION);
//	if(p_val_duration == NULL) {
//		NSLog(@"Property duration not found");
//		p_val_duration = [self initNullProperty];
//	} /* Gets duration property */
//    
//    
//	/* Gets presentation title property */
//	prop_val_t *p_val_pres_title = sslog_ss_get_property (timeslot, PROPERTY_PRESENTATIONTITLE);
//	if(p_val_pres_title == NULL) {
//		NSLog(@"Property presentation title not found");
//		p_val_pres_title = [self initNullProperty];
//	} /* Gets presentation title property */
//    
//    
//	/* Gets person link property */
//	char *imgLink = (char *) malloc (sizeof(char) * 200);
//    
//	strcpy(imgLink, "http://upload.wikimedia.org/wikipedia/commons/3/36/Bonhomme_crystal_marron.png");
//    
//	prop_val_t *p_val_person_link = sslog_ss_get_property (timeslot, PROPERTY_PERSONLINK);
//	if(p_val_person_link != NULL) {
//        
//		individual_t *person = (individual_t *)p_val_person_link->prop_value;
//		sslog_ss_populate_individual(person);
//        
//		prop_val_t *p_val_img = sslog_ss_get_property (person, PROPERTY_IMG);
//        
//		if(p_val_img != NULL) {
//			strcpy(imgLink, (char *)p_val_img->prop_value);
//		}
//	}
//    
//    //TODO: заполнить Agenda (имя, длительность, название презентации)
//	/* Calling Agenda's addTimeslotItemToList Java method */
////	(*env)->CallVoidMethod(env, obj, methodId,
////                           (*env)->NewStringUTF(env, (char *)(p_val_name->prop_value)),
////                           (*env)->NewStringUTF(env, (char *)(p_val_duration->prop_value)),
////                           (*env)->NewStringUTF(env, (char *)(p_val_pres_title->prop_value)),
////                           (*env)->NewStringUTF(env, imgLink));
//    
//	free(imgLink);
//    
//}
//
////Loads presentation with `uuid` or current presentation
//-(int)loadPresentationWithUuid:(NSString*)stringUuid {
//    
//	const char *uuid = [stringUuid cStringUsingEncoding:NSASCIIStringEncoding];
//	char *slideImg = (char *) malloc (sizeof(char) * 200);
//	individual_t *projectorService;
//    
//	/* load current presentation */
//	if(strcmp(uuid, "") == 0) {
//		list_t* projectorServiceList = sslog_ss_get_individual_by_class_all(CLASS_PROJECTORSERVICE);
//        
//        if(projectorServiceList != NULL) {
//            list_head_t* pos = NULL;
//            list_for_each(pos, &projectorServiceList->links) {
//                list_t* node = list_entry(pos, list_t, links);
//                projectorService = (individual_t*)(node->data);
//                sslog_ss_populate_individual(projectorService);
//                break;
//            }
//            
//            // TODO: slideImg empty !!!
//            prop_val_t *p_val_slideImg = sslog_ss_get_property (projectorService,
//                                                                PROPERTY_CURRENTSLIDEIMG);
//            if(p_val_slideImg != NULL) {
//                strcpy(slideImg, (char *)p_val_slideImg->prop_value);
//            } else {
//                NSLog(@"slide image empty");
//            }
//        }
//	} else {
//		/* load presentation by `uuid` */
//		// TODO
//	}
//    
//	free(slideImg);
//    
//	return 0;
//}

@end
