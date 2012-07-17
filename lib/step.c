/*
 *  step.c  --  Simple Ruby Extensions
 */

#include "step.h"

#include <st.h>
#include <rubyio.h>
#include <re.h>

#include <sys/stat.h>
#include <sys/file.h>


static VALUE step_index_blk( VALUE);
static VALUE step_rindex_blk( VALUE);
#ifdef ARRAY_INDEX_WITH_BLOCK
static VALUE step_index_val( VALUE, VALUE);
static VALUE step_rindex_val( VALUE, VALUE);
#endif
static VALUE step_eat_lines_elem( VALUE);
static VALUE step_each_line_elem( VALUE);
static VALUE step_do_unumask( VALUE);


static ID id_delete_at;
static ID id_cmp;
static ID id_eat_lines;
static ID id_eqq;


struct step_flock {
    struct step_flock *prev;
    VALUE              file;
    int                op;
    int                last_op;
};

static struct step_flock *flocks_root = NULL;

static void  step_init_flock( struct step_flock *, VALUE, VALUE);
static VALUE step_do_unflock( VALUE);



/*
 *  Document-class: Object
 */

/*
 *  call-seq:
 *     new_string     -> str
 *
 *  Returns another string that may be modified without touching the original
 *  object. This means +dup+ for a string and +to_s+ for any other object.
 *
 *  If a block is given, that may modify a created string (built from a
 *  non-string object). For a dup'ed string object the block will not be
 *  called.
 */

VALUE
rb_obj_new_string( VALUE obj)
{
    VALUE r;

    r = rb_obj_as_string( obj);
    if (rb_block_given_p())
        rb_yield( r);
    return r;
}


/*
 *  Document-module: Kernel
 */

#ifdef KERNEL_TAP

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
 *     nil_if val  -> nil or self
 *
 *  Returns +nil+ when the string matches +val+. +val+ is compared using
 *  the <code>===</code> operator, just like in the case statement.
 *
 *     "hello".nil_if "NONE"      #=> "hello"
 *     "NONE".nil_if "NONE"       #=> nil
 *     "NONE".nil_if /^none$/i    #=> nil
 */

VALUE
rb_obj_nil_if( VALUE obj, VALUE val)
{
    if (!id_eqq)
        id_eqq = rb_intern( "===");
    return RTEST( rb_funcall( val, id_eqq, 1, obj)) ? Qnil : obj;
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

    n = l = RSTRING_LEN( str);
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
        r = l - n;
        val = rb_str_new5( str, RSTRING_PTR( str), n);
        memmove( RSTRING_PTR( str), RSTRING_PTR( str) + n, r);
    } else {
        r = l + n;
        val = rb_str_new5( str, RSTRING_PTR( str) + r, -n);
    }
    RSTRING_LEN( str) = r;
    OBJ_INFECT( val, str);
    return val;
}


/*
 *  Document-method: eat_lines
 *
 *  call-seq:
 *     eat_lines() { |l| .. }   -> nil
 *
 *  Returns parts of +str+, line by line. The line returned will be deleted
 *  from the string. If no +break+ occurs, the string will be empty
 *  afterwards.
 *
 *     a = "foo\nbar\nbaz"
 *     a.eat_lines { |l| break l }   #=> "foo\n"
 *     a                             #=> "bar\nbaz"
 */

