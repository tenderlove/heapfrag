#include <ruby.h>

struct heap_info {
    int pages_seen;
    int fd;
};

int object_itr(void * start, void * finish, size_t step, void * data)
{
    VALUE v = (VALUE)start;
    struct heap_info *info = (struct heap_info *)data;
    size_t n;
    ID flags[5];

    dprintf(info->fd, "page %d: ", info->pages_seen);
    for(; v != (VALUE)finish; v += step) {
	switch (BUILTIN_TYPE(v)) {
	    default: {
			 if((n = rb_obj_gc_flags(v, flags, sizeof(flags))) > 0) {
			     ID ID_old = rb_intern("old");
			     int is_old = 0;
			     for(size_t i = 0; i < n; i++) {
				 if(ID_old == flags[i]) is_old = 1;
			     }
			     if (is_old) {
				 dprintf(info->fd, "2");
			     } else {
				 dprintf(info->fd, "1");
			     }
			 } else {
			     dprintf(info->fd, "1");
			 }
			 break;
		     }
	    case T_NONE:
		dprintf(info->fd, "0");
		break;
	}
    }
    dprintf(info->fd, "\n");
    info->pages_seen++;
    return 0;
}

static VALUE heapstar_start(VALUE mod, VALUE usec, VALUE fd)
{
    struct heap_info info;
    rb_iv_set(mod, "file", fd);

    info.pages_seen = 0;
    info.fd = NUM2INT(rb_funcall(fd, rb_intern("to_i"), 0));
    rb_objspace_each_objects(object_itr, &info);
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
