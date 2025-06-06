/*
 *  supplement.c  --  Simple Ruby Extensions
 */

#include "supplement.h"

#include "process.h"

#include <ruby/st.h>
#include <ruby/io.h>
#include <ruby/re.h>

#ifdef FEATURE_THREAD_EXCLUSIVE
#ifdef HAVE_HEADER_RUBYSIG_H
#include <rubysig.h>
#endif
#endif


static void  supplement_ary_assure_notempty( VALUE);
static VALUE supplement_index_blk( VALUE);
static VALUE supplement_index_ref( VALUE, VALUE);
static VALUE supplement_rindex_blk( VALUE);
static VALUE supplement_rindex_ref( VALUE, VALUE);
#ifdef FEATURE_ARRAY_INDEX_WITH_BLOCK
static VALUE supplement_index_val( VALUE, VALUE);
static VALUE supplement_rindex_val( VALUE, VALUE);
#endif
static VALUE supplement_do_unumask( VALUE);
#ifdef FEATURE_THREAD_EXCLUSIVE
static VALUE bsruby_set_thread_critical( VALUE);
#endif



static ID id_delete_at;
static ID id_cmp;
static ID id_eqq;
static ID id_mkdir;
static ID id_index;



/*
 *  Document-class: Object
 */

/*
 *  call-seq:
 *     new_string                -> str
 *     new_string { |str| ... }  -> str
 *
 *  Returns another string that may be modified without touching the original
 *  object. This means +dup+ for a string and +to_s+ for any other object.
 *
 *  If a block is given, that may modify a created string (built from a
 *  non-string object). For a dup'ed string object the block will not be
 *  called (See +String#new_string+).
 */

VALUE
rb_obj_new_string( VALUE obj)
{
    VALUE r;

    r = rb_obj_as_string( obj);
    if (OBJ_FROZEN( r))
        r = rb_str_new_string( r);
    if (rb_block_given_p())
        rb_yield( r);
    return r;
}


/*
 *  call-seq:
 *     nil_if val  -> nil or self
 *
 *  Returns +nil+ when the string matches +val+. +val+ is compared using
 *  the <code>===</code> operator, just like in the case statement.
 *
 *     "hello".nil_if "NONE"      #=> "hello"
 *     "NONE".nil_if "NONE"       #=> nil
 *     "NONE".nil_if /^none$/i    #=> nil
 *
 *     20.nil_if 10      #=> 20
 *     10.nil_if 10      #=> nil
 *     1.0.nil_if Float  #=> nil
 *
 */

VALUE
rb_obj_nil_if( VALUE obj, VALUE val)
{
    if (!id_eqq)
        id_eqq = rb_intern( "===");
    return RTEST( rb_funcall( val, id_eqq, 1, obj)) ? Qnil : obj;
}


/*
 *  Document-module: Kernel
 */

#ifdef FEATURE_KERNEL_TAP

/*
 *  call-seq:
 *     tap { |x| ... }    -> obj
 *
 *  Yields <code>x</code> to the block, and then returns <code>x</code>.
 *  The primary purpose of this method is to "tap into" a method chain,
 *  in order to perform operations on intermediate results within the chain.
 *
 *      (1..10)                  .tap { |x| puts "original: #{x.inspect}" }
 *        .to_a                  .tap { |x| puts "array:    #{x.inspect}" }
 *        .select { |x| x%2==0 } .tap { |x| puts "evens:    #{x.inspect}" }
 *        .map { |x| x*x }       .tap { |x| puts "squares:  #{x.inspect}" }
 *
 */

VALUE
rb_krn_tap( VALUE obj)
{
    rb_yield( obj);
    return obj;
}

#endif

/*
 *  call-seq:
 *     tap! { |x| ... }    -> obj
 *
 *  Yields +x+ to the block, and then returns +x+ if and only
 *  if +x+ is not +nil+.
 *
 */

VALUE
rb_krn_tap_bang( VALUE obj)
{
    if (!NIL_P( obj))
        rb_yield( obj);
    return obj;
}


/*
 *  call-seq:
 *     with { |x| ... }    -> obj
 *
 *  Yields +x+ to the block.
 *  This difference to +tap+ is that the block's result will be returned.
 *
 *  Use this to narrow your namespace.
 *
 */

VALUE
rb_krn_with( VALUE obj)
{
    return rb_yield( obj);
}


/*
 *  Document-class: Module
 */

/*
 *  call-seq:
 *     plain_name   -> str
 *
 *  The last part of a `::`-separated Module name path.
 */

VALUE
rb_module_plain_name( VALUE cls)
{
    VALUE n;
    VALUE i;

    n = rb_str_dup( rb_funcall( cls, rb_intern( "name"), 0));
    i = rb_funcall( n, rb_intern( "rindex"), 1, rb_str_new( "::", 2));
    if (!NIL_P( i)) {
        i = rb_funcall( i, rb_intern( "+"), 1, INT2FIX( 2));
        rb_funcall( n, rb_intern( "slice!"), 2, INT2FIX( 0), i);
    }
    return n;
}


/*
 *  Document-class: NilClass
 */

/*
 *  call-seq:
 *     notempty?   -> nil
 *
 *  This spares testing for +nil+ when checking strings.
 */

VALUE
rb_nil_notempty_p( VALUE str)
{
    return Qnil;
}

/*
 *  Document-method: nonzero?
 *
 *  call-seq:
 *     nonzero?   -> nil
 *
 *  This spares testing for +nil+ when checking numbers.
 */

/*
 *  Document-method: each_line
 *
 *  call-seq:
 *     each_line { |l| ... }   -> nil
 *
 *  This spares testing for +nil+ when checking strings.
 */

VALUE
rb_nil_each_line( VALUE str)
{
    RETURN_ENUMERATOR( str, 0, 0);
    return Qnil;
}



/*
 *  Document-class: String
 */


/*
 *  call-seq:
 *     new_string     -> str
 *
 *  Returns another string that may be modified without touching the original
 *  object. This means +dup+ for a string and +to_s+ for any other object.
 *
 */

VALUE
rb_str_new_string( VALUE obj)
{
    return rb_str_dup( obj);
}


/*
 *  call-seq:
 *     notempty?   -> nil or self
 *
 *  Returns <code>self</code> if and only if <code>str</code> is not
 *  empty, <code>nil</code> otherwise.
 *
 *     "hello".notempty?   #=> "hello"
 *     "".notempty?        #=> nil
 */

VALUE
rb_str_notempty_p( VALUE str)
{
#if 0
    /* Ruby Coding style */
    if (RSTRING_LEN( str) == 0)
        return Qnil;
    return str;
#else
    return RSTRING_LEN( str) ? str : Qnil;
#endif
}


/*
 *  call-seq:
 *     eat( n = nil)   -> str
 *
 *  Returns first <code>n</code> characters of <code>self</code> or
 *  whole string if <code>n</code> is <code>nil</code>. The returned
 *  substring will be deleted from <code>self</code>. If <code>n</code>
 *  is negative, characters will be eaten from the right.
 *
 *     a = "upcase"
 *     a.eat 2             #=> "up"
 *     a                   #=> "case"
 */

VALUE
rb_str_eat( int argc, VALUE *argv, VALUE str)
{
    VALUE val;
    int n;
    int l;
    int r;

    n = l = rb_str_strlen( str);
    if (rb_scan_args( argc, argv, "01", &val) == 1) {
        if (!NIL_P( val)) {
            int v = NUM2INT( val);
            if (v >= 0) {
                if (n >= v) n = v;
            } else {
                n = -n;
                if (n <= v) n = v;
            }
        }
    }
    rb_str_modify( str);
    if (n > 0) {
        r = 0;
    } else if (n < 0) {
        r = l + n;
        n = -n;
    } else
        return Qnil;
    val = rb_str_substr( str, r, n);
    if (!NIL_P(val))
        rb_str_update( str, r, n, rb_str_new( NULL, 0));
    return val;
}


/*
 *  call-seq:
 *     cut!( length)   -> str
 *
 *  Cut string to <code>length</code>. If nothing was removed,
 *  <code>nil</code> is returned.
 *
 *     a = "hello"
 *     a.cut! 4            #=> "hell"
 *     a                   #=> "hell"
 *     a.cut! 4            #=> nil
 */

VALUE
rb_str_cut_bang( VALUE str, VALUE len)
{
    int l;
    int n;

    rb_str_modify( str);
    l = NUM2INT( len);
    if (l < 0)
        l = 0;
    n = rb_str_strlen( str);
    if (l < n) {
        rb_str_update( str, l, n - l, rb_str_new( NULL, 0));
        return str;
    }
    return Qnil;
}


#ifdef FEATURE_STRING_CLEAR

/*
 *  call-seq:
 *     clear   -> self
 *
 *  Set to empty string. Equivalent to <code>str.replace ""</code>.
 *
 *     a = "hello"  #=> "hello"
 *     a.clear      #=> ""
 *     a.empty?     #=> true
 */

VALUE
rb_str_clear( VALUE str)
{
    rb_str_modify( str);
    rb_str_resize( str, 0);
    return str;
}

#endif


/*
 *  call-seq:
 *     head( n = 1)   -> str
 *
 *  Returns first <code>n</code> bytes in <code>str</code>.
 *
 *     "hello".head( 2)    #=> "he"
 */

VALUE
rb_str_head( int argc, VALUE *argv, VALUE str)
{
    VALUE n;
    VALUE str2;
    long len;

    len = rb_scan_args( argc, argv, "01", &n) == 1 ? NUM2LONG( n) : 1;
    return rb_str_substr( str, 0, len);
}


/*
 *  call-seq:
 *     rest( n = 1)   -> str
 *
 *  Return rest after <code>n</code> bytes in <code>str</code>.
 *
 *     "hello".rest( 2)    #=> "llo"
 */

VALUE
rb_str_rest( int argc, VALUE *argv, VALUE str)
{
    VALUE n;
    long l, beg, len;

    beg = rb_scan_args( argc, argv, "01", &n) == 1 ? NUM2LONG( n) : 1;
    if (beg < 0)
        beg = 0;
    l = rb_str_strlen( str);
    return rb_str_substr( str, beg, l - beg);
}


/*
 *  call-seq:
 *     tail( n = 1)   -> str
 *
 *  Returns last <code>n</code> bytes in <code>str</code>.
 *
 *     "hello".tail( 2)    #=> "lo"
 */

VALUE
rb_str_tail( int argc, VALUE *argv, VALUE str)
{
    VALUE n;
    long l, beg, len;

    len = rb_scan_args( argc, argv, "01", &n) == 1 ? NUM2LONG( n) : 1;
    l = rb_str_strlen( str);
    beg = l - len;
    if (beg < 0)
        beg = 0, len = l;
    return rb_str_substr( str, beg, len);
}


#ifdef FEATURE_STRING_START_WITH

/*
 *  call-seq:
 *     start_with?( oth)   -> true or false
 *
 *  Checks whether the head is <code>oth</code>.
 *
 *     "sys-apps".start_with?( "sys-")    #=> true
 */

VALUE
rb_str_start_with_p( VALUE str, VALUE oth)
{
    return NIL_P( rb_str_starts_with_p( str, oth)) ? Qfalse : Qtrue;
}


/*
 *  call-seq:
 *     end_with?( oth)   -> true or false
 *
 *  Checks whether the tail is <code>oth</code>.
 *
 *     "sys-apps".end_with?( "-apps")    #=> true
 */

VALUE
rb_str_end_with_p( VALUE str, VALUE oth)
{
    return NIL_P( rb_str_ends_with_p( str, oth)) ? Qfalse : Qtrue;
}

#endif

/*
 *  call-seq:
 *     starts_with?( oth)   -> nil or int
 *
 *  Checks whether the head is <code>oth</code>. Returns length of
 *  <code>oth</code> when matching.
 *
 *     "sys-apps".starts_with?( "sys-")    #=> 4
 *
 *  Caution! The Ruby 1.9.3 method #start_with? (notice the missing s)
 *  just returns +true+ or +false+.
 */

VALUE
rb_str_starts_with_p( VALUE str, VALUE oth)
{
    long i;
    char *s, *o;
    VALUE ost;

    ost = rb_string_value( &oth);
    i = RSTRING_LEN( ost);
    if (i > RSTRING_LEN( str))
        return Qnil;
    s = RSTRING_PTR( str);
    o = RSTRING_PTR( ost);
    for (; i; i--, s++, o++) {
        if (*s != *o)
            return Qnil;
    }
    return INT2FIX( rb_str_strlen( ost));
}


/*
 *  call-seq:
 *     ends_with?( oth)   -> nil or int
 *
 *  Checks whether the tail is <code>oth</code>. Returns the position
 *  where <code>oth</code> starts when matching.
 *
 *     "sys-apps".ends_with?( "-apps")    #=> 3
 *
 *  Caution! The Ruby 1.9.3 method #start_with? (notice the missing s)
 *  just returns +true+ or +false+.
 */

VALUE
rb_str_ends_with_p( VALUE str, VALUE oth)
{
    long i;
    char *s, *o;
    VALUE ost;

    ost = rb_string_value( &oth);
    i = RSTRING_LEN( ost);
    if (i > RSTRING_LEN( str))
        return Qnil;
    s = RSTRING_END( str);
    o = RSTRING_END( ost);
    for (; i; i--)
        if (*--s != *--o)
            return Qnil;
    return INT2FIX( rb_str_strlen( str) - rb_str_strlen( ost));
}

#ifdef FEATURE_STRING_ORD

/*
 *  call-seq:
 *     ord()   -> nil or int
 *
 *  Returns the ASCII value of the first character, if any.
 *
 *  Caution! For UTF-8 characters, this will return the first byte's value.
 *  This will do no harm in Ruby 1.8 but the standard Ruby 1.9 +String#ord+
 *  returns the first codepoint. Please do not overwrite that method.
 */

VALUE rb_str_ord( VALUE str)
{
    return RSTRING_LEN( str) > 0 ? INT2FIX( RSTRING_PTR( str)[ 0]) : Qnil;
}

#endif

/*
 *  call-seq:
 *     axe( n = 80)   -> str
 *
 *  Cut off everthing beyond then <code>n</code>th character. Replace the
 *  last bytes by ellipses.
 *
 *     a = "Redistribution and use in source and binary forms, with or without"
 *     a.axe( 16)     #=> "Redistributio..."
 */

VALUE
rb_str_axe( int argc, VALUE *argv, VALUE str)
{
    VALUE n;
    VALUE ret;
    long newlen, oldlen;

    if (rb_scan_args( argc, argv, "01", &n) == 1 && !NIL_P( n))
        newlen = NUM2LONG( n);
    else
        newlen = 80;
    if (newlen < 0)
        return Qnil;

    oldlen = rb_str_strlen( str);
    if (newlen < oldlen) {
        VALUE ell;
        long e;

        ell = rb_str_new2( "...");
        e = rb_str_strlen( ell);
        if (newlen > e) {
            ret = rb_str_substr( str, 0, newlen - e);
            rb_str_append( ret, ell);
        } else
            ret = rb_str_substr( str, 0, newlen);
    } else
        ret = str;
    return ret;
}


