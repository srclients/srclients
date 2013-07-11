//
//  KP.h
//  sr-ios-client
//
//  Created by Alexander on 21.04.13.
//  Copyright (c) 2013 alexkataev. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "structures.h"
#import "SmartRoomOntology.h"
#import "Reachability.h"


@interface KP : NSObject

@property(nonatomic) char* ip;
@property(nonatomic) int port;
@property(nonatomic) char* hostname;

- (BOOL)isInternetAvailableNow;
- (int)connectToSmartSpaceOnIp:(char *)ip
                      ToPort:(int)port
                WithHostname:(char *)hostname;
//- (const char*)genUuid;
- (const char*) generateUuid:(char *)uuid;
- (int)disconnectFromSmartSpace;
- (int)userRegistrationWithUserName:(NSString *)userName andWithPassword:(NSString *)password;
- (int)searchPersonByIndividual:(individual_t *)person withName:(NSString *)userName andPassword:(NSString *)password;
- (int)activatePersonWithIndividual:(individual_t *)profile;
- (prop_val_t *)initNullProperty;
- (NSMutableArray *)getServicesInfo;
//- (int)initSubscription;
//- (int)loadPersonInfoWithProperty:(void *)property;
//- (int)loadTimeslotList;
//- (void)addTimeslotToListWithIndividual:(individual_t *)timeslot;
//- (int)loadPresentationWithUuid:(NSString*)stringUuid;

@end
