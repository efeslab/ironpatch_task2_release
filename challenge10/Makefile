all: patched vuln

.PHONY: patched vuln

patched:
	make -C patched

vuln:
	make -C vuln

clean:
	make clean -C patched
	make clean -C vuln
