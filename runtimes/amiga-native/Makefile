BUILD_DIR ?= build-host
CC ?= cc

.PHONY: configure test clean compiler-check

compiler-check:
	@tmp=$$(mktemp /tmp/openvn-cc-check.XXXXXX); \
	if printf 'int main(void){return 0;}\n' | $(CC) -x c - -o "$$tmp" >/dev/null 2>&1; then \
		rm -f "$$tmp"; \
	else \
		rm -f "$$tmp"; \
		echo "No working native C compiler found. Host C tests skipped."; \
	fi

configure:
	@tmp=$$(mktemp /tmp/openvn-cc-check.XXXXXX); \
	if printf 'int main(void){return 0;}\n' | $(CC) -x c - -o "$$tmp" >/dev/null 2>&1; then \
		rm -f "$$tmp"; \
		cmake -S . -B $(BUILD_DIR) -DCMAKE_C_COMPILER=$(CC); \
	else \
		rm -f "$$tmp"; \
		echo "No working native C compiler found. Host C tests skipped."; \
	fi

test:
	@tmp=$$(mktemp /tmp/openvn-cc-check.XXXXXX); \
	if printf 'int main(void){return 0;}\n' | $(CC) -x c - -o "$$tmp" >/dev/null 2>&1; then \
		rm -f "$$tmp"; \
		cmake -S . -B $(BUILD_DIR) -DCMAKE_C_COMPILER=$(CC) && \
		cmake --build $(BUILD_DIR) && \
		ctest --test-dir $(BUILD_DIR) --output-on-failure; \
	else \
		rm -f "$$tmp"; \
		echo "No working native C compiler found. Host C tests skipped."; \
	fi

clean:
	rm -rf $(BUILD_DIR)
