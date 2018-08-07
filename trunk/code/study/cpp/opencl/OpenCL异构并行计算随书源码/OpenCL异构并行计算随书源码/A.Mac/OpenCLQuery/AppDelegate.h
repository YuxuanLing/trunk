//
//  AppDelegate.h
//  OpenCLQuery
//
//  Created by Zenny Chen on 13-9-9.
//  Copyright (c) 2013年 Zenny Chen. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    NSMutableArray *mPlatformArray;
    NSMutableArray *mDeviceArray;
    NSMutableArray *mContentKeys;
    NSMutableArray *mContentValues;
    
    NSTableColumn *mFeatureNameCol;
    NSTableColumn *mFeatureValueCol;
    NSScrollView *mScrollView;
}

@property (assign) IBOutlet NSWindow *window;

@end
