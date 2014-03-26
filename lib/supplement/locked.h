/*
 *  supplement/filesys.h  --  File system tools
 */

#ifndef __SUPPLEMENT_FILESYS_H__
#define __SUPPLEMENT_FILESYS_H__

#if   HAVE_HEADER_RUBY_H
    #include <ruby.h>
#elif HAVE_HEADER_RUBY_RUBY_H
    #include <ruby/ruby.h>
#endif

VALUE rb_locked_init( int argc, VALUE *argv, VALUE self);
VALUE rb_locked_close( VALUE self);

extern void Init_locked( void);

#endif

