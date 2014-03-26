/*
 *  supplement/locked.c  --  Locked File
 */

#include "locked.h"

#if   HAVE_HEADER_RUBYIO_H
    #include <rubyio.h>
#elif HAVE_HEADER_RUBY_IO_H
    #include <ruby/io.h>
#endif

#include <sys/file.h>


/*
 *  Document-class: LockedFile
 *
 *  Open a File and lock it.
 */

/*
 *  call-seq:
 *     LockedFile.open( *args)                 -> file
 *     LockedFile.open( *args) { |file| ... }  -> ob.j
 *
 *  Same as File.open but do an +flock+. If the lock fails, the method
 *  +lock_failed+ will be called. If that doesn't exist, another lock
 *  will be tried.
 */

VALUE
rb_locked_init( int argc, VALUE *argv, VALUE self)
{
#ifdef HAVE_HEADER_RUBY_H
    OpenFile *fptr;
#else
    rb_io_t *fptr;
#endif
#ifdef HAVE_FUNC_RB_THREAD_WAIT_FOR
    struct timeval time;
#endif
    int op;

    rb_call_super( argc, argv);
    GetOpenFile( self, fptr);

    op = fptr->mode & FMODE_WRITABLE ? LOCK_EX : LOCK_SH;
    op |= LOCK_NB;
#ifdef HAVE_HEADER_RUBY_H
    while (flock( fileno( fptr->f), op) < 0) {
#else
    while (flock( fptr->fd, op) < 0) {
#endif
        static ID id_lock_failed = 0;

        switch (errno) {
	case EINTR:
	    break;
        case EAGAIN:
        case EACCES:
#if defined( EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
            if (id_lock_failed == 0)
                id_lock_failed = rb_intern( "lock_failed");
            if (rb_respond_to( self, id_lock_failed))
                rb_funcall( self, id_lock_failed, 0);
            else {
#ifdef HAVE_FUNC_RB_THREAD_WAIT_FOR
                time.tv_sec = 0;
                time.tv_usec = 100 * 1000;
                rb_thread_wait_for(time);
#else
                rb_thread_polling();
#endif
            }
            break;
        default:
#ifdef HAVE_HEADER_RUBY_H
            rb_sys_fail( fptr->path);
#else
            rb_sys_fail_str( fptr->pathv);
#endif
            break;
        }
    }
    return self;
}


/*
 *  call-seq:
 *     close()                 -> nil
 *
 *  Unlock using flock and close.
 */

VALUE
rb_locked_close( VALUE self)
{
#ifdef HAVE_HEADER_RUBY_H
    OpenFile *fptr;
#else
    rb_io_t *fptr;
#endif

    GetOpenFile( self, fptr);
#ifdef HAVE_HEADER_RUBY_H
    flock( fileno( fptr->f), LOCK_UN);
#else
    flock( fptr->fd, LOCK_UN);
#endif
    rb_call_super( 0, NULL);
    return Qnil;
}

void Init_locked( void)
{
    VALUE rb_cLockedFile;

    rb_cLockedFile = rb_define_class( "LockedFile", rb_cFile);

    rb_define_method( rb_cLockedFile, "initialize", &rb_locked_init, -1);
    rb_define_method( rb_cLockedFile, "close",      &rb_locked_close, 0);
}

