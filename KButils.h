
#define SCREEN_CALIBRATION 0  // start at calibration screen
#define SCREEN_HOME 1         // Home screen
#define SCREEN_IMAGEVIEWER 2  //
#define SCREEN_PASSWORDMAN 3  // Password Manager

//#define SCREEN_CROSSHAIRDEMO 1 // stuff to do after calibration finished
#include "keyLookUp.h"              // OEM key lookup table (US) and converter
#include "smStruct.h"               // struct to store state machine
//#include "buttons.h"                // Utilites for drawing/watching buttons

// KB-Functions
#include "calibrationUtils.h"       // set screen calibration variables
#include "homeScreen.h"             // Provide basic home screen
#include "imageViewer.h"            // Basic image viewer to demonstrate file open dialogue
#include "passwordManager.h"        // Provides encrypted password manager

#include "dialogue.h"
