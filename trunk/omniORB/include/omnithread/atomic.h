// -*- Mode: C++; -*-
//				Package : omnithread
// omnithread/atomic.h		Created : 2011/04/14 dgrisby
//
//    Copyright (C) 2011 Apasphere Ltd
//
//    This file is part of the omnithread library
//
//    The omnithread library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
//    02111-1307, USA
//

//
// Atomic operations for platforms that support them.

#ifndef __omnithread_atomic_h_
#define __omnithread_atomic_h_


#if defined(__GNUG__)
#  if (__GNUG__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUG__ > 4)
#    define OMNI_ATOMIC_GCC_SYNC
#  endif
#endif


#ifdef OMNI_ATOMIC_GCC_SYNC

#  define OMNI_REFCOUNT_DEFINED

class omni_refcount {
public:
  inline omni_refcount(int start) : count(start) {}
  inline ~omni_refcount() {}

  // Atomically increment reference count
  inline void inc() {
    __sync_fetch_and_add(&count, 1);
  }

  // Atomically decrement reference count and return new value
  inline int dec() {
    return __sync_sub_and_fetch(&count, 1);
  }

  // Return snapshot of current value. Real count may have changed by
  // the time the value is looked at!
  inline int value() {
    return count;
  }

private:
  volatile int count;
};

#endif // OMNI_ATOMIC_GCC_SYNC


#endif // __omnithread_atomic_h_