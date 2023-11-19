/*
 *  supplement.h  --  Simple Ruby Extensions
 */


#ifndef __SUPPELEMENT_H__
#define __SUPPELEMENT_H__

#if   HAVE_HEADER_RUBY_H
    #include <ruby.h>
#elif HAVE_HEADER_RUBY_RUBY_H
    #include <ruby/ruby.h>
#endif


extern VALUE rb_obj_new_string( VALUE);
extern VALUE rb_obj_nil_if( VALUE, VALUE);
#ifdef FEATURE_KERNEL_TAP
extern VALUE rb_krn_tap( VALUE);
#endif
extern VALUE rb_krn_tap_bang( VALUE);
extern VALUE rb_krn_with( VALUE);

extern VALUE rb_nil_notempty_p( VALUE);
extern VALUE rb_nil_each_line( VALUE);

extern VALUE rb_str_new_string( VALUE);
extern VALUE rb_str_notempty_p( VALUE);
extern VALUE rb_str_eat( int, VALUE *, VALUE);
extern VALUE rb_str_cut_bang( VALUE, VALUE);
#ifdef FEATURE_STRING_CLEAR
extern VALUE rb_str_clear( VALUE);
#endif
extern VALUE rb_str_head( int, VALUE *, VALUE);
extern VALUE rb_str_rest( int, VALUE *, VALUE);
extern VALUE rb_str_tail( int, VALUE *, VALUE);
#ifdef FEATURE_STRING_START_WITH
extern VALUE rb_str_start_with_p( VALUE, VALUE);
extern VALUE rb_str_end_with_p( VALUE, VALUE);
#endif
extern VALUE rb_str_starts_with_p( VALUE, VALUE);
extern VALUE rb_str_ends_with_p( VALUE, VALUE);
#ifdef FEATURE_STRING_ORD
extern VALUE rb_str_ord( VALUE);
#endif
extern VALUE rb_str_axe( int, VALUE *, VALUE);

extern VALUE rb_num_pos_p( VALUE);
extern VALUE rb_num_neg_p( VALUE);
extern VALUE rb_num_grammatical( VALUE, VALUE, VALUE);
extern VALUE rb_num_sqrt( VALUE);
extern VALUE rb_num_cbrt( VALUE);

extern VALUE rb_ary_notempty_p( VALUE);
extern VALUE rb_ary_indexes( VALUE);
extern VALUE rb_ary_range( VALUE);
extern VALUE rb_ary_pick(  int, VALUE *, VALUE);
extern VALUE rb_ary_rpick( int, VALUE *, VALUE);
#ifdef FEATURE_ARRAY_INDEX_WITH_BLOCK
extern VALUE rb_ary_index( int, VALUE *, VALUE);
extern VALUE rb_ary_rindex( int, VALUE *, VALUE);
#endif
#ifdef FEATURE_ARRAY_SELECT_BANG
extern VALUE rb_ary_select_bang( VALUE);
#endif

extern VALUE rb_hash_notempty_p( VALUE);

#ifdef FEATURE_FILE_SIZE
extern VALUE rb_file_size( VALUE);
#endif
extern VALUE rb_file_s_umask( int, VALUE *, VALUE);
extern VALUE rb_dir_s_current( VALUE);
extern VALUE rb_dir_s_mkdir_bang( int, VALUE *, VALUE);
extern VALUE rb_dir_children( VALUE);

extern VALUE rb_match_begin( int, VALUE *, VALUE);
extern VALUE rb_match_end( int, VALUE *, VALUE);

#ifdef FEATURE_THREAD_EXCLUSIVE
extern VALUE rb_thread_exclusive( void);
#endif

extern VALUE rb_struct_fields( int, VALUE *, VALUE);

extern VALUE rb_nil_to_bool( VALUE);
extern VALUE rb_obj_to_bool( VALUE);

extern void Init_supplement( void);

#endif

