/*
 *  sync.c  --  Sync all
 */


#include "sync.h"

#include <ruby.h>
#include <unistd.h>


static VALUE rb_process_sync( VALUE obj);


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


void Init_step_sync( void)
{
    rb_define_singleton_method( rb_mProcess, "sync", rb_process_sync, 0);
}

