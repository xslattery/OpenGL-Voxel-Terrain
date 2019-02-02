//
//  typedefs.hpp
//  2DProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#ifndef _TYPEDEFS_HPP
#define _TYPEDEFS_HPP

#include <iostream>
// #include <stdint.h>

// typedef uint8_t 		byte;
// typedef uint8_t 		u8;
// typedef uint16_t 		u16;
// typedef uint32_t 		u32;
// typedef uint64_t 		u64;
// typedef int8_t 			i8;
// typedef int16_t 		i16;
// typedef int32_t 		i32;
// typedef int64_t 		i64;

#define LOG(...) 		std::cout 					<< __VA_ARGS__ ;	// These will later be change to store information.
#define WARNING(...) 	std::cout << "WARNING: "	<< __VA_ARGS__ ;	// The logs will be prefixed with the log type.
#define ERROR(...) 		std::cout << "ERROR: " 		<< __VA_ARGS__ ;	// They will also be numbered in the order they are recieved.

#endif