VALUE
rb_str_eat_lines( VALUE self)
{
    VALUE val;
    int l, n;
    char *p;

    rb_str_modify( self);
    RETURN_ENUMERATOR( self, 0, 0);
    while ((l = RSTRING_LEN( self)) > 0) {
        p = RSTRING_PTR( self);
        /* "\n" and "\r\n" both end in "\n" */
        for (n = 0; l > 0 && *p != '\n'; n++, l--, p++)
            ;
        if (l > 0)
            n++, l--;
        val = rb_str_new5( self, RSTRING_PTR( self), n);
        /* It would be faster if the pointer could be moved ... */
        memmove( RSTRING_PTR( self), RSTRING_PTR( self) + n, l);
        RSTRING_LEN( self) = l;
        rb_yield( val);
    }
    return Qnil;
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
    int l = NUM2INT( len);

    rb_str_modify( str);
    if (l < RSTRING_LEN( str)) {
        RSTRING_LEN( str) = l;
        return str;
    }
    return Qnil;
}


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

    if (len < 0)
        return Qnil;
    if (len > RSTRING_LEN( str))
        len = RSTRING_LEN( str);
    str2 = rb_str_new5( str, RSTRING_PTR( str), len);
    OBJ_INFECT( str2, str);

    return str2;
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
    VALUE str2;
    long beg, len;

    beg = rb_scan_args( argc, argv, "01", &n) == 1 ? NUM2LONG( n) : 1;

    if (beg > RSTRING_LEN( str))
        return Qnil;
    if (beg < 0)
        beg = 0;
    len = RSTRING_LEN( str) - beg;
    str2 = rb_str_new5( str, RSTRING_PTR( str) + beg, len);
    OBJ_INFECT( str2, str);

    return str2;
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
    VALUE str2;
    long beg, len;

    len = rb_scan_args( argc, argv, "01", &n) == 1 ? NUM2LONG( n) : 1;

    if (len < 0)
        return Qnil;
    beg = RSTRING_LEN( str) - len;
    if (beg < 0) {
        beg = 0;
        len = RSTRING_LEN( str);
    }
    str2 = rb_str_new5( str, RSTRING_PTR( str) + beg, len);
    OBJ_INFECT( str2, str);

    return str2;
}


/*
 *  call-seq:
 *     start_with?( oth)   -> nil or int
 *
 *  Checks whether the head is <code>oth</code>. Returns length of
 *  <code>oth</code> when matching.
 *
 *     "sys-apps".start_with?( "sys-")    #=> 4
 */

VALUE
rb_str_start_with_p( VALUE str, VALUE oth)
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
    return INT2FIX( RSTRING_LEN( ost));
}


/*
 *  call-seq:
 *     end_with?( oth)   -> nil or int
 *
 *  Checks whether the tail is <code>oth</code>. Returns the position
 *  where <code>oth</code> starts when matching.
 *
 *     "sys-apps".end_with?( "-apps")    #=> 3
 */

VALUE
rb_str_end_with_p( VALUE str, VALUE oth)
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
    return INT2FIX( RSTRING_LEN( str) - RSTRING_LEN( ost));
}

#ifdef STRING_ORD

/*
 *  call-seq:
 *     ord()   -> nil or int
 *
 *  Returns the ASCII value of the first character, if any.
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
    VALUE str2;
    long len;
    int x;

    if (rb_scan_args( argc, argv, "01", &n) == 1 && !NIL_P( n))
        len = NUM2LONG( n);
    else
        len = 80;

    if (len < 0)
        return Qnil;
    x = len < RSTRING_LEN( str) ? 3 : 0;
    if (len > RSTRING_LEN( str))
        len = RSTRING_LEN( str);
    str2 = rb_str_new5( str, RSTRING_PTR( str), len);
    for (; x && len; --x)
        RSTRING_PTR( str2)[ --len] = '.';
    OBJ_INFECT( str2, str);

    return str2;
}


/*
 *  Document-class: Numeric
 */

/*
 *  Document-method: nil_if
 *
 *  call-seq:
 *     nil_if val  -> nil or self
 *
 *  Returns +nil+ when the number is equal to +val+. +val+ is compared using
 *  the <code>===</code> operator, just like in the case statement.
 *
 *     20.nil_if 10      #=> 20
 *     10.nil_if 10      #=> nil
 *     1.0.nil_if Float  #=> nil
 */


/*
 *  call-seq:
 *     pos?  ->  true or false
 *
 *  Check whether +num+ is positive.
 *
 */

