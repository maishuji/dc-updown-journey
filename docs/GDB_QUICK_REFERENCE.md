# Dreamcast GDB Quick Reference

## Quick Start Commands

```bash
# Build and debug in one command
make debug-dc DC_IP=192.168.0.14

# Or step by step:
./build-dreamcast-debug.sh
sh-elf-gdb build-dreamcast-debug/src/udjourney/updown-journey.elf
(gdb) target remote 192.168.0.14:2159
```

## Essential GDB Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| `continue` | `c` | Resume execution |
| `step` | `s` | Step into function |
| `next` | `n` | Step over function |
| `finish` | | Exit current function |
| `backtrace` | `bt` | Show call stack |
| `print var` | `p var` | Show variable value |
| `info locals` | | Show all local variables |
| `info registers` | | Show CPU registers |
| `quit` | `q` | Exit GDB |

## Breakpoints

```gdb
break main                    # Break at function
break Game::init             # Break at method
break main.cpp:42            # Break at line
break Player.cpp:100 if hp<=0 # Conditional breakpoint
info breakpoints             # List all breakpoints
delete 1                     # Delete breakpoint #1
```

## Inspection

```gdb
print player_x               # Print variable
print/x address             # Print in hex
print *this                 # Print object contents
display hp                  # Auto-display on each stop
watch hp                    # Break when hp changes
x/10x 0x8c010000           # Examine memory (10 words)
```

## Execution Control

```gdb
until 100                   # Run until line 100
return                      # Return from function early
jump 50                     # Skip to line 50
kill                        # Kill program
```

## Tips

- Use Tab for command completion
- Up/Down arrows for command history
- `help <command>` for detailed help
- `set logging on` to save session to file

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Can't connect | Check IP, ping Dreamcast, verify dcload-ip running |
| No symbols | Run `./build-dreamcast-debug.sh` |
| Source not found | `directory /workspaces/dc-updown-journey/src` |
| GDB not found | `source /opt/toolchains/dc/kos/environ.sh` |

See [DREAMCAST_DEBUGGING.md](DREAMCAST_DEBUGGING.md) for complete guide.
