//
//  KPViewController.h
//  sr-ios-client
//
//  Created by Alexander on 21.04.13.
//  Copyright (c) 2013 alexkataev. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface KPViewController : UITableViewController {
    NSArray* listOfServices;
    NSMutableArray* servicesStatus;
}

- (IBAction)signOutButton:(id)sender;

//- (void)disableRowWithLabels:(NSMutableArray *)servicesNames;

@end