VALUE
rb_num_pos_p( VALUE num)
{
    VALUE r = Qfalse;

    switch (TYPE( num)) {
        case T_FIXNUM:
            if (NUM2LONG( num) > 0)
                r = Qtrue;
            break;

        case T_BIGNUM:
            if (RBIGNUM( num)->sign)  /* 0 is not a Bignum. */
                r = Qtrue;
            break;

        case T_FLOAT:
            if (RFLOAT( num)->value > 0)
                r = Qtrue;
            break;

        default:
            return rb_num_neg_p( rb_funcall( INT2FIX( 0), id_cmp, 1, num));
            break;
    }
    return r;
}

/*
 *  call-seq:
 *     neg?  ->  true or false
 *
 *  Check whether +num+ is negative.
 *
 */

VALUE
rb_num_neg_p( VALUE num)
{
    VALUE r = Qfalse;

    switch (TYPE( num)) {
        case T_FIXNUM:
            if (NUM2LONG( num) < 0)
                r = Qtrue;
            break;

        case T_BIGNUM:
            if (!RBIGNUM( num)->sign)  /* 0 is not a Bignum. */
                r = Qtrue;
            break;

        case T_FLOAT:
            if (RFLOAT( num)->value < 0)
                r = Qtrue;
            break;

        default:
            return rb_num_neg_p( rb_funcall( num, id_cmp, 1, INT2FIX( 0)));
            break;
    }
    return r;
}

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
            d = RFLOAT( num)->value;
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
    return RARRAY( ary)->len == 0 ? Qnil : ary;
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

    j = RARRAY( ary)->len;
    ret = rb_ary_new2( j);
    RARRAY( ret)->len = j;
    for (i = 0; j; ++i, --j) {
        rb_ary_store( ret, i, INT2FIX( i));
    }
    return ret;
}


/*
 *  call-seq:
 *     pick { |elem| ... }  -> obj or nil
 *
 *  Deletes the element where the <em>block</em> first returns
 *  <code>true</code>. Or <code>nil</code> if nothing is found.
 *
 *     a = %w(ant bat cat dog)
 *     a.pick { |e| e =~ /^c/ }  #=> "cat"
 *     a                         #=> ["ant", "bat", "dog"]
 *     a.pick { |e| e =~ /^x/ }  #=> nil
 */

VALUE
rb_ary_pick( VALUE ary)
{
    VALUE pos;

    pos = step_index_blk( ary);
    if (!NIL_P( pos))
        return rb_funcall( ary, id_delete_at, 1, pos);
    return Qnil;
}

VALUE
step_index_blk( VALUE ary)
{
    long i, j;

    for (i = 0, j = RARRAY( ary)->len; j > 0; i++, j--) {
        if (RTEST( rb_yield( RARRAY( ary)->ptr[ i])))
            return LONG2NUM( i);
    }
    return Qnil;
}

/*
 *  call-seq:
 *     rpick { |elem| ... }  -> obj or nil
 *
 *  Deletes the element where the <em>block</em> first returns
 *  <code>true</code>. Or <code>nil</code> if nothing is found. Search
 *  from right to left.
 *
 *     a = %w(ant cow bat cat dog)
 *     a.rpick { |e| e =~ /^c/ }  #=> "cat"
 *     a                          #=> ["ant", "cow", "bat", "dog"]
 *     a.rpick { |e| e =~ /^x/ }  #=> nil
 */

VALUE
rb_ary_rpick( VALUE ary)
{
    VALUE pos;

    pos = step_rindex_blk( ary);
    if (!NIL_P( pos))
        return rb_funcall( ary, id_delete_at, 1, pos);
    return Qnil;
}

VALUE
step_rindex_blk( VALUE ary)
{
    long i;

    for (i = RARRAY( ary)->len; i;) {
        --i;
        if (rb_yield( RARRAY( ary)->ptr[ i]))
            return LONG2NUM( i);
    }
    return Qnil;
}


#ifdef ARRAY_INDEX_WITH_BLOCK

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
        return step_index_val( ary, val);
    } else
        return step_index_blk( ary);
    return Qnil;
}

