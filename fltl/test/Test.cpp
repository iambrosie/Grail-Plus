/*
 * Test.cpp
 *
 *  Created on: Jan 24, 2011
 *      Author: Peter Goodman
 *     Version: $Id$
 *
 * Copyright 2011 Peter Goodman, all rights reserved.
 */

#ifndef FLTL_TEST_CPP_
#define FLTL_TEST_CPP_

#include "fltl/test/Test.hpp"

namespace fltl { namespace test {

    namespace detail {

        TestBase::TestBase(void) throw() { }

        /// the first and last tests to run
        TestBase *TestBase::first_test(0);
        TestBase *TestBase::last_test(0);

    }

    /// run all tests
    void run_tests(void) throw() {

        for(detail::TestBase *curr(detail::TestBase::first_test);
            0 != curr;
            curr = curr->next) {

            curr->run_test();
        }
    }

}}

#endif /* FLTL_TEST_CPP_ */
