//
//  osx_fileio.h
//  RTSProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#ifndef osx_fileio_h
#define osx_fileio_h

const char* getApplicationSupport();
const char* getFilePathFromDlg( GLFWwindow* win );

void createDirAt( const char* dir );
void removeFileAt( const char* filename );


#endif /* osx_fileio_h */