VALUE
step_index_val( VALUE ary, VALUE val)
{
    long i;

    for (i = 0; i < RARRAY( ary)->len; i++)
        if (rb_equal( RARRAY( ary)->ptr[ i], val))
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
        return step_rindex_val( ary, val);
    } else
        return step_rindex_blk( ary);
    return Qnil;
}

VALUE
step_rindex_val( VALUE ary, VALUE val)
{
    long i;

    for (i = RARRAY( ary)->len; i;)
        if (rb_equal( RARRAY( ary)->ptr[ --i], val))
            return LONG2NUM( i);
    return Qnil;
}

#endif

/*
 *  Document-method: eat_lines
 *
 *  call-seq:
 *     eat_lines   -> ary
 *
 *  Do +eat_lines+ for each member. Members are typically strings and
 *  files. You don't need to +flatten+ as array lines will be eaten, too.
 *
 */

VALUE
rb_ary_eat_lines( VALUE self)
{
    long i, j;

    /*
    RETURN_ENUMERATOR( self, 0, 0);
    */
    for (i = 0, j = RARRAY( self)->len; j > 0; i++, j--)
        rb_iterate( &step_eat_lines_elem, RARRAY( self)->ptr[ i],
                            &rb_yield, Qnil);
    return Qnil;
}

VALUE step_eat_lines_elem( VALUE elem)
{
    rb_funcall( elem, id_eat_lines, 0);
    return Qnil;
}


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
    return RHASH( hash)->tbl->num_entries == 0 ? Qnil : hash;
}


/*
 *  Document-class: IO
 */

/*
 *  Document-method: eat_lines
 *
 *  call-seq:
 *     eat_lines   -> nil
 *
 *  Same as +IO#each_line+ but returns +nil+ so that +break+ values
 *  may be distinguished.
 *
 */
VALUE
rb_io_eat_lines( VALUE self)
{
    rb_iterate( &step_each_line_elem, self, &rb_yield, Qnil);
    return Qnil;
}

VALUE
step_each_line_elem( VALUE elem)
{
    rb_funcall( elem, rb_intern( "each_line"), 0);
    return Qnil;
}


/*
 *  Document-class: File
 */

/*
 *  call-seq:
 *     size   -> integer
 *
 *  Returns <code>file</code>'s size. A shortcut for
 *  <code>file.stat.size</code>. This constitutes consistency with
 *  <code>StringIO</code>.
 *
 *     file.size     #=> 16384
 */

VALUE
rb_file_size( VALUE obj)
{
    OpenFile *fptr;
    struct stat st;

    GetOpenFile( obj, fptr);
    if (fstat( fileno( fptr->f), &st) == -1) {
        rb_sys_fail( fptr->path);
    }
    return INT2FIX( st.st_size);
}


/*
 *  call-seq:
 *     flockb( excl = nil, nb = nil) { || ... }  -> nil
 *
 *  Lock file using the <code>flock()</code> system call.
 *  When the <code>nb</code> flag is <code>true</code>, the method
 *  won't block but rather raise an exception. Catch
 *  <code>SystemCallError</code>. The calls may be nested in any order.
 *
 *    File.open "/var/mail/joe", "a" do  |f|
 *      f.flockb true do
 *        f.write another_message
 *      end
 *    end
 */

VALUE
rb_file_flockb( int argc, VALUE *argv, VALUE file)
{
    VALUE excl, nb;
    struct step_flock cur_flock;
    OpenFile *fptr;
    int fd;
    int op;

    rb_scan_args( argc, argv, "02", &excl, &nb);
    step_init_flock( &cur_flock, file, excl);

    GetOpenFile( file, fptr);
    fd = fileno( fptr->f);

    op = cur_flock.op | LOCK_NB;
    while (flock( fd, op) < 0) {
        switch (errno) {
          case EAGAIN:
          case EACCES:
#if defined( EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
          case EWOULDBLOCK:
#endif
            if (!RTEST( nb)) {
                rb_thread_polling();        /* busy wait */
                rb_io_check_closed( fptr);
                continue;
            }
            /* fall through */
          default:
            rb_sys_fail( fptr->path);
        }
    }
    cur_flock.prev = flocks_root;
    flocks_root = &cur_flock;
    return rb_ensure( rb_yield, Qnil, step_do_unflock, Qnil);
}

