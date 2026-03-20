#include "runtime/runtime.h"

#include "wasm3.h"
#include "m3_env.h"
#include "serial.h"
#include "graphics.h"

#include "interrupt.h"
#include "ioapic.h"
#include "drivers/lapic.h"

#include "drivers/io.h"

#include "time.h"
#include "math.h"

IM3Environment env;
IM3Runtime runtime;
IM3Module module;

#define KEY_BUFFER_SIZE 64
static uint8_t key_buffer[KEY_BUFFER_SIZE];
static uint32_t key_head = 0;
static uint32_t key_tail = 0;

void push_key(uint8_t scancode) {
    uint32_t next = (key_head + 1) % KEY_BUFFER_SIZE;
    
    if (next == key_tail) { 
        key_tail = (key_tail + 1) % KEY_BUFFER_SIZE;
    }
    
    key_buffer[key_head] = scancode;
    key_head = next;
}

uint8_t pop_key() {
    if (key_head == key_tail) return 0; // 데이터 없음
    uint8_t scancode = key_buffer[key_tail];
    key_tail = (key_tail + 1) % KEY_BUFFER_SIZE;
    return scancode;
}

void keyboard_handler(interrupt_frame_t* regs) {
    uint8_t scancode = inb(0x60);
    if (!(scancode & 0x80)) {
        push_key(scancode);
    }
    lapic_eoi();
}

m3ApiRawFunction(host_kprintf) {
    m3ApiGetArg(uint32_t, wasm_ptr);

    const char* msg = m3_GetMemory(runtime, NULL, 0) + wasm_ptr;

    if (msg) {
        kprintf("%s", msg);
    }

    m3ApiSuccess();
}
m3ApiRawFunction(host_draw_pixel) {
    m3ApiGetArg(uint32_t, x);
    m3ApiGetArg(uint32_t, y);
    m3ApiGetArg(uint32_t, color);
    
    draw_pixel(x, y, color);

    m3ApiSuccess();
}
m3ApiRawFunction(m3_get_key) {
    m3ApiReturnType(uint32_t);
    m3ApiReturn(pop_key());
}
m3ApiRawFunction(m3_get_ms) {
    m3ApiReturnType(uint64_t);

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    uint64_t ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    
    m3ApiReturn(ms);
}
m3ApiRawFunction(m3_cosf) {
    m3ApiReturnType(float);
    m3ApiGetArg(float, x);
    m3ApiReturn(cosf(x));
}

m3ApiRawFunction(m3_sinf) {
    m3ApiReturnType(float);
    m3ApiGetArg(float, x);
    m3ApiReturn(sinf(x));
}

m3ApiRawFunction(m3_fabsf) {
    m3ApiReturnType(float);
    m3ApiGetArg(float, x);
    m3ApiReturn(fabsf(x));
}

void runtime_init() {
    env = m3_NewEnvironment();
    runtime = m3_NewRuntime(env, 512 * 1024, NULL);
    ioapic_set_irq(1, 0, 34);
    register_interrupt_handler(34, keyboard_handler);
}

void runtime_execute(const uint8_t* wasm_binary, uint32_t size) {
    IM3Module module;
    M3Result result = m3_ParseModule(env, &module, wasm_binary, size);
    if (result) {
        kprintf("[Runtime] Error parsing module: %s\n", result);
        return;
    }

    result = m3_LoadModule(runtime, module);
    if (result) {
        kprintf("[Runtime] Error loading module: %s\n", result);
        return;
    }

    m3_LinkRawFunction(module, "env", "serial_printf", "v(i)", host_kprintf);
    m3_LinkRawFunction(module, "env", "draw_pixel", "v(iii)", host_draw_pixel);
    m3_LinkRawFunction(module, "env", "get_key", "i()", &m3_get_key);
    m3_LinkRawFunction(module, "env", "get_ms", "I()", m3_get_ms);

    m3_LinkRawFunction(module, "env", "cosf", "f(f)", m3_cosf);
    m3_LinkRawFunction(module, "env", "sinf", "f(f)", m3_sinf);
    m3_LinkRawFunction(module, "env", "fabsf", "f(f)", m3_fabsf);

    IM3Function f;
    result = m3_FindFunction(&f, runtime, "main");
    if (result) {
        kprintf("[Runtime] Function not found: %s\n", result);
        return;
    }

    result = m3_CallV(f);
    if (result) {
        kprintf("[Runtime] Execution error: %s\n", result);
    }
}