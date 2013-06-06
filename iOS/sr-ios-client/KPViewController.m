//
//  KPViewController.m
//  sr-ios-client
//
//  Created by Alexander on 21.04.13.
//  Copyright (c) 2013 alexkataev. All rights reserved.
//

#import "KPViewController.h"
#import "KP.h"

@interface KPViewController ()

@end

@implementation KPViewController

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
    
    KP* services = [[KP alloc] init];
    servicesStatus = [services getServicesInfo];

    //[self disableRowWithLabels:servicesStatus];
    
    //NSIndexPath* indexPath = [NSIndexPath indexPathForRow:0 inSection:0];

    
    UIBarButtonItem *item = [[UIBarButtonItem alloc] initWithTitle:@"Sign Out"
                                                             style:UIBarButtonItemStyleBordered
                                                            target:self
                                                            action:@selector(signOutButton:)];
    self.navigationItem.hidesBackButton = YES;
    self.navigationItem.leftBarButtonItem = item;   
    
    listOfServices = [NSArray arrayWithObjects: @"Agenda", @"Projector", @"Sensors", @"Blogging", nil];
    
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


//List of services
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [listOfServices count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *simpleTableIdentifier = @"ServicesTableItem";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
    }
    
    cell.textLabel.text = [listOfServices objectAtIndex:indexPath.row];
    if ([cell.textLabel.text isEqualToString:@"Agenda"] ) {
        cell.imageView.image = [UIImage imageNamed:@"agenda.png"];
        if([[servicesStatus objectAtIndex:0] isEqualToString:@"Agenda"]) {
            cell.userInteractionEnabled = NO;
        }
    }
    if ([cell.textLabel.text isEqualToString:@"Projector"] ) {
        cell.imageView.image = [UIImage imageNamed:@"projector.png"];
        if([[servicesStatus objectAtIndex:1] isEqualToString:@"Projector"]) {
            cell.userInteractionEnabled = NO;
        }
    }
    if ([cell.textLabel.text isEqualToString:@"Sensors"] ) {
        cell.imageView.image = [UIImage imageNamed:@"sensors.png"];
//        if([[servicesStatus objectAtIndex:2] isEqualToString:@"Sensors"]) {
//            cell.userInteractionEnabled = NO;
//        }
        cell.userInteractionEnabled = NO;
    }
    if ([cell.textLabel.text isEqualToString:@"Blogging"] ) {
        cell.imageView.image = [UIImage imageNamed:@"blogging.png"];
//        if([[servicesStatus objectAtIndex:3] isEqualToString:@"Blogging"]) {
//            cell.userInteractionEnabled = NO;
//        }
        cell.userInteractionEnabled = NO;
    }
    
    return cell;
}

- (IBAction)signOutButton:(id)sender {
    
    KP* disconnect = [[KP alloc] init];
    
    if ([disconnect disconnectFromSmartSpace] == 0) {
        [self.navigationController popViewControllerAnimated:YES];
    }
    
}

//- (void)disableRowWithLabels:(NSMutableArray *)servicesNames {
//    
//    if ([[servicesNames objectAtIndex:0] isEqualToString:@"Agenda"] ) {
//        
//        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:0 inSection:0];
//        UITableViewCell *agendaCell = [self.tableView cellForRowAtIndexPath:indexPath];
//        agendaCell.userInteractionEnabled = NO;
//    }
//    if ([[servicesNames objectAtIndex:1] isEqualToString:@"Projector"] ) {
//        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:1 inSection:0];
//        UITableViewCell *projectorCell = [self.tableView cellForRowAtIndexPath:indexPath];
//        projectorCell.userInteractionEnabled = NO;
//    }
//
//}

@end






