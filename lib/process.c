/*
 *  process.c  --  Renice; sync all
 */


#include "process.h"

#if   HAVE_HEADER_RUBY_H
    #include <ruby.h>
#elif HAVE_HEADER_RUBY_RUBY_H
    #include <ruby/ruby.h>
#endif

#include <sys/time.h>
#include <sys/resource.h>

#include <unistd.h>


static VALUE rb_process_renice( int argc, VALUE *argv, VALUE obj);
static VALUE rb_process_sync( VALUE obj);


/*
 *  call-seq:
 *     Process.renice( pid = nil, priority) -> nil
 *
 * Set a new nice value. If pid is +nil+, renice the current process.
 */

VALUE
rb_process_renice( int argc, VALUE *argv, VALUE obj)
{
    VALUE p1, p2;
    pid_t pid;
    int prio;

    if (rb_scan_args( argc, argv, "11", &p1, &p2) == 1)
        pid = getpid(),     prio = NUM2INT( p1);
    else
        pid = NUM2INT( p1), prio = NUM2INT( p2);

    rb_secure(2);
    if (setpriority( PRIO_PROCESS, pid, prio) < 0)
        rb_sys_fail(0);

    return Qnil;
}


/*
 *  call-seq:
 *     Process.sync -> nil
 *
 * Force completion of pending disk writes (flush cache). See sync(8).
 */

VALUE
rb_process_sync( VALUE obj)
{
    sync();
    return Qnil;
}


void Init_supplement_process( void)
{
    rb_define_singleton_method( rb_mProcess, "renice", rb_process_renice, -1);
    rb_define_singleton_method( rb_mProcess, "sync", rb_process_sync, 0);
}

