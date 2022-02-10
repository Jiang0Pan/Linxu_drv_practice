KVERS = $(shell uname -r)

# kernel modules
obj-m += drv_globalmem_multidev.o

# specify flags for the module compilation
# for module debug information
#EXTRA_CFLAGS= -g -o0

build:kernel_module

CONFIG_MODULE_SIG=n

kernel_module:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
