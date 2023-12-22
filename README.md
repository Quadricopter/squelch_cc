squelch_cc
====

`squelch_cc` is a command line tool which provide an alternative to `csdr squelch_and_smeter_cc`.  
It doesn't needs two mandatory `fifo` to start.

Typical usage
-------------

`squelch_cc` only works with `float complex`. It should be placed right after `csdr` decimation:
```
... | \
csdr convert_u8_f | \
csdr shift_addition_cc <addition> | \
csdr fir_decimate_cc <decimate> | \
squelch_cc | \
csdr fmdemod_quadri_cf | \
...
```

Options
-------

* `--dump` Dump signal level on `stderr`  
* `--level <level>` Set squelch threshold  (Default: 0.01)
* `--blocking` Stop filling `stdout` with `'\0'` if the gate is closed
* `--count <count>` Samples window size for noise level computation (Default: 1024 samples)