void
step_init_flock( struct step_flock *s, VALUE file, VALUE excl)
{
    struct step_flock *i;

    s->file = file;

    s->last_op = LOCK_UN;
    for (i = flocks_root; i != NULL; i = i->prev) {
        if (i->file == file) {
            s->last_op = i->op;
            break;
        }
    }

    switch (s->last_op) {
        case LOCK_UN:
        case LOCK_SH:
            s->op = RTEST( excl) ? LOCK_EX : LOCK_SH;
            break;
        case LOCK_EX:
            s->op = LOCK_EX;
            break;
        default:
            s->op = LOCK_UN;  /* should never be reached. */
            break;
    }
}

VALUE
step_do_unflock( VALUE v)
{
    OpenFile *fptr;
    int fd;

    GetOpenFile( flocks_root->file, fptr);
    fd = fileno( fptr->f);
    flock( fd, flocks_root->last_op);

    flocks_root = flocks_root->prev;

    return Qnil;
}


/*
 *  call-seq:
 *     umask()             -> int
 *     umask( int)         -> int
 *     umask( int) { ... } -> obj
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
step_file_s_umask( int argc, VALUE *argv)
{
    int omask = 0;

    rb_secure( 2);
    switch (argc) {
    case 0:
        omask = umask( 0777);
        umask( omask);
        break;
    case 1:
        omask = umask( NUM2INT( argv[ 0]));
        if (rb_block_given_p())
          return rb_ensure( rb_yield, Qnil, step_do_unumask, INT2FIX( omask));
        break;
    default:
        rb_raise( rb_eArgError,
              "wrong number of arguments (%d for 0..1)", argc);
    }
    return INT2FIX( omask);
}

VALUE
step_do_unumask( VALUE v)
{
    umask( NUM2INT( v));
    return Qnil;
}


/*
 *  Document-class: Dir
 */

/*
 *  call-seq:
 *     current   -> dir
 *
 *  Current directory as a Dir object.
 *
 */
VALUE
step_dir_s_current( VALUE dir)
{
    return rb_funcall( dir, rb_intern( "new"), 1, rb_str_new( ".", 1));
}

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

    if (i < 0 || RMATCH( match)->regs->num_regs <= i)
        rb_raise( rb_eIndexError, "index %d out of matches", i);

    if (RMATCH( match)->regs->beg[i] < 0)
        return Qnil;

    return INT2FIX( RMATCH( match)->regs->beg[i]);
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

    if (i < 0 || RMATCH( match)->regs->num_regs <= i)
        rb_raise( rb_eIndexError, "index %d out of matches", i);

    if (RMATCH( match)->regs->beg[i] < 0)
        return Qnil;

    return INT2FIX( RMATCH( match)->regs->end[i]);
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
 *  call-seq:
 *     nil_if val   -> nil
 *
 *  This spares testing for +nil+ when checking strings.
 */

VALUE
rb_nil_nil_if( VALUE str, VALUE val)
{
    return Qnil;
}

/*
 *  Document-method: each_line
 *
 *  call-seq:
 *     each_line { |l| ... }   -> nil
 *
 *  This spares testing for +nil+ when checking strings.
 */

/*
 *  Document-method: eat_lines
 *
 *  call-seq:
 *     eat_lines { |l| ... }   -> nil
 *
 *  This spares testing for +nil+ when checking strings.
 */


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


