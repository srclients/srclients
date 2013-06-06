//
//  SignInViewController.h
//  sr-ios-client
//
//  Created by Alexander on 21.04.13.
//  Copyright (c) 2013 alexkataev. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface SignInViewController : UIViewController

@property (weak, nonatomic) IBOutlet UITextField *emailTextField;
@property (weak, nonatomic) IBOutlet UITextField *passwordTextField;
@property (weak, nonatomic) IBOutlet UIButton *signInButton;

- (IBAction)signInButton:(id)sender;

@end
