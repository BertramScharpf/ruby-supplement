/*
 *  step/terminal.h  --  Terminal methods
 */

#ifndef __STEP_TERMINAL_H__
#define __STEP_TERMINAL_H__

#if   HAVE_HEADER_RUBY_H
    #include <ruby.h>
#elif HAVE_HEADER_RUBY_RUBY_H
    #include <ruby/ruby.h>
#endif


extern VALUE rb_io_unget( int, VALUE *, VALUE);
extern VALUE rb_io_winsize( VALUE);

extern void Init_terminal( void);

#endif

