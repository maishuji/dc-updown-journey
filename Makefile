DC_IP ?= 192.168.0.85

run-dc:
	dc-tool-ip -t $(DC_IP) -x build/Debug/src/udjourney/upown-journey.elf

lint:
	cpplint --recursive src/

tidy:
	clang-tidy src/udjourney/src/Bonus.cpp -p build/Debug/ \
	--extra-arg=-D__DREAMCAST__	\
	--extra-arg=-target=sh-elf \
	--extra-arg=--sysroot=/opt/toolchains/dc/sh-elf/sh-elf \
	--extra-arg=-nostdinc++ \
	--extra-arg=-isystem/opt/toolchains/dc/kos/include/ \
	--extra-arg=-isystem/opt/toolchains/dc/kos/arch/dreamcast/include/ \
	--extra-arg=-isystem/opt/toolchains/dc/kos-ports/include/ \
	--extra-arg=-isystem/opt/toolchains/dc/kos-ports/arch/dreamcast/include/\
	--extra-arg=-isystem/opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/ \
	--extra-arg=-isystem/opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/sh-elf/ \
	--extra-arg=-isystem/opt/toolchains/dc/sh-elf/sh-elf/include/ \
	--header-filter='src/udjourney/src/*' \
	--system-headers=false

tidy2:
	clang-tidy src/udjourney/src/Bonus.cpp -p build/Debug/ 
	-checks=-*,clang-analyzer-*,-clang-analyzer-cplusplus* \
	--exclude-header-filter='kos\.h' \
	--exclude-header-filter='raylib\/*\.h' \
	--compile-commands=/build/Debug/compile_commands.json \
	--extra-arg=--sysroot=/opt/toolchains/dc/sh-elf/sh-elf \
	--extra-arg=-nostdinc++ 
	--extra-arg=-isystem/opt/toolchains/dc/kos/include \
	--extra-arg=-isystem/opt/toolchains/dc/kos/arch/dreamcast/include \
	--extra-arg=-isystem/opt/toolchains/dc/kos-ports/include \
	--extra-arg=-isystem/opt/toolchains/dc/kos-ports/arch/dreamcast/include \
	--extra-arg=-isystem/opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1 \
	--extra-arg=-isystem/opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/sh-elf \
	--extra-arg=-isystem/opt/toolchains/dc/sh-elf/sh-elf/include \
	--header-filter='src/udjourney/src/*' 


tidy3:
	clang-tidy src/udjourney/src/Bonus.cpp -p build/Debug/ \
  --system-headers=false \
  --extra-arg=-isystem=/opt/toolchains/dc/sh-elf/sh-elf/include/ \
  --extra-arg=-isystem=/opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1 \
  --extra-arg=-isystem=/opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/sh-elf \
  --extra-arg=-isystem=/opt/toolchains/dc/sh-elf/sh-elf/include \
  --extra-arg=-isystem=/opt/toolchains/dc/kos-ports/include \
  --extra-arg=-isystem=/opt/toolchains/dc/kos-ports/arch/dreamcast/include \
  --header-filter='src/udjourney/src/*'

