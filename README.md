# Linux Systems Programming: A Journey Through Pain

> "It works on my machine." — every process before it gets `SIGKILL`ed

This repository documents one student's heroic descent into the abyss of Linux systems programming. Forks were forked. Signals were caught. Zombies were reaped. Nobody slept (especially not the processes — they were too busy `while(1)`-ing).

---

## What's In Here

### `linux/`

The main arena. Where bugs go to live forever (or until the OS kills them).

#### Shells (Yes, plural. We built multiple shells.)

| File | Description |
|------|-------------|
| `nutshell.c` | A shell, in a nutshell. Accepts commands. Sometimes runs them. Baby's first REPL. |
| `seashell.c` | nutshell grew up, got a job table, learned about background processes, and started handling `SIGCHLD` like an adult. |
| `bjsigchild.c` | seashell went to therapy and fixed its zombie problem. |
| `seashell_vs.c` | The final form. Whether it's "versus" bugs or "very stable" is left as an exercise to the grader. |

#### "Other Programs"

| File | Description |
|------|-------------|
| `bj.c` | `while(1);` — A program that does absolutely nothing, as fast as possible, forever. 100% CPU. No regrets. The philosophical statement of this entire repo. |
| `boss.c` | Prints "Sairam", then forks. That's it. That's the whole program. The boss reproduces and leaves. |
| `ptp.c` | Spawns 3 children, waits for them to finish napping (literally `sleep(rand()%4+1)`), and collects their exit codes like Pokémon. |

---

### `linux/signals/`

Learning to talk to processes using the Unix equivalent of throwing rocks at them.

| File | What signal crime was committed |
|------|-------------------------------|
| `catchctrlc.c` | Ctrl+C? Caught it. Nice try. |
| `ignctrlc.c` | Ctrl+C? Don't care. Even nicer try. |
| `countSIGINT.c` | Counts every time you press Ctrl+C. Judges you silently. |
| `alarm.c` | Sets a timer. Gets surprised when it goes off. `SIGALRM` jumpscare. |
| `ParentSIGUSR1ToChild.c` | Parent sends `SIGUSR1` to child. The most passive-aggressive IPC imaginable. |

---

### `linux/advsignals/`

Advanced Signal Manipulation — or: "What if signals, but harder?"

| File | Description |
|------|-------------|
| `sigaction.c` | `signal()` was too easy. Time to fill out a `struct sigaction` like a real adult. |
| `sigmask.c` | Blocking signals so you don't have to deal with them right now. Emotional unavailability, but for processes. |
| `sigpending.c` | Asking the kernel "hey, what signals are you holding onto for me?" Cosmic anxiety check. |
| `safehandler.c` | Uses `write()` instead of `printf()` in signal handlers because we learned what async-signal-safety means and now we can't unlearn it. |

---

### `process-subsystem-practice/`

The fork() extended universe.

| File | Description |
|------|-------------|
| `forktree.c` | `fork(); fork(); fork();` — 8 processes print "Hello". Nobody asked them to. They did it anyway. |
| `forkzombie.c` | Creates a zombie process on purpose. Genuinely unsettling. |
| `memisotry.c` | Proves that child processes get their own memory. Ruins the shared-memory dream. |
| `forkingtry.c` | The very first fork. The one that started it all. We don't talk about what happened. |

---

## How to Build

```bash
gcc -o program program.c
# or if you like options:
gcc -Wall -Wextra -o program program.c
# or if you're feeling dangerous:
gcc -o bj bj.c && ./bj   # (your laptop will never forgive you)
```

---

## Lessons Learned

1. `fork()` returns **twice**. This is fine.
2. Always `wait()` for your children. Zombie processes are your fault.
3. `printf()` in a signal handler is undefined behavior. You've probably done it anyway.
4. A shell is just an infinite loop with delusions of grandeur.
5. `while(1);` is technically a valid program.
6. The kernel doesn't care about your feelings, only your exit codes.

---

## System Requirements

- Linux (obviously)
- A terminal
- The will to live
- `gcc`

---

*Written in C, because some of us enjoy suffering close to the hardware.*
