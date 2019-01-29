// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the 
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)
//

// A stored entry with name and age that is transmitted as an argument by DSTC.
// We use packed attribute to ensure that we can run across architectures.
//
// Optionally, we can specify __attribute__((endianness(little))) to
// ensure endian-ness as well.
//
struct __attribute__((packed))
name_and_age {
    char name[64];
    int age;
};
        
