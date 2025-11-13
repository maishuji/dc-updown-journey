DC_IP ?= 192.168.0.85

run-dc:
	dc-tool-ip -t $(DC_IP) -x build/Debug/src/udjourney/updown-journey.elf

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf build/
	rm -rf out/
	@echo "Clean complete"

.PHONY: lint
lint:
	cpplint --filter=-build/c++11,-runtime/references,-build/header_guard,-build/c++17 --exclude=src/udjourney/include/udjourney/dreamcast_json_compat.h --recursive src/
	
.PHONY: lint-strict
lint-strict: clean
	@echo "🔍 Running strict cpplint (like CI)..."
	cpplint --filter=-build/c++11,-runtime/references,-build/header_guard,-build/c++17 \
    	--exclude=src/udjourney/include/udjourney/dreamcast_json_compat.h \
    	--recursive src/

tidy:
	@echo "Run clang-tidy on the source files"
	find src/udjourney/ -name '*.cpp' -o -name '*.hpp' | xargs clang-tidy -p build/Debug  -header-filter=.* -quiet