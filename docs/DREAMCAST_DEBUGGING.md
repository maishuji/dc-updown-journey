# Debugging the Dreamcast Build with GDB

This guide explains how to debug the UDJourney game on Dreamcast hardware or emulator using GDB.

## Prerequisites

1. **KallistiOS toolchain** with `sh-elf-gdb` installed
2. **Dreamcast with dcload-ip** (or dcload-serial) running
3. **Network connection** to your Dreamcast (via BBA or emulator)
4. **dc-tool-ip** for deploying and communicating with the Dreamcast

## Quick Start

### Method 1: Using Make (Recommended)

The fastest way to build and debug:

```bash
# Build with debug symbols and start interactive GDB session
make debug-dc DC_IP=192.168.0.14
```

Replace `192.168.0.14` with your Dreamcast's IP address.

### Method 2: Using VS Code

1. Open the project in VS Code
2. Press `F5` or go to Run → Start Debugging
3. Select "Debug Dreamcast (Remote GDB)" from the dropdown
4. Enter your Dreamcast's IP address when prompted
5. The build will start automatically, then GDB will connect

### Method 3: Manual Steps

```bash
# 1. Source KallistiOS environment
source /opt/toolchains/dc/kos/environ.sh

# 2. Build with debug symbols
./build-dreamcast-debug.sh

# 3. Deploy to Dreamcast (optional - can also use dcload's menu)
dc-tool-ip -t 192.168.0.14 -x build-dreamcast-debug/src/udjourney/updown-journey.elf

# 4. Start GDB
sh-elf-gdb build-dreamcast-debug/src/udjourney/updown-journey.elf

# 5. In GDB, connect to Dreamcast
(gdb) target remote 192.168.0.14:2159
```

## Build Configuration

The debug build uses the following flags:
- `-g`: Include debugging information
- `-ggdb3`: Maximum debug info for GDB
- `-O0`: No optimization (makes debugging easier)

The debug build output is placed in `build-dreamcast-debug/` to keep it separate from release builds.

## Setting Up dcload-ip on Dreamcast

Before debugging, your Dreamcast needs to be running dcload-ip:

1. **Burn dcload-ip to a CD** or use an SD card loader
2. **Boot dcload-ip** on your Dreamcast
3. **Note the IP address** displayed on screen
4. **Ensure network connectivity** between your PC and Dreamcast

See [dreamcast-connectivity.md](dreamcast-connectivity.md) for detailed network setup instructions.

## Common GDB Commands

### Basic Navigation
```gdb
# Continue execution
continue
# or
c

# Step into function
step
# or
s

# Step over function (don't enter)
next
# or
n

# Finish current function
finish
```

### Breakpoints
```gdb
# Set breakpoint at function
break main
break Game::init
break Player::update

# Set breakpoint at file:line
break main.cpp:42

# List breakpoints
info breakpoints

# Delete breakpoint
delete 1

# Disable/enable breakpoint
disable 1
enable 1
```

### Inspection
```gdb
# Print variable value
print player_x
print this->m_health

# Print with format (hex, binary, etc.)
print/x address
print/t bitmask

# Display variable on every stop
display player_x

# Show call stack
backtrace
# or
bt

# Show local variables
info locals

# Show function arguments
info args
```

### Memory and Registers
```gdb
# Examine memory (x/FMT ADDRESS)
x/10x 0x8c010000    # 10 words in hex
x/s string_pointer   # as string

# Show registers
info registers

# Show specific register
print $r0
```

### Execution Control
```gdb
# Run until location
until main.cpp:100

# Jump to location (skip code)
jump main.cpp:50

# Return from function early
return

# Kill program
kill
```

## Debugging Tips

### 1. Source Code Mapping

GDB should automatically find source files. If not:
```gdb
# Set source directory
directory /workspaces/dc-updown-journey/src
```

### 2. Optimized Code

The debug build uses `-O0` for no optimization, making debugging much easier. If you need to debug optimized code:
- Use `info line` to see actual code locations
- Some variables may be optimized away
- Use `disassemble` to see assembly

### 3. Conditional Breakpoints

```gdb
# Break only when condition is true
break Monster.cpp:42 if health <= 0

# Break after N hits
ignore 1 100   # Skip breakpoint 1 for 100 hits
```

### 4. Watchpoints

```gdb
# Break when variable changes
watch player_health

# Break when memory location is written
watch *0x8c100000

# Break when variable is read
rwatch variable
```

### 5. Logging Output

