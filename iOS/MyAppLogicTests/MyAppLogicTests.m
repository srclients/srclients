//
//  MyAppLogicTests.m
//  MyAppLogicTests
//
//  Created by Alexander on 06.06.13.
//  Copyright (c) 2013 alexkataev. All rights reserved.
//

#import "MyAppLogicTests.h"

@implementation MyAppLogicTests

- (void)setUp
{
    [super setUp];
    
    // Set-up code here.
}

- (void)tearDown
{
    // Tear-down code here.
    
    [super tearDown];
}

- (void)testConnectToSmartSpaceOnIpToPortWithHostname
{
    KP* connect = [[KP alloc] init];

    //Невозможно проверить c некорректными параметрами - есть проверка внутри SmartSlog
    //STAssertFalse(([connect connectToSmartSpaceOnIp:"abra_" ToPort:3846592 WithHostname:0] == 0), @"Incorrect ip, port and hostname");
    
    STAssertFalse(([connect connectToSmartSpaceOnIp:"194.85.173.9" ToPort:10010 WithHostname:"X"] == -1), @"No internet connection or server is not available!");
}

- (void)testGetServicesInfo {
    
    KP* connect = [[KP alloc] init];
    
    NSMutableArray *checkServices = [[NSMutableArray alloc] init];
    [checkServices addObject:@"Agenda"];
    [checkServices addObject:@"Projector"];
    
    STAssertFalse(([connect getServicesInfo] == checkServices), @"Services are not available!");

}

//- (void)testConnectToSmartSpaceOnIp:(char *)ip
//                             ToPort:(int)port
//                       WithHostname:(char *)hostname
//{
//    
//    STAssertEquals(ip, "194.85.173.9", @"Incorrect ip-address");
//    
//}



@end
