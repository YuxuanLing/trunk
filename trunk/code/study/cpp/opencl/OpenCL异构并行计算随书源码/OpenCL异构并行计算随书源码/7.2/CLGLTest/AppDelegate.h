//
//  AppDelegate.h
//  CLGLTest
//
//  Created by Zenny Chen on 14-8-1.
//  Copyright (c) 2014年 Adwo. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "MyGLView.h"

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
{
@private
    
    CGFloat mYPos;
}

@property (assign) IBOutlet NSWindow *window;

@end