```gdb
# Enable logging to file
set logging on
set logging file debug.log

# Log only output (not commands)
set logging redirect on
```

## Troubleshooting

### Cannot Connect to Dreamcast

```
(gdb) target remote 192.168.0.14:2159
Connection timed out.
```

**Solutions:**
1. Verify Dreamcast IP: Check the IP shown on dcload-ip screen
2. Test network: `ping 192.168.0.14`
3. Check firewall: Ensure port 2159 is open
4. Restart dcload-ip on Dreamcast

### No Debug Symbols

```
(gdb) break main
Function "main" not defined.
```

**Solution:** Rebuild with the debug script:
```bash
./build-dreamcast-debug.sh
```

Verify symbols are present:
```bash
sh-elf-nm build-dreamcast-debug/src/udjourney/updown-journey.elf | grep main
```

### GDB Not Found

```
bash: sh-elf-gdb: command not found
```

**Solution:** Source KallistiOS environment:
```bash
source /opt/toolchains/dc/kos/environ.sh
```

Add to your `~/.bashrc` to make it permanent:
```bash
echo 'source /opt/toolchains/dc/kos/environ.sh' >> ~/.bashrc
```

### Symbols Not Loading

If GDB can't find source files:
```gdb
# Show search path
show directories

# Add source directory
directory /workspaces/dc-updown-journey/src/udjourney/src
directory /workspaces/dc-updown-journey/src/udj-core/src

# Or use absolute path in breakpoints
break /workspaces/dc-updown-journey/src/udjourney/src/main.cpp:42
```

## Advanced Debugging

### Remote Debugging with Emulators

Some Dreamcast emulators (like Flycast/Redream) support GDB remote debugging:

1. Start emulator with GDB server enabled (usually port 23946)
2. Connect with: `target remote localhost:23946`

### Core Dumps

If the game crashes, you can examine the state:

```gdb
# In dcload-ip, the crash info may be displayed
# Use registers and backtrace to understand the crash

# Check PC (program counter)
info registers pc

# Disassemble around crash location
disassemble $pc-20,$pc+20
```

### Multi-threaded Debugging

KallistiOS has limited threading support, but you can:

```gdb
# List threads
info threads

# Switch to thread
thread 2

# Apply command to all threads
thread apply all backtrace
```

## GDB Configuration File

Create `~/.gdbinit` for automatic setup:

```gdb
# Pretty printing
set print pretty on
set print array on
set print array-indexes on

# History
set history save on
set history filename ~/.gdb_history
set history size 10000

# SH4 architecture
set architecture sh4

# Auto-load safe path
add-auto-load-safe-path /workspaces/dc-updown-journey
```

## Makefile Targets Reference

```bash
# Build debug version
make build-dc-debug

# Build and deploy debug version
make deploy-dc-debug DC_IP=192.168.0.14

# Build, deploy, and start GDB session
make debug-dc DC_IP=192.168.0.14
```

## VS Code Tasks Reference

Available tasks (Ctrl+Shift+P → "Tasks: Run Task"):
- **Build Dreamcast (Debug)** - Build with debug symbols
- **Deploy to Dreamcast (Debug)** - Build and deploy debug version
- **Start GDB Debug Session** - Interactive GDB in terminal

## Additional Resources

- [GDB Manual](https://sourceware.org/gdb/documentation/)
- [KallistiOS Documentation](https://kos-docs.dreamcast.wiki/)
- [Dreamcast Connectivity Guide](dreamcast-connectivity.md)
- [SH4 Architecture Reference](http://www.shared-ptr.com/sh_insns.html)

## Example Debugging Session

```bash
# Start debugging
make debug-dc DC_IP=192.168.0.14

# In GDB:
(gdb) break main
Breakpoint 1 at 0x8c010234: file main.cpp, line 15.

(gdb) continue
Continuing.

Breakpoint 1, main () at main.cpp:15
15      int main(int argc, char** argv) {

(gdb) next
16          Game game;

(gdb) step
Game::Game() at Game.cpp:12
12          : m_running(true) {

(gdb) print this
$1 = (Game *) 0x8c102000

(gdb) finish
Run till exit from #0  Game::Game() at Game.cpp:12

(gdb) watch game.m_running
Hardware watchpoint 2: game.m_running

(gdb) continue
Continuing.
# ... game runs until m_running changes ...
```

Happy debugging! 🐛🔍



## Hello 

sh-elf-gdb <path to elf>

in gdb : 
target remote 127.0.0.1:<port>
