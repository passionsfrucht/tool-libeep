.PHONY: python clean

.ONESHELL:
python:

	mkdir build
	cmake -S . -B build
	make -C build
	mkdir libeep
	cp build/python/v3/pyeep.so libeep
	cp python/__init__.py libeep


.ONESHELL:
clean:
	git clean -fx
	rm -rf build
	rm -rf python/libeep
	rm -rf python/python