/*
 *  Document-class: Numeric
 */

/*
 *  call-seq:
 *     grammatical sing, plu  -> str
 *
 *  Singular or plural
 *
 *     1.grammatical "line", "lines"         #=>  "line"
 *     6.grammatical "child", "children"     #=>  "children"
 */

VALUE
rb_num_grammatical( VALUE num, VALUE sing, VALUE plu)
{
    long l;
    double d;

    switch (TYPE( num)) {
        case T_FIXNUM:
            l = NUM2LONG( num);
            if (l == 1l || l == -1l)
                return sing;
            break;

        case T_BIGNUM:
            /* 1 is not a Bignum */
            break;

        case T_FLOAT:
            d = RFLOAT_VALUE( num);
            if (d == 1.0 || d == -1.0)
                return sing;
            break;

        default:
            l = NUM2LONG( rb_funcall( num, id_cmp, 1, INT2FIX( 1)));
            if (l == 0)
                return sing;
            l = NUM2LONG( rb_funcall( num, id_cmp, 1, INT2FIX(-1)));
            if (l == 0)
                return sing;
            break;
    }
    return plu;
}


/*
 *  call-seq:
 *     num.sqrt   -> num
 *
 *  Square root.
 *
 *     144.sqrt       #=> 12.0
 */

VALUE
rb_num_sqrt( VALUE num)
{
    return rb_float_new( sqrt( RFLOAT_VALUE( rb_Float( num))));
}

/*
 *  call-seq:
 *     num.cbrt   -> num
 *
 *  Cube root.
 *
 *     27.cbrt       #=> 3.0
 */

VALUE
rb_num_cbrt( VALUE num)
{
#if HAVE_FUNC_CBRT
    return rb_float_new( cbrt( RFLOAT_VALUE( rb_Float( num))));
#else
    double n;
    int neg;
    int i;

    n = RFLOAT_VALUE( rb_Float( num));
    if ((neg = n < 0))
        n = -n;
    n = sqrt( sqrt( n));
    i = 2;
    for (;;) {
        double w = n;
        int j;

        for (j=i;j;--j) w = sqrt( w);
        i *= 2;
        w *= n;
        if (n == w) break;
        n = w;
    }
    return rb_float_new( neg ? -n : n);
#endif
}


/*
 *  Document-class: Array
 */

/*
 *  call-seq:
 *     notempty?   -> nil or self
 *
 *  Returns <code>self</code> if and only if <code>ary</code> is not
 *  empty, <code>nil</code> otherwise.
 *
 *     %w(a b).notempty?   #=> [ "a", "b"]
 *     [].notempty?        #=> nil
 */

VALUE
rb_ary_notempty_p( VALUE ary)
{
    return RARRAY_LEN( ary) == 0 ? Qnil : ary;
}

/*
 *  call-seq:
 *     first = obj  -> obj
 *
 *  Replace the first element. Equivalent to <code>ary[0]=</code>.
 *
 *    a = [ -1, 39, 56]
 *    a.first = 42
 *    a                            #=> [42, 39, 56]
 *    a.unshift 266
 *    a.first /= 14
 *    a                            #=> [19, 42, 39, 56]
 */

VALUE
rb_ary_first_set( VALUE ary, VALUE val)
{
    rb_ary_modify( ary);
    supplement_ary_assure_notempty( ary);
    RARRAY_ASET( ary, 0, val);
    return val;
}

void
supplement_ary_assure_notempty( VALUE ary)
{
    long len = RARRAY_LEN( ary);
    if (len == 0) {
        VALUE q = Qnil;
        rb_ary_cat( ary, &q, 1);
    }
}

/*
 *  call-seq:
 *     last = obj  -> obj
 *
 *  Replace the last element. Almost equivalent to
 *  <code>ary[-1]=</code> (works also for empty arrays).
 *
 *    a = [ 42, 39, -1]
 *    a.last = 42
 *    a                            #=> [42, 39, 42]
 *    a.push 266
 *    a.last /= 14
 *    a                            #=> [42, 39, 42, 19]
 */

VALUE
rb_ary_last_set( VALUE ary, VALUE val)
{
    rb_ary_modify( ary);
    supplement_ary_assure_notempty( ary);
    RARRAY_ASET( ary, RARRAY_LEN( ary) - 1, val);
    return val;
}


/*
 *  call-seq:
 *     range()    ->  range
 *
 *  Returns the range <code>0...size</code>.
 *
 *     [ "a", "h", "q"].range   #=> 0...3
 */

VALUE
rb_ary_range( VALUE ary)
{
    return rb_range_new( INT2FIX( 0), INT2FIX( RARRAY_LEN( ary)), 1);
}


