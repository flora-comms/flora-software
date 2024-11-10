/*
======   AVAlink.h  ======

Version: v1.0
Last Update: Nov 6 2024

======   DESCRIPTION   =====

Header file that includes all necessary libs for AVAlink firmware.

*/

#ifndef AVALINK_H
#define AVALINK_H
#pragma once

#include <AVAlinkRadio.h>


// ----- GLOBALS ------ //



// ------ PROTOTYPES ------ //

/// @brief Initializes AVAlink hardware
void initAvalink();

void userButtonPress(void);

#endif