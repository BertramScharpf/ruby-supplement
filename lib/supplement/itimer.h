/*
 *  supplement/itimer.h  --  Interval timer
 */

#ifndef __SUPPLEMENT_ITIMER_H__
#define __SUPPLEMENT_ITIMER_H__

#if   HAVE_HEADER_RUBY_H
    #include <ruby.h>
#elif HAVE_HEADER_RUBY_RUBY_H
    #include <ruby/ruby.h>
#endif


extern VALUE rb_process_setitimer( int, VALUE *, VALUE);
extern VALUE rb_process_getitimer( VALUE obj);

extern void Init_itimer( void);

#endif