/*
 *  call-seq:
 *     indexes()  ->  ary
 *     keys()     ->  ary
 *
 *  Returns the indexes from <code>0</code> to
 *  <code>array.length()</code> as an array.
 *
 *     [ "a", "h", "q"].indexes   #=> [ 0, 1, 2]
 */

VALUE
rb_ary_indexes( VALUE ary)
{
    VALUE ret;
    int i, j;

    j = RARRAY_LEN( ary);
    ret = rb_ary_new2( j);
    for (i = 0; j; ++i, --j) {
        rb_ary_push( ret, INT2FIX( i));
    }
    return ret;
}


/*
 *  call-seq:
 *     pick( ref)           -> obj or nil
 *     pick { |elem| ... }  -> obj or nil
 *
 *  Deletes the element where first <code>ref === obj</code> is true or the
 *  <em>block</em> first returns <code>true</code>. The result will be
 *  <code>nil</code> if nothing is found.
 *
 *     a = %w(ant bat cat dog)
 *     a.pick { |e| e =~ /^c/ }  #=> "cat"
 *     a                         #=> ["ant", "bat", "dog"]
 *     a.pick { |e| e =~ /^x/ }  #=> nil
 */

VALUE
rb_ary_pick( int argc, VALUE *argv, VALUE ary)
{
    VALUE ref;
    VALUE pos;
    VALUE p;

    if (rb_scan_args( argc, argv, "01", &ref) == 1)
        pos = supplement_index_ref( ary, ref);
    else
        pos = supplement_index_blk( ary);

    if (!NIL_P( pos))
        return rb_funcall( ary, id_delete_at, 1, pos);
    return Qnil;
}

VALUE
supplement_index_ref( VALUE ary, VALUE ref)
{
    long i, j;

    if (!id_eqq)
        id_eqq = rb_intern( "===");
    for (i = 0, j = RARRAY_LEN( ary); j > 0; i++, j--) {
        if (RTEST( rb_funcall( ref, id_eqq, 1, RARRAY_PTR( ary)[ i])))
            return LONG2NUM( i);
    }
    return Qnil;
}

VALUE
supplement_index_blk( VALUE ary)
{
    long i, j;

    for (i = 0, j = RARRAY_LEN( ary); j > 0; i++, j--) {
        if (RTEST( rb_yield( RARRAY_PTR( ary)[ i])))
            return LONG2NUM( i);
    }
    return Qnil;
}

/*
 *  call-seq:
 *     rpick( ref)           -> obj or nil
 *     rpick { |elem| ... }  -> obj or nil
 *
 *  Deletes the element where first <code>ref === obj</code> is true or the
 *  <em>block</em> first returns <code>true</code>. The result will be
 *  <code>nil</code> if nothing is found. Search from right to left.
 *
 *     a = %w(ant cow bat cat dog)
 *     a.rpick { |e| e =~ /^c/ }  #=> "cat"
 *     a                          #=> ["ant", "cow", "bat", "dog"]
 *     a.rpick { |e| e =~ /^x/ }  #=> nil
 */

VALUE
rb_ary_rpick( int argc, VALUE *argv, VALUE ary)
{
    VALUE ref;
    VALUE pos;
    VALUE p;

    if (rb_scan_args( argc, argv, "01", &ref) == 1)
        pos = supplement_rindex_ref( ary, ref);
    else
        pos = supplement_rindex_blk( ary);

    if (!NIL_P( pos))
        return rb_funcall( ary, id_delete_at, 1, pos);
    return Qnil;
}

VALUE
supplement_rindex_ref( VALUE ary, VALUE ref)
{
    long i;

    if (!id_eqq)
        id_eqq = rb_intern( "===");
    for (i = RARRAY_LEN( ary); i;) {
        --i;
        if (RTEST( rb_funcall( ref, id_eqq, 1, RARRAY_PTR( ary)[ i])))
            return LONG2NUM( i);
    }
    return Qnil;
}

VALUE
supplement_rindex_blk( VALUE ary)
{
    long i;

    for (i = RARRAY_LEN( ary); i;) {
        --i;
        if (RTEST( rb_yield( RARRAY_PTR( ary)[ i])))
            return LONG2NUM( i);
    }
    return Qnil;
}


#ifdef FEATURE_ARRAY_INDEX_WITH_BLOCK

/*
 *  call-seq:
 *     index( obj)              ->  int or nil
 *     index() { |elem| ... }   ->  int or nil
 *
 *  Returns the index of the first object in <code>self</code> such that
 *  is <code>==</code> to <code>obj</code> or the <em>block</em> returns
 *  <code>true</code>. If no match is found, <code>nil</code> is
 *  returned.
 *
 *     a = %w(a b c d e)
 *     a.index("b")               #=> 1
 *     a.index("z")               #=> nil
 *     a.index { |e| e >= "b" }   #=> 1
 *     a.index { |e| e >= "q" }   #=> nil
 */

VALUE
rb_ary_index( int argc, VALUE *argv, VALUE ary)
{
    VALUE val;

    if (rb_scan_args( argc, argv, "01", &val) == 1) {
        if (rb_block_given_p())
            rb_warning( "given block not used");
        return supplement_index_val( ary, val);
    } else
        return supplement_index_blk( ary);
    return Qnil;
}

