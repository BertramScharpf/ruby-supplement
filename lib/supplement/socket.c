/*
 *  supplement/socket.c  --  Socket methods
 */

#include <ruby/io.h>


static ID id_accept_orig = 0;
static ID id_close = 0;

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
    VALUE rb_cUNIXServer;

    id_accept_orig = rb_intern( "accept_orig");
    id_close = rb_intern( "close");

    rb_require( "socket");
    rb_cTCPServer  = rb_const_get( rb_cObject, rb_intern( "TCPServer"));
    rb_cUNIXServer = rb_const_get( rb_cObject, rb_intern( "UNIXServer"));

    rb_define_alias( rb_cTCPServer,  "accept_orig", "accept");
    rb_define_method( rb_cTCPServer,  "accept", rb_socket_accept, 0);
    rb_define_alias( rb_cUNIXServer, "accept_orig", "accept");
    rb_define_method( rb_cUNIXServer, "accept", rb_socket_accept, 0);
}

