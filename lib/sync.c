/*
 *  sync.c  --  Sync all
 */


#include "sync.h"

#include <unistd.h>


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
