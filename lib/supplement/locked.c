/*
 *  supplement/locked.c  --  Locked File
 */

#include "locked.h"

#include <ruby/io.h>

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
#ifdef FEATURE_OLD_IO_FUNCTIONS
    rb_io_t *fptr;
#endif
#ifdef HAVE_FUNC_RB_THREAD_WAIT_FOR
    struct timeval time;
#endif
    int op;

#ifdef FEATURE_OLD_IO_FUNCTIONS
    GetOpenFile( self, fptr);
#endif

#ifdef FEATURE_SUPER_KWARGS
    rb_call_super_kw(argc, argv, RB_PASS_CALLED_KEYWORDS);
#else
    rb_call_super( argc, argv);
#endif

#ifdef FEATURE_OLD_IO_FUNCTIONS
    op = fptr->mode & FMODE_WRITABLE ? LOCK_EX : LOCK_SH;
#else
    op = rb_io_mode( self) & FMODE_WRITABLE ? LOCK_EX : LOCK_SH;
#endif
    op |= LOCK_NB;
#ifdef FEATURE_OLD_IO_FUNCTIONS
    while (flock( fptr->fd, op) < 0) {
#else
    while (flock( rb_io_descriptor( self), op) < 0) {
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
#ifdef FEATURE_OLD_IO_FUNCTIONS
            rb_sys_fail_str( fptr->pathv);
#else
            rb_sys_fail_str( rb_io_path( self));
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
    flock( rb_io_descriptor( self), LOCK_UN);
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

