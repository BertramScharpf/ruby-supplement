/*
 *  step.h  --  Simple Ruby Extensions
 */


#ifndef __STEP_H__
#define __STEP_H__

#include <ruby.h>


extern VALUE rb_krn_tap( VALUE);
extern VALUE rb_krn_tap_bang( VALUE);
extern VALUE rb_str_notempty( VALUE);
extern VALUE rb_str_nil_if( VALUE, VALUE);
extern VALUE rb_str_eat( int, VALUE *, VALUE);
extern VALUE rb_str_eat_lines( VALUE);
extern VALUE rb_str_cut_bang( VALUE, VALUE);
extern VALUE rb_str_clear( VALUE);
extern VALUE rb_str_head( int, VALUE *, VALUE);
extern VALUE rb_str_rest( int, VALUE *, VALUE);
extern VALUE rb_str_tail( int, VALUE *, VALUE);
extern VALUE rb_str_start_with_q( VALUE, VALUE);
extern VALUE rb_str_end_with_q( VALUE, VALUE);
#ifdef STRING_ORD
extern VALUE rb_str_ord( VALUE);
#endif
extern VALUE rb_ary_notempty( VALUE);
extern VALUE rb_ary_indexes( VALUE);
extern VALUE rb_ary_pick( VALUE);
extern VALUE rb_ary_rpick( VALUE);
#ifdef ARRAY_INDEX_WITH_BLOCK
extern VALUE rb_ary_index( int, VALUE *, VALUE);
extern VALUE rb_ary_rindex( int, VALUE *, VALUE);
#endif
extern VALUE rb_ary_eat_lines( VALUE);
extern VALUE rb_num_nil_if( VALUE, VALUE);
extern VALUE rb_num_neg_p( VALUE);
extern VALUE rb_num_grammatical( VALUE, VALUE, VALUE);
extern VALUE rb_hash_notempty( VALUE);
extern VALUE rb_file_size( VALUE);
extern VALUE rb_file_flockb( int, VALUE *, VALUE);
extern VALUE rb_file_eat_lines( VALUE);
extern VALUE step_file_s_umask( int, VALUE *);
extern VALUE rb_match_begin( int, VALUE *, VALUE);
extern VALUE rb_match_end( int, VALUE *, VALUE);
extern VALUE rb_nil_notempty( VALUE);
extern VALUE rb_nil_nil_if( VALUE, VALUE);

extern void Init_step( void);

#endif

