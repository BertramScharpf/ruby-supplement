/*
 *  supplement/socket.c  --  Socket methods
 */

#if   HAVE_HEADER_RUBYIO_H
    #include <rubyio.h>
#elif HAVE_HEADER_RUBY_IO_H
    #include <ruby/io.h>
#endif


static ID id_accept_orig;
static ID id_close;

static VALUE socket_close( VALUE);


/*
 *  call-seq:
 *     socket.accept() { |socket| ... }  -> obj
 *
 *  Accept a connection and yield it to a block, if given.
 */

VALUE
rb_socket_accept( VALUE socket)
{
    VALUE a = rb_funcall( socket, id_accept_orig, 0);
    return rb_block_given_p() ?
        rb_ensure( rb_yield, a, socket_close, a) : a;
}

VALUE
socket_close( VALUE v)
{
    return rb_funcall( v, id_close, 0);
}


void Init_socket( void)
{
    VALUE rb_cTCPServer;

    id_accept_orig = rb_intern( "accept_orig");
    id_close = rb_intern( "close");

    rb_require( "socket");
    rb_cTCPServer = rb_const_get( rb_cObject, rb_intern( "TCPServer"));

    rb_define_alias( rb_cTCPServer, "accept_orig", "accept");
    rb_define_method( rb_cTCPServer, "accept", rb_socket_accept, 0);
}

