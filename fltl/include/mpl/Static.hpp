/*
 * Static.hpp
 *
 *  Created on: Jan 13, 2011
 *      Author: Peter Goodman
 *     Version: $Id$
 *
 * Copyright 2011 Peter Goodman, all rights reserved.
 */

#ifndef FLTL_STATIC_HPP_
#define FLTL_STATIC_HPP_

namespace fltl { namespace mpl {

    /// produces a value of a given type
    template <typename T>
    class Static {
    private:

        static T default_value(void) throw() {
            T val;
            return val;
        }

    public:
        static const T VALUE;
    };

    template <typename T>
    const T Static<T>::VALUE(Static<T>::default_value());
}}

#endif /* FLTL_STATIC_HPP_ */
