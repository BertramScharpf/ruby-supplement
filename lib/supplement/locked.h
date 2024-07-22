/*
 *  supplement/filesys.h  --  File system tools
 */

#ifndef __SUPPLEMENT_FILESYS_H__
#define __SUPPLEMENT_FILESYS_H__

#include <ruby/ruby.h>

VALUE rb_locked_init( int argc, VALUE *argv, VALUE self);
VALUE rb_locked_close( VALUE self);

extern void Init_locked( void);

#endif

