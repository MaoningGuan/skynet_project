.PHONY: all linux macosx

all: linux

linux: PLAT=linux

macosx: PLAT=macosx

linux macosx:
	mkdir -p ./output
	cd 3rd/skynet && $(MAKE) $(PLAT) && cp ./skynet ../../output/skynet

clean:
	rm -rf ./output
	cd 3rd/skynet && $(MAKE) clean

