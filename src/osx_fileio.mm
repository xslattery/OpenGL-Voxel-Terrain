//
//  osx_fileio.mm
//  RTSProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#define GLFW_INCLUDE_NONE // FIXES : gl.h and gl3.h conflict.
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

const char* getApplicationSupport() {

#if BUILD_FOR_APP_BUNDLE
    NSString* bundleID = [[NSBundle mainBundle] bundleIdentifier];
#else
    NSString* bundleID = @"_DEBUG-2DProject";
#endif

    NSFileManager*fm = [NSFileManager defaultManager];
    NSURL* dirPath = nil;
    
    // Find the application support directory in the home directory.
    NSArray* appSupportDir = [fm URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    if ( [appSupportDir count] > 0 ) {
        
        // Append the bundle ID to the URL for the
        // Application Support directory
        dirPath = [[appSupportDir objectAtIndex:0] URLByAppendingPathComponent:bundleID];
        
        // If the directory does not exist, this method creates it.
        // This method is only available in OS X v10.7 and iOS 5.0 or later.
        NSError* theError = nil;
        if ( ![fm createDirectoryAtURL:dirPath withIntermediateDirectories:YES attributes:nil error:&theError] ) {
            // Handle the error.
            return (char*)"?";
        }
    }

    //NSLog(@"applicationSupportDirectory: '%s'",  [dirPath.path UTF8String] );
    return (const char*)[[dirPath.path stringByAppendingString:@"/"] UTF8String];
}

void createDirAt( const char* dir ) {
    NSFileManager*fm = [NSFileManager defaultManager];
    NSString* directory = [NSString stringWithUTF8String:dir];
    NSError* error = nil;
    [[NSFileManager defaultManager] createDirectoryAtPath:directory withIntermediateDirectories:NO attributes:nil error:&error];
}

void removeFileAt( const char* filename ) {
	NSString* fileremoval = [NSString stringWithUTF8String:filename];
	[[NSFileManager defaultManager] removeItemAtPath:fileremoval error:NULL];
}

const char* getFilePathFromDlg( GLFWwindow* win ) {

    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    [openDlg setCanChooseFiles:YES];
    [openDlg setAllowsMultipleSelection:NO];
    [openDlg setCanChooseDirectories:NO];

    if ( [openDlg runModal] == NSModalResponseOK ) {
        NSURL* file = [[openDlg URLs] objectAtIndex:0];
        return (char*)[[file path] UTF8String];
    }    

    // Get the main window for the document.
   // NSWindow* window = glfwGetCocoaWindow( win );
 
   // // Create and configure the panel.
   // NSOpenPanel* panel = [NSOpenPanel openPanel];
   // [panel setCanChooseDirectories:YES];
   // [panel setAllowsMultipleSelection:YES];
   // [panel setMessage:@"Import one or more files or directories."];
 
   // // Display the panel attached to the document's window.
   // [panel beginSheetModalForWindow:window completionHandler:^(NSInteger result){
   //    if (result == NSFileHandlingPanelOKButton) {
   //       NSArray* urls = [panel URLs];
 
   //       // Use the URLs to build a list of items to import.
   //    }
 
   //  }];

    return (const char*)"?";
}