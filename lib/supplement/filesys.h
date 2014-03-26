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


extern VALUE rb_fsstat_s_alloc( VALUE);
extern VALUE rb_fsstat_init( VALUE, VALUE);
extern VALUE rb_fsstat_init_copy( VALUE, VALUE);

extern VALUE rb_fsstat_type(   VALUE);
extern VALUE rb_fsstat_bsize(  VALUE);
extern VALUE rb_fsstat_blocks( VALUE);
extern VALUE rb_fsstat_bfree(  VALUE);
extern VALUE rb_fsstat_bavail( VALUE);
extern VALUE rb_fsstat_files(  VALUE);
extern VALUE rb_fsstat_ffree(  VALUE);
extern VALUE rb_fsstat_fsid(   VALUE);

extern VALUE rb_fsstat_bytes(  VALUE);
extern VALUE rb_fsstat_free(   VALUE);
extern VALUE rb_fsstat_avail(  VALUE);
extern VALUE rb_fsstat_pfree(  VALUE);
extern VALUE rb_fsstat_pavail( VALUE);

extern VALUE rb_fsstat_inspect( VALUE);

extern void Init_filesys( void);

#endif

