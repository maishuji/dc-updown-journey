DC_IP ?= 192.168.0.85

run-dc:
	dc-tool-ip -t $(DC_IP) -x build/Debug/src/udjourney/upown-journey.elf

lint:
	cpplint --recursive src/
