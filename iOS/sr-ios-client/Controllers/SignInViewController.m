//
//  SignInViewController.m
//  sr-ios-client
//
//  Created by Alexander on 21.04.13.
//  Copyright (c) 2013 alexkataev. All rights reserved.
//

#import "SignInViewController.h"
#import "KP.h"

@interface SignInViewController ()
@end

@implementation SignInViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)signInButton:(id)sender {
    
    KP* connect = [[KP alloc] init];
    
    char* ip;
    int port;
    char* hostname;
    
    port = 10010;
    //port = 10999;
    ip = "194.85.173.9";
    //ip = "194.85.173.14";
    
    //ip = "192.168.0.22";
    //ip = "192.168.112.108";
    //ip = "sib-pub.cs.prv";
    
    hostname = "X";
    
    [connect connectToSmartSpaceOnIp:ip ToPort:port WithHostname:hostname];
    
}
@end
