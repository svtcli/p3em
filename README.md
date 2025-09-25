# p3em

**P**ermitted, **P**ortable, **P**arsable **E**nergy **M**eter for CPUs and GPUs. 
- Native `Intel`, `NVIDIA` and `AMD` GPU via `xxx-smi`. Native CPU support via `perf`
- Millisecond latency
- Fortran, C, C++, Python examples provided
- Encapsulated in *unix* shell for complete freedom of interface and portability.
- Add any custom energy meter simply by executing in shell.

## Usage 
The flexibility and low latency of `p3em` makes it perfect to profile energy consumption of _regions of interest_ in your of code.
If you are at this point, you likely have already your regions of interest marked by your _internal code timer_. If not, add one right now (examples go here). 
We won't give you yet another API. 
Use the one you already have, by instrumenting your timer `now()` funciton 
to return energy instead of time. Both quantities are monotonic, additive in intervals, and 
your timer will zero either variable. 
A likely : 
- add p3em as submodule
- follow the provided examples (C/Fortran/C++/pyton) for insrumenting your timer 
- run your code without any change
- process the performance measures with your usual pipeline.
For further examples look here: (link Tmr project after submoduled.)
Not what you are looking for right now? Read below to understand and customize `p3em`'s behaviour.


## Meaning
- `p3em.sh` parses the sample energy meters to yield a running total of consumed energy.
- The measure is intended at node level; node fractions are no fun.
- The provided energy meters aim only at the _device cores_, as a proxy of _device size_
 for providing properly normalized performance values, directly comparable among devices. See paper (TBA)
- Interested in other metrics with different meanings, e.g: global "at the plug"
measures? Include memory/disk? Adding CPU and GPU together? See below!

## Add your own energy meter
Simply add your own executable or script, written in your favourite language, printing any running total 
of consumed energy, and lowest possible latency, following the example. 
Just run `p3em.sh` in your shell for visualising it.
examples: (link Ivan's nvml timer).