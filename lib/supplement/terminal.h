/*
 *  supplement/terminal.h  --  Terminal methods
 */

#ifndef __SUPPLEMENT_TERMINAL_H__
#define __SUPPLEMENT_TERMINAL_H__

#include <ruby/ruby.h>


extern VALUE rb_io_unget( int, VALUE *, VALUE);
extern VALUE rb_io_wingeom( VALUE);

extern void Init_terminal( void);

#endif

