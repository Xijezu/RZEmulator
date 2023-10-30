#pragma once
/*
 * Stacktrace.h
 *
 *  Created on: 15.08.2018
 *      Author: 0x5248
 */


class Stacktrace {
public:
    static void enableStacktracing();
    static void signalHandler(int signum);
};