void Init_step( void)
{
    rb_define_alias(  rb_cObject, "cls", "class");
    rb_define_method( rb_cObject, "new_string", rb_obj_new_string, 0);
#ifdef KERNEL_TAP
    rb_define_method( rb_mKernel, "tap", rb_krn_tap, 0);
#endif
    rb_define_method( rb_mKernel, "tap!", rb_krn_tap_bang, 0);

    rb_define_method( rb_cString, "new_string", rb_str_new_string, 0);
    rb_define_method( rb_cString, "notempty?", rb_str_notempty_p, 0);
    rb_define_method( rb_cString, "nil_if", rb_obj_nil_if, 1);
    rb_define_method( rb_cString, "eat", rb_str_eat, -1);
    rb_define_method( rb_cString, "eat_lines", rb_str_eat_lines, 0);
    rb_define_method( rb_cString, "cut!", rb_str_cut_bang, 1);
    rb_define_method( rb_cString, "clear", rb_str_clear, 0);
    rb_define_method( rb_cString, "head", rb_str_head, -1);
    rb_define_method( rb_cString, "rest", rb_str_rest, -1);
    rb_define_method( rb_cString, "tail", rb_str_tail, -1);
    rb_define_method( rb_cString, "start_with?", rb_str_start_with_p, 1);
    rb_define_method( rb_cString, "end_with?", rb_str_end_with_p, 1);
    rb_define_alias(  rb_cString, "starts_with?", "start_with?");
    rb_define_alias(  rb_cString, "ends_with?", "end_with?");
    rb_define_alias(  rb_cString, "starts_with", "start_with?");
    rb_define_alias(  rb_cString, "ends_with", "end_with?");
#ifdef STRING_ORD
    rb_define_method( rb_cString, "ord", rb_str_ord, 0);
#endif
    rb_define_method( rb_cString, "axe", rb_str_axe, -1);

    rb_define_method( rb_cNumeric, "nil_if", rb_obj_nil_if, 1);
    rb_define_method( rb_cNumeric, "pos?", rb_num_pos_p, 0);
    rb_define_method( rb_cNumeric, "neg?", rb_num_neg_p, 0);
    rb_define_method( rb_cNumeric, "grammatical", rb_num_grammatical, 2);

    rb_define_method( rb_cArray, "notempty?", rb_ary_notempty_p, 0);
    rb_define_method( rb_cArray, "indexes", rb_ary_indexes, 0);
    rb_define_alias(  rb_cArray, "keys", "indexes");
    rb_define_method( rb_cArray, "pick", rb_ary_pick, 0);
    rb_define_method( rb_cArray, "rpick", rb_ary_rpick, 0);
#ifdef ARRAY_INDEX_WITH_BLOCK
    rb_define_method( rb_cArray, "index", rb_ary_index, -1);
    rb_define_method( rb_cArray, "rindex", rb_ary_rindex, -1);
#endif
    rb_define_method( rb_cArray, "eat_lines", rb_ary_eat_lines, 0);

    rb_define_method( rb_cHash, "notempty?", rb_hash_notempty_p, 0);

    rb_define_method( rb_cIO, "eat_lines", rb_io_eat_lines, 0);

    rb_define_method( rb_cFile, "size", rb_file_size, 0);
    rb_define_method( rb_cFile, "flockb", rb_file_flockb, -1);
    rb_define_singleton_method( rb_cFile, "umask", step_file_s_umask, -1);

    rb_define_singleton_method( rb_cDir, "current", step_dir_s_current, 0);

    rb_define_method( rb_cMatch, "begin", rb_match_begin, -1);
    rb_define_method( rb_cMatch, "end", rb_match_end, -1);

    rb_define_method( rb_cNilClass, "notempty?", rb_nil_notempty_p, 0);
    rb_define_method( rb_cNilClass, "nonzero?", rb_nil_notempty_p, 0);
    rb_define_method( rb_cNilClass, "nil_if", rb_nil_nil_if, 1);
    rb_define_method( rb_cNilClass, "each_line", rb_nil_notempty_p, 0);
    rb_define_method( rb_cNilClass, "eat_lines", rb_nil_notempty_p, 0);

    rb_define_alias( rb_singleton_class( rb_cStruct), "[]", "new");

    id_delete_at = rb_intern( "delete_at");
    id_cmp       = rb_intern( "<=>");
    id_eat_lines = rb_intern( "eat_lines");
    id_eqq       = 0;
}

