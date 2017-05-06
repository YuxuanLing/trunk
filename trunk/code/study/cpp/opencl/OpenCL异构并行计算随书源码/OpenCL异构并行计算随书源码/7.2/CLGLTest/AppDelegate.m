//
//  AppDelegate.m
//  CLGLTest
//
//  Created by Zenny Chen on 14-8-1.
//  Copyright (c) 2014年 Adwo. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    self.window.delegate = self;
    
    NSView *baseView = self.window.contentView;
    
    mYPos = baseView.frame.size.height - 60.0f;
    
    NSButton *showButton = [[NSButton alloc] initWithFrame:NSMakeRect(20.0f, mYPos, 90.0f, 35.0f)];
    [showButton setButtonType:NSMomentaryLightButton];
    [showButton setBezelStyle:NSRoundedBezelStyle];
    [showButton setTitle:@"Show"];
    [showButton setTarget:self];
    [showButton setAction:@selector(showButtonClicked:)];
    [baseView addSubview:showButton];
    [showButton release];
    
    NSButton *closeButton = [[NSButton alloc] initWithFrame:NSMakeRect(showButton.frame.origin.x + showButton.frame.size.width + 30.0f, mYPos, 90.0f, 35.0f)];
    [closeButton setButtonType:NSMomentaryLightButton];
    [closeButton setBezelStyle:NSRoundedBezelStyle];
    [closeButton setTitle:@"Close"];
    [closeButton setTarget:self];
    [closeButton setAction:@selector(closeButtonClicked:)];
    [baseView addSubview:closeButton];
    [closeButton release];
    
    NSButton *updateButton = [[NSButton alloc] initWithFrame:NSMakeRect(closeButton.frame.origin.x + closeButton.frame.size.width + 30.0f, mYPos, 90.0f, 35.0f)];
    [updateButton setButtonType:NSMomentaryLightButton];
    [updateButton setBezelStyle:NSRoundedBezelStyle];
    [updateButton setTitle:@"Update"];
    [updateButton setTarget:self];
    [updateButton setAction:@selector(upateButtonClicked:)];
    [baseView addSubview:updateButton];
    [updateButton release];
    
    mYPos -= 30.0f;
}

- (void)showButtonClicked:(id)sender
{
    NSView *baseView = self.window.contentView;
    MyGLView *glView = [baseView viewWithTag:13];
    if(glView != nil)
        return;
    
    glView = [[MyGLView alloc] initWithFrame:NSMakeRect((baseView.frame.size.width - 512.0f) * 0.5f, mYPos - 512.0f, 512.0f, 512.0f)];
    glView.tag = 13;
    [baseView addSubview:glView];
    [glView release];
}

- (void)closeButtonClicked:(id)sender
{
    NSView *baseView = self.window.contentView;
    MyGLView *glView = [baseView viewWithTag:13];
    if(glView != nil)
    {
        [glView destroyBuffers];
        [glView removeFromSuperview];
    }
}

- (void)upateButtonClicked:(id)sender
{
    NSView *baseView = self.window.contentView;
    MyGLView *glView = [baseView viewWithTag:13];
    if(glView != nil)
        [glView setNeedsDisplay:YES];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    return NSTerminateNow;
}

- (void)windowWillClose:(NSNotification *)notification
{
    [[NSApplication sharedApplication] terminate:self];
}

@end

