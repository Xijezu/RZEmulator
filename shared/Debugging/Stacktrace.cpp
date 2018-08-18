/*
 * Stacktrace.cpp
 *
 *  Created on: 15.08.2018
 *      Author: 0x5248
 */

#include "Stacktrace.h"
#include <iostream>
#include <signal.h>
#include <boost/stacktrace.hpp>
#include <boost/filesystem.hpp>

void Stacktrace::enableStacktracing()
{
	::signal(SIGSEGV, &signalHandler);
	::signal(SIGABRT, &signalHandler);
}

void Stacktrace::parseDump()
{
	if (boost::filesystem::exists("./backtrace.dump")) {
	    // there is a backtrace
	    std::ifstream ifs("./backtrace.dump");

	    boost::stacktrace::stacktrace st = boost::stacktrace::stacktrace::from_dump(ifs);
	    std::cout << "Previous run crashed:\n" << st << std::endl;

	    // cleaning up
	    ifs.close();
	    boost::filesystem::remove("./backtrace.dump");
	}
}

void Stacktrace::signalHandler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cout << "Server crashed:\n" << boost::stacktrace::stacktrace() << std::endl;
    boost::stacktrace::safe_dump_to("./backtrace.dump");
    ::raise(SIGABRT);
}
