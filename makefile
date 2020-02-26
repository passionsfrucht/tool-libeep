.PHONY: python clean

.ONESHELL:
python:

	mkdir build
	cmake -S . -B build
	make -C build
	mkdir eep
	cp build/python/v3/pyeep.so eep
	cp python/__init__.py eep


.ONESHELL:
clean:
	git clean -fx
	rm -rf build
	rm -rf python/eep
	rm -rf python/python