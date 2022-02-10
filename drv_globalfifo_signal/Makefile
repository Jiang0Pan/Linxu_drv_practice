KVERS = $(shell uname -r)

# kernel modules
obj-m += drv_globalfifo_signal.o

# specify flags for the module compilation
# for module debug information
#EXTRA_CFLAGS= -g -o0

build:kernel_module

CONFIG_MODULE_SIG=n

kernel_module:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
	gcc app_globalfifo_signal.c -o app_globalfifo_signal
	gcc app_globalfifo_signal_1.c -o app_globalfifo_signal_1
	gcc app_globalfifo_signal_2.c -o app_globalfifo_signal_2	

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm app_globalfifo_signal app_globalfifo_signal_1 app_globalfifo_signal_2

