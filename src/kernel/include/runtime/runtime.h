#pragma once

#include <stdint.h>

void runtime_init();
void runtime_execute(const uint8_t* wasm_binary, uint32_t size);