//
//  MyGLView.h
//  OpenGLShaderBasic
//
//  Created by Zenny Chen on 10/4/10.
//  Copyright 2010 GreenGames Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MyGLView : NSOpenGLView

- (void)setTag:(NSInteger)tag;
- (void)destroyBuffers;

@end

