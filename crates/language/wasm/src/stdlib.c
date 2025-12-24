// C stdlib implementation for wasm32-unknown-unknown builds.
// This allocator delegates to Rust's global allocator (dlmalloc) via extern
// functions, ensuring both Rust and C code use the same allocator and avoiding
// memory corruption from two allocators sharing the same linear memory heap.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Rust allocator functions exported from the tree-sitter Rust crate.
extern void *rust_malloc(size_t size);
extern void rust_free(void *ptr, size_t size);
extern void *rust_realloc(void *ptr, size_t old_size, size_t new_size);

// Each allocation is preceded by its size so we can call rust_free/rust_realloc
// with the correct size. We use a fixed 8-byte header for alignment.
typedef struct {
  size_t size;
  size_t _pad;
} Block;

#define BLOCK_SIZE sizeof(Block)

void *malloc(size_t size) {
  if (size == 0) return NULL;

  size_t total = BLOCK_SIZE + size;
  void *raw = rust_malloc(total);
  if (raw == NULL) return NULL;

  Block *block = (Block *)raw;
  block->size = total;

  return (void *)((uintptr_t)raw + BLOCK_SIZE);
}

void free(void *ptr) {
  if (ptr == NULL) return;

  Block *block = (Block *)((uintptr_t)ptr - BLOCK_SIZE);
  rust_free((void *)block, block->size);
}

void *calloc(size_t count, size_t size) {
  size_t total = count * size;
  void *ptr = malloc(total);
  if (ptr) memset(ptr, 0, total);
  return ptr;
}

void *realloc(void *ptr, size_t new_size) {
  if (ptr == NULL) {
    return malloc(new_size);
  }
  if (new_size == 0) {
    free(ptr);
    return NULL;
  }

  Block *block = (Block *)((uintptr_t)ptr - BLOCK_SIZE);
  size_t old_total = block->size;
  size_t new_total = BLOCK_SIZE + new_size;

  void *new_raw = rust_realloc((void *)block, old_total, new_total);
  if (new_raw == NULL) return NULL;

  Block *new_block = (Block *)new_raw;
  new_block->size = new_total;

  return (void *)((uintptr_t)new_raw + BLOCK_SIZE);
}

// Keep reset_heap for compatibility with tree-sitter's wasm runtime
// (used when embedding grammars as wasm, not in our use case)
void reset_heap(void *new_heap_start) {
  (void)new_heap_start;
  // No-op: we don't manage our own heap anymore
}

__attribute__((noreturn)) void abort(void) {
  __builtin_trap();
}
