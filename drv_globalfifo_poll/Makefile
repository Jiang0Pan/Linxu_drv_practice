KVERS = $(shell uname -r)

# kernel modules
obj-m += drv_globalfifo_poll.o

# specify flags for the module compilation
# for module debug information
#EXTRA_CFLAGS= -g -o0

build:kernel_module

CONFIG_MODULE_SIG=n

kernel_module:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
	gcc app_globalfifo_select.c -o app_globalfifo_select
	gcc app_globalfifo_epoll.c -o app_globalfifo_epoll

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm app_globalfifo_select
	rm app_globalfifo_epoll