VALUE
supplement_index_val( VALUE ary, VALUE val)
{
    long i;

    for (i = 0; i < RARRAY_LEN( ary); i++)
        if (rb_equal( RARRAY_PTR( ary)[ i], val))
            return LONG2NUM( i);
    return Qnil;
}


/*
 *  call-seq:
 *     rindex( obj)              ->  int or nil
 *     rindex() { |elem| ... }   ->  int or nil
 *
 *  Returns the index of the first object in <code>self</code> such that
 *  is <code>==</code> to <code>obj</code> or the <em>block</em> returns
 *  <code>true</code>. If no match is found, <code>nil</code> is
 *  returned. Search from right to left.
 *
 *     a = %w(a b c d e)
 *     a.rindex("b")               #=> 1
 *     a.rindex("z")               #=> nil
 *     a.rindex { |e| e >= "b" }   #=> 4
 *     a.rindex { |e| e >= "q" }   #=> nil
 */

VALUE
rb_ary_rindex( int argc, VALUE *argv, VALUE ary)
{
    VALUE val;

    if (rb_scan_args( argc, argv, "01", &val) == 1) {
        if (rb_block_given_p())
            rb_warning( "given block not used");
        return supplement_rindex_val( ary, val);
    } else
        return supplement_rindex_blk( ary);
    return Qnil;
}

VALUE
supplement_rindex_val( VALUE ary, VALUE val)
{
    long i;

    for (i = RARRAY_LEN( ary); i;)
        if (rb_equal( RARRAY_PTR( ary)[ --i], val))
            return LONG2NUM( i);
    return Qnil;
}

#endif


/*
 *  Document-class: Hash
 */

/*
 *  call-seq:
 *     notempty?   -> nil or self
 *
 *  Returns <code>self</code> if and only if <code>hash</code> is not
 *  empty, <code>nil</code> otherwise.
 *
 *     { :a => "A"}.notempty?   #=> { :a => "A"}
 *     {}.notempty?             #=> nil
 */

VALUE
rb_hash_notempty_p( VALUE hash)
{
    return RHASH_SIZE( hash) == 0 ? Qnil : hash;
}


/*
 *  Document-class: File
 */

/*
 *  call-seq:
 *     File.umask()             -> int
 *     File.umask( int)         -> int
 *     File.umask( int) { ... } -> obj
 *
 *  Returns the current umask value for this process.  If the optional
 *  argument is given, set the umask to that value and return the
 *  previous value.  If a block is given, the umask value will be
 *  reset and the blocks value is returned.
 *
 *  Umask values are <em>subtracted</em> from the default permissions,
 *  so a umask of <code>0222</code> would make a file read-only for
 *  everyone.
 *
 *     File.umask( 0006)   #=> 18
 *     File.umask          #=> 6
 */

VALUE
rb_file_s_umask( int argc, VALUE *argv, VALUE file)
{
    int omask = 0;

    switch (argc) {
    case 0:
        omask = umask( 0777);
        umask( omask);
        break;
    case 1:
        omask = umask( NUM2INT( argv[ 0]));
        if (rb_block_given_p())
          return rb_ensure( rb_yield, Qnil,
                              supplement_do_unumask, INT2FIX( omask));
        break;
    default:
        rb_raise( rb_eArgError,
              "wrong number of arguments (%d for 0..1)", argc);
    }
    return INT2FIX( omask);
}

VALUE
supplement_do_unumask( VALUE v)
{
    umask( NUM2INT( v));
    return Qnil;
}


/*
 *  Document-class: Dir
 */

/*
 *  call-seq:
 *     Dir.current()   -> dir
 *
 *  Current directory as a Dir object.
 *
 */

VALUE
rb_dir_s_current( VALUE dir)
{
    VALUE dot = rb_str_new( ".", 1);
    return rb_funcall_passing_block( dir, rb_intern( "open"), 1, &dot);
}


/*
 *  call-seq:
 *     Dir.mkdir!( path, modes = nil)   -> str or nil
 *
 *  Make a directory and all subdirectories if needed.
 *
 *  If you specifiy modes, be sure that you have the permission to create
 *  subdirectories.
 *
 *  Returns the path demanded if the directory was created and +nil+ if
 *  it existed before.
 *
 */

VALUE
rb_dir_s_mkdir_bang( int argc, VALUE *argv, VALUE dir)
{
    VALUE path, modes;

    rb_scan_args( argc, argv, "11", &path, &modes);
    if (!rb_file_directory_p( rb_cFile, path)) {
        VALUE parent[2];

        parent[ 0] = rb_file_dirname( path);
        parent[ 1] = modes;
        rb_dir_s_mkdir_bang( 2, parent, dir);
        if (!id_mkdir)
            id_mkdir = rb_intern( "mkdir");
        if (NIL_P(modes))
            rb_funcall( rb_cDir, id_mkdir, 1, path);
        else
            rb_funcall( rb_cDir, id_mkdir, 2, path, modes);
        return path;
    }
    return Qnil;
}


