ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: build clean

build:  test_program.o
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	cc -g -o test_program test_program.o

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f modules.order Module.symvers
else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m :=    crash1.o k8101.o

endif
