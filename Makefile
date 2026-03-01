DC_IP ?= 192.168.0.14
# Use this if connecting through host machine's dc-tool-ip:
# DC_IP ?= host.docker.internal

# Build Dreamcast version with debug symbols
.PHONY: build-dc-debug
build-dc-debug:
	@echo "Building Dreamcast version with debug symbols..."
	./build-dreamcast-debug.sh

# Kill any existing dc-tool-ip processes
.PHONY: kill-dc-debug
kill-dc-debug:
	@echo "Killing any existing dc-tool-ip processes..."
	@pkill -9 dc-tool-ip 2>/dev/null || true
	@echo "Done"

# Deploy debug build to Dreamcast and start GDB server
.PHONY: deploy-dc-debug
deploy-dc-debug: kill-dc-debug build-dc-debug
	@echo "Deploying debug build to Dreamcast at $(DC_IP) in GDB mode..."
	@echo "This will wait for GDB to connect. Run 'make connect-gdb' in another terminal."
	dc-tool-ip -t $(DC_IP) -g -x build-dreamcast-debug/src/udjourney/updown-journey.elf

# Connect GDB client (run this in a separate terminal after deploy-dc-debug)
.PHONY: connect-gdb
connect-gdb:
	@echo "Connecting GDB to Dreamcast at $(DC_IP):2159..."
	@echo "Make sure deploy-dc-debug is running and showing 'waiting for gdb client connection...'"
	@echo ""
	@echo "GDB Commands:"
	@echo "  - 'continue' or 'c' to start execution"
	@echo "  - 'break main' to set a breakpoint at main"
	@echo "  - 'step' or 's' to step through code"
	@echo "  - 'next' or 'n' to step over functions"
	@echo "  - 'backtrace' or 'bt' to see call stack"
	@echo ""
	@sleep 1
	sh-elf-gdb build-dreamcast-debug/src/udjourney/updown-journey.elf \
		-ex "set architecture sh4" \
		-ex "set remotetimeout 30" \
		-ex "target remote $(DC_IP):2159"

# Complete debug workflow (deploy and connect in one command) - RECOMMENDED
.PHONY: debug-dc
debug-dc: kill-dc-debug build-dc-debug
	@echo "Starting automated debug session..."
	@echo "Deploying to Dreamcast and connecting GDB..."
	@(dc-tool-ip -t $(DC_IP) -g -x build-dreamcast-debug/src/udjourney/updown-journey.elf 2>&1 | tee /tmp/dc-tool.log &); \
	DC_PID=$$! ; \
	sleep 2; \
	sh-elf-gdb build-dreamcast-debug/src/udjourney/updown-journey.elf \
		-ex "set architecture sh4" \
		-ex "set remotetimeout 60" \
		-ex "target remote $(DC_IP):2159" || (echo "GDB connection failed, killing dc-tool..."; kill $$DC_PID 2>/dev/null)

# Run on Dreamcast (release build)
run-dc:
	dc-tool-ip -t $(DC_IP) -x build/src/udjourney/updown-journey.elf

run-dc2:
	dc-tool-ip -g -t $(DC_IP) -x build/src/udjourney/updown-journey.elf

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf build/
	rm -rf out/
	@echo "Clean complete"

.PHONY: lint
lint:
	cpplint --filter=-build/c++11,-runtime/references,-whitespace/indent_namespace,-build/header_guard,-build/c++17 --exclude=src/udjourney/include/udjourney/dreamcast_json_compat.h --recursive src/
	
.PHONY: lint-strict
lint-strict: clean
	@echo "🔍 Running strict cpplint (like CI)..."
	cpplint --filter=-build/c++11,-runtime/references,-build/header_guard,-build/c++17 \
    	--exclude=src/udjourney/include/udjourney/dreamcast_json_compat.h \
    	--recursive src/

tidy:
	@echo "Run clang-tidy on the source files"
	find src/udjourney/ -name '*.cpp' -o -name '*.hpp' | xargs clang-tidy -p build/Debug  -header-filter=.* -quiet
