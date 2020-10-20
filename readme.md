# File Auditing

## Build Kernel Module

### Init

```bash
make
insmod AuditModule.ko
```

### Log

```bash
dmesg
dmesg -c // clear the log
```

### Exit

```bash
rmmod AuditModule.ko
```

### Illustration

the following sentence reloads the system call

```C
sys_call_table[__NR_openat] = (demo_sys_call_ptr_t) hacked_openat;
```

