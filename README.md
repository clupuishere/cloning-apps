# Applications for showcasing Nephele
This repo wraps a range of applications that can be used for testing and
evaluating the Nephele cloning solution for unikernel-based VMs.  The
applications run on Mini-OS and Unikraft unikernels, but also as Linux
processes. Based on the build configuration, a binary image can run one or more
of the following:
- **counter**
- **simple UDP server** used in the instantiation and memory consumption
  microbenchmarks of Nephele
- **simple TCP server**
- **measure-fork** was used for measuring the `fork()` duration for different
  memory sizes in the Nephele paper
- **memory-overhead** measures the overhead added by Nephele for memory
  accesses after cloning a parent guest
- **files** tests the cloning support for 9pfs  
- **children** is a fork server that waits for incoming requests to clone

# How to build
## Mini-OS
Our modified [Mini-OS](https://github.com/nephele-vm/mini-os) can be built
together with the cloning applications by setting the `CLONING_APPS_DIR` and
`APP` environment variables. For a thorough example, please check the
[build.sh](https://github.com/nephele-vm/experiments/blob/main/build/build.sh)
script in our [experiments repo](https://github.com/nephele-vm/experiments).

```
make <other parameters ..>  CLONING_APPS_DIR=$CLONING_APPS_DIR APP=server-udp

```

## Unikraft
The current tree is created also as a regular Unikraft application tree,
providing the files needed for building it as Unikraft application:
`Config.uk`, `Makefile.uk` and `Makefile.unikraft`.  Thus the following command
will built the binary image for Unikraft guests:

```
make -f Makefile.unikraft
```

You should use the [Unikraft
kernel](https://github.com/nephele-unikraft/unikraft) modified for Nephele and
the libraries (e.g., pthread-embedded, newlib, lwip, mimalloc) provided by our
[Unikraft-related
repositories](https://github.com/orgs/nephele-unikraft/repositories).  The
[build.sh](https://github.com/nephele-vm/experiments/blob/main/build/build.sh)
script in our [experiments repo](https://github.com/nephele-vm/experiments)
builds both Mini-OS and Unikraft images by running:

```
./build.sh guests
```

## Linux
To build for Linux, run:

```
make -f Makefile.linux
```
