/*
 *  supplement/socket.h  --  Socket methods
 */

#ifndef __SUPPLEMENT_SOCKET_H__
#define __SUPPLEMENT_SOCKET_H__

#if   HAVE_HEADER_RUBY_H
    #include <ruby.h>
#elif HAVE_HEADER_RUBY_RUBY_H
    #include <ruby/ruby.h>
#endif


extern VALUE rb_socket_accept( VALUE);


extern void Init_socket( void);

#endif

