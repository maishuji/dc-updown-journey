DC_IP ?= 192.168.0.84

run-dc-remote:
	dc-tool-ip -t $(DC_IP) -x build/Debug/src/udjourney/upown-journey.elf