#ifdef FEATURE_DIR_CHILDREN

/*
 *  call-seq:
 *     children()   -> ary
 *
 *  Entries without <code>"."</code> and <code>".."</code>.
 *
 */

VALUE
rb_dir_children( VALUE self)
{
    VALUE e;

    e = rb_funcall( self, rb_intern( "entries"), 0);
    rb_ary_delete( e, rb_str_new( ".", 1));
    rb_ary_delete( e, rb_str_new( "..", 2));
    return e;
}

#endif



/*
 *  Document-class: Match
 */

/*
 *  call-seq:
 *     begin( n = nil)   -> integer
 *
 *  Returns the offset of the start of the <code>n</code>th element of
 *  the match array in the string. In case <code>n</code> is
 *  <code>nil</code> the 0th match (whole) is assumed.
 *
 *     m = /(.)(.)(\d+)(\d)/.match("THX1138.")
 *     m.begin( 0)  #=> 1
 *     m.begin      #=> 1
 *     m.begin( 2)  #=> 2
 */

VALUE
rb_match_begin( int argc, VALUE *argv, VALUE match)
{
    VALUE val;
    int i;

    if (rb_scan_args( argc, argv, "01", &val) == 1) {
        i = NIL_P( val) ? 0 : NUM2INT( val);
    } else
        i = 0;

    if (i < 0 || RMATCH_REGS( match)->num_regs <= i)
        rb_raise( rb_eIndexError, "index %d out of matches", i);

    if (RMATCH_REGS( match)->beg[i] < 0)
        return Qnil;

    return INT2FIX( RMATCH_REGS( match)->beg[i]);
}


/*
 *  call-seq:
 *     end( n = nil)   -> integer
 *
 *  Returns the offset of the character immediately following the end of
 *  the <code>n</code>th element of the match array in the string. In
 *  case <code>n</code> is <code>nil</code> the 0th match (whole) is
 *  assumed.
 *
 *     m = /(.)(.)(\d+)(\d)/.match("THX1138.")
 *     m.end( 0)  #=> 7
 *     m.end      #=> 7
 *     m.end( 2)  #=> 3
 */

VALUE
rb_match_end( int argc, VALUE *argv, VALUE match)
{
    VALUE val;
    int i;

    if (rb_scan_args( argc, argv, "01", &val) == 1) {
        i = NIL_P( val) ? 0 : NUM2INT( val);
    } else
        i = 0;

    if (i < 0 || RMATCH_REGS( match)->num_regs <= i)
        rb_raise( rb_eIndexError, "index %d out of matches", i);

    if (RMATCH_REGS( match)->beg[i] < 0)
        return Qnil;

    return INT2FIX( RMATCH_REGS( match)->end[i]);
}


/*
 *  call-seq:
 *     fields( ...)   -> ary
 *
 *  Returns the values of some fields.
 *
 *     S = Struct[ :a, :b, :c]
 *     s = S[ "A", "B", "C"]
 *     s.fields( :b, :a)    #=> [ "B", "A"]
 */

VALUE
rb_struct_fields( int argc, VALUE *argv, VALUE strct)
{
    VALUE ary;
    int i;

    ary = rb_ary_new2( argc);
    for (i = 0; i < argc; i++)
        rb_ary_push( ary, rb_struct_aref( strct, argv[ i]));
    return ary;
}


#ifdef FEATURE_THREAD_EXCLUSIVE

/*
 *  Document-class: Thread
 */

/*
 *  call-seq:
 *     Thread.exclusive { ... }  -> obj
 *
 *  Sets the global ``thread critical'' condition temporarily. Return
 *  value is the object returned by the <em>block</em>.
 */

VALUE
rb_thread_exclusive( void)
{
    VALUE old_tc = rb_thread_critical;

    rb_thread_critical = Qtrue;
    return rb_ensure( rb_yield, Qnil, bsruby_set_thread_critical, old_tc);
}

VALUE
bsruby_set_thread_critical( VALUE c)
{
    rb_thread_critical = c;
    return Qnil;
}

#endif



/*
 *  Document-class: Struct
 */

/*
 *  Document-method: []
 *
 *  call-seq:
 *     Struct[ ...]  -> cls
 *
 *  Alias for Struct.new; This applies to Struct subclass generation
 *  as well as to the subclasses instance creation.
 *
 *    S = Struct[ :a, :b]    #=>  S
 *    s = S[ 'A', 'B']       #=>  #<struct S a="A", b="B">
 *
 */



/*
 *  Document-class: Object
 */

/*
 *  call-seq:
 *     to_bool -> true
 *
 *  Returns true for anything but +nil+ and +false+.
 */

VALUE
rb_nil_to_bool( VALUE obj)
{
    return Qfalse;
}

VALUE
rb_obj_to_bool( VALUE obj)
{
    return Qtrue;
}



void Init_supplement( void)
{
    rb_define_method( rb_cObject, "new_string", rb_obj_new_string, 0);
    rb_define_method( rb_cObject, "nil_if", rb_obj_nil_if, 1);
#ifdef FEATURE_KERNEL_TAP
    rb_define_method( rb_mKernel, "tap", rb_krn_tap, 0);
#endif
    rb_define_method( rb_mKernel, "tap!", rb_krn_tap_bang, 0);
    rb_define_method( rb_mKernel, "with", rb_krn_with, 0);

    rb_define_method( rb_cModule, "plain_name", rb_module_plain_name, 0);

    rb_define_method( rb_cNilClass, "notempty?", rb_nil_notempty_p, 0);
    rb_define_method( rb_cNilClass, "nonzero?", rb_nil_notempty_p, 0);
    rb_define_method( rb_cNilClass, "each_line", rb_nil_each_line, 0);

    rb_define_method( rb_cString, "new_string", rb_str_new_string, 0);
    rb_define_method( rb_cString, "notempty?", rb_str_notempty_p, 0);
    rb_define_method( rb_cString, "eat", rb_str_eat, -1);
    rb_define_method( rb_cString, "cut!", rb_str_cut_bang, 1);
#ifdef FEATURE_STRING_CLEAR
    rb_define_method( rb_cString, "clear", rb_str_clear, 0);
#endif
    rb_define_method( rb_cString, "head", rb_str_head, -1);
    rb_define_method( rb_cString, "rest", rb_str_rest, -1);
    rb_define_method( rb_cString, "tail", rb_str_tail, -1);
#ifdef FEATURE_STRING_START_WITH
    rb_define_method( rb_cString, "start_with?", rb_str_start_with_p, 1);
    rb_define_method( rb_cString, "end_with?", rb_str_end_with_p, 1);
#endif
    rb_define_method( rb_cString, "starts_with?", rb_str_starts_with_p, 1);
    rb_define_method( rb_cString, "ends_with?", rb_str_ends_with_p, 1);
    rb_define_alias(  rb_cString, "starts_with", "start_with?");
    rb_define_alias(  rb_cString, "ends_with", "end_with?");
#ifdef FEATURE_STRING_ORD
    rb_define_method( rb_cString, "ord", rb_str_ord, 0);
#endif
    rb_define_method( rb_cString, "axe", rb_str_axe, -1);

    rb_define_method( rb_cNumeric, "grammatical", rb_num_grammatical, 2);
    rb_define_method( rb_cNumeric, "sqrt", rb_num_sqrt, 0);
    rb_define_method( rb_cNumeric, "cbrt", rb_num_cbrt, 0);

    rb_define_method( rb_cArray, "notempty?", rb_ary_notempty_p, 0);
    rb_define_method( rb_cArray, "first=", rb_ary_first_set, 1);
    rb_define_method( rb_cArray, "last=", rb_ary_last_set, 1);
    rb_define_method( rb_cArray, "indexes", rb_ary_indexes, 0);
    rb_define_alias(  rb_cArray, "keys", "indexes");
    rb_define_method( rb_cArray, "range", rb_ary_range, 0);
    rb_define_method( rb_cArray, "pick", rb_ary_pick, -1);
    rb_define_method( rb_cArray, "rpick", rb_ary_rpick, -1);
#ifdef FEATURE_ARRAY_INDEX_WITH_BLOCK
    rb_define_method( rb_cArray, "index", rb_ary_index, -1);
    rb_define_method( rb_cArray, "rindex", rb_ary_rindex, -1);
#endif

    rb_define_method( rb_cHash, "notempty?", rb_hash_notempty_p, 0);

    rb_define_singleton_method( rb_cFile, "umask", rb_file_s_umask, -1);

    rb_define_singleton_method( rb_cDir, "current", rb_dir_s_current, 0);
    rb_define_singleton_method( rb_cDir, "mkdir!", rb_dir_s_mkdir_bang, -1);
#ifdef FEATURE_DIR_CHILDREN
    rb_define_method( rb_cDir, "children", rb_dir_children, 0);
#endif
    rb_define_alias(  rb_cDir, "entries!", "children");

    rb_define_method( rb_cMatch, "begin", rb_match_begin, -1);
    rb_define_method( rb_cMatch, "end", rb_match_end, -1);

#ifdef FEATURE_THREAD_EXCLUSIVE
    rb_define_singleton_method( rb_cThread, "exclusive",
                                                    rb_thread_exclusive, 0);
#endif

    rb_define_alias( rb_singleton_class( rb_cStruct), "[]", "new");
    rb_define_method( rb_cStruct, "fields", rb_struct_fields, -1);
    rb_define_alias(  rb_cStruct, "fetch_values", "fields");

    rb_define_method( rb_cNilClass, "to_bool", rb_nil_to_bool, 0);
    rb_define_method( rb_cFalseClass, "to_bool", rb_nil_to_bool, 0);
    rb_define_method( rb_cObject, "to_bool", rb_obj_to_bool, 0);

    id_delete_at   = rb_intern( "delete_at");
    id_cmp         = rb_intern( "<=>");
    id_eqq         = 0;
    id_mkdir       = 0;
    id_index       = 0;

    Init_supplement_process();
}

