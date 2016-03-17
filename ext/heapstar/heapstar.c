#include <ruby.h>

static VALUE heapstar_start(VALUE mod, VALUE usec, VALUE fd)
{
    return Qtrue;
}

static VALUE heapstar_stop(VALUE mod)
{
    return Qfalse;
}

void Init_heapstar()
{
    VALUE mHeapstar = rb_define_module("Heapstar");
    rb_define_singleton_method(mHeapstar, "start", heapstar_start, 2);
    rb_define_singleton_method(mHeapstar, "stop", heapstar_stop, 0);
}
/* vim: set noet sws=4 sw=4: */
