/*
 * Stacktrace.cpp
 *
 *  Created on: 15.08.2018
 *      Author: 0x5248
 */

#include "Stacktrace.h"
#include "Log.h"
#include <iostream>
#include <signal.h>
#include <boost/stacktrace.hpp>
#include <sstream>

void Stacktrace::enableStacktracing()
{
	::signal(SIGSEGV, &signalHandler);
	::signal(SIGABRT, &signalHandler);
}

void Stacktrace::signalHandler(int signum)
{
	::signal(signum, SIG_DFL);

	std::stringstream tmp;
	tmp << "Server crashed:\n" << boost::stacktrace::stacktrace();
	std::string output = tmp.str();

	NG_LOG_FATAL("server", output);

    ::raise(SIGABRT);
}
