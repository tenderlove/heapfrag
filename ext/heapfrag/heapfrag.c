#include <ruby.h>
#include <signal.h>
#include <sys/time.h>

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

    dprintf(info->fd, "{\"page\":%d,\"heap\":[", info->pages_seen);
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
	if ((v + step) != (VALUE)finish) {
	    dprintf(info->fd, ",");
	}
    }
    dprintf(info->fd, "]},");
    info->pages_seen++;
    return 0;
}

static int write_fd;

static void realtime_handler(int signum, siginfo_t *info, void *context)
{
    struct heap_info ruby_info;

    ruby_info.pages_seen = 0;
    ruby_info.fd = write_fd;

    dprintf(ruby_info.fd, "[");
    rb_objspace_each_objects(object_itr, &ruby_info);
    dprintf(ruby_info.fd, "{}]\n");
}

static VALUE heapfrag_start(VALUE mod, VALUE usec, VALUE fd)
{
    struct sigaction act;
    struct itimerval itv;

    rb_iv_set(mod, "file", fd); /* prevent GC */

    write_fd = NUM2INT(rb_funcall(fd, rb_intern("to_i"), 0));

    act.sa_sigaction = realtime_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGALRM, &act, NULL);

    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = NUM2LONG(usec);
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = NUM2LONG(usec);
    setitimer(ITIMER_REAL, &itv, NULL);

    return Qtrue;
}

static VALUE heapfrag_stop(VALUE mod)
{
    return Qfalse;
}

void Init_heapfrag()
{
    VALUE mHeapfrag = rb_define_module("Heapfrag");
    rb_define_singleton_method(mHeapfrag, "start", heapfrag_start, 2);
    rb_define_singleton_method(mHeapfrag, "stop", heapfrag_stop, 0);
}
/* vim: set noet sws=4 sw=4: */
