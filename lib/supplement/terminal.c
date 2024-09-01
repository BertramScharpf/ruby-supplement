/*
 *  supplement/terminal.c  --  Terminal methods
 */

#include "terminal.h"

#include <ruby/io.h>

#include <sys/ioctl.h>
#include <termios.h>

static VALUE io_unget( VALUE);
static VALUE io_reset( VALUE);


#ifdef FEATURE_OLD_IO_FUNCTIONS
  #define RB_FAIL_PATH( io, fptr)   ((fptr)->pathv)
  #define RB_IO_VAR(    io, fptr)   fptr
#else
  #define RB_FAIL_PATH( io, fptr)   rb_io_path( io)
  #define RB_IO_VAR(    io, fptr)   io
#endif

/*
 *  call-seq:
 *     io.unget(str, ...)   -> nil
 *
 *  Feed <em>str</em> into the TTY's input queue.
 *
 *     $stdin.unget "hello\n"    #=> nil
 */

VALUE
rb_io_unget( int argc, VALUE *argv, VALUE io)
{
#ifdef FEATURE_OLD_IO_FUNCTIONS
    rb_io_t *fptr;
#endif
    int fd;
    struct termios oldtio, newtio;
    void *v[5];

#ifdef FEATURE_OLD_IO_FUNCTIONS
    GetOpenFile( io, fptr);
    fd = fptr->fd;
#else
    fd = rb_io_descriptor( io);
#endif

    if (tcgetattr( fd, &oldtio) < 0)
        rb_sys_fail_str( RB_FAIL_PATH( io, fptr));
    newtio = oldtio;
    newtio.c_iflag &= ~ICRNL;
    if (tcsetattr( fd, TCSANOW, &newtio) < 0)
        rb_sys_fail_str( RB_FAIL_PATH( io, fptr));

    v[0] = &fd, v[1] = (void *) RB_IO_VAR( io, fptr), v[2] = &oldtio,
        v[3] = &argc, v[4] = (void *) argv;
    return rb_ensure( io_unget, (VALUE) v, io_reset, (VALUE) v);
}

VALUE
io_unget( VALUE v)
{
    void **vp = (void **) v;
    VALUE str;
    long i;
    int j;
    char *p;
    int *argc;
    VALUE *argv;

    argc = (int *)vp[3], argv = (VALUE *) vp[4];
    for (j = 0 ; j < *argc; ++j) {
        str = argv[ j];
        p = RSTRING_PTR(str);
        for (i = RSTRING_LEN(str); i; --i, ++p)
            if (ioctl( *(int *) vp[0], TIOCSTI, p) < 0)
                rb_sys_fail_str( RB_FAIL_PATH( (VALUE) vp[1], (rb_io_t *) vp[1]));
    }
    return Qnil;
}

VALUE
io_reset( VALUE v)
{
    void **vp = (void **) v;

    if (tcsetattr( *(int *) vp[0], TCSANOW, (struct termios *) vp[2]) < 0)
        rb_sys_fail_str( RB_FAIL_PATH( (VALUE) vp[1], (rb_io_t *) vp[1]));

    return Qnil;
}


/*
 *  call-seq:
 *     io.wingeom()   -> ary
 *
 *  Get the available window space.
 *
 *     cols, rows, xpixel, ypixel = $stdout.wingeom
 */

VALUE
rb_io_wingeom( VALUE self)
{
#ifdef FEATURE_OLD_IO_FUNCTIONS
    rb_io_t *fptr;
#endif
    int fd;
    struct winsize w;
    VALUE r;

#ifdef FEATURE_OLD_IO_FUNCTIONS
    GetOpenFile( self, fptr);
    fd = fptr->fd;
#else
    fd = rb_io_descriptor( self);
#endif

    if (ioctl( fd, TIOCGWINSZ, &w) < 0)
        rb_sys_fail_str( RB_FAIL_PATH( self, fptr));
    r = rb_ary_new2( 4);
    rb_ary_store( r, 0, INT2NUM( w.ws_col));
    rb_ary_store( r, 1, INT2NUM( w.ws_row));
    rb_ary_store( r, 2, INT2NUM( w.ws_xpixel));
    rb_ary_store( r, 3, INT2NUM( w.ws_ypixel));
    return r;
}


void Init_terminal( void)
{
    rb_define_method( rb_cIO, "unget", rb_io_unget, -1);
    rb_define_method( rb_cIO, "wingeom", rb_io_wingeom, 0);
}

