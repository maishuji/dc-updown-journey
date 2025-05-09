DC_IP ?= 192.168.0.85

run-dc:
	dc-tool-ip -t $(DC_IP) -x build/Debug/src/udjourney/upown-journey.elf

lint:
	cpplint --filter=-build/c++11,-runtime/references,-build/header_guard --recursive src/
	
tidy:
	@echo "Run clang-tidy on the source files"
	find src/udjourney/src/ -name '*.cpp' -o -name '*.hpp' | xargs clang-tidy -p build/Debug  -header-filter=.* -quiet