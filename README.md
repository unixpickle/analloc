# analloc

This is a completely stand-alone memory allocator which uses [Buddy memory allocation](http://en.wikipedia.org/wiki/Buddy_memory_allocation) under the hood. The point of *analloc* is to provide a flexible, lightweight allocator to operating system kernels.

# Usage

Initializing *analloc* is a simple matter. The usage is roughly as follows:

 * Reserve a chunk of physical memory for use with *analloc*. This chunk may begin with some pre-allocated memory.
 * Allocate some memory for an `analloc_t` structure. This memory may be part of the pre-allocated memory in the physical memory chunk you plan to use for *analloc*.
 * Call `analloc_with_chunk()`. The first argument is a pointer to the analloc_t structure, the next is a pointer to the absolute beginning of the buffer, the next is the number of bytes *used* by your pre-allocated data at the beginning of the buffer. If you use another location to store the `analloc_t`, this may very well be 0. The next argument is the *page* size. This specifies the minimum amount of bytes the user should be able to allocate.
 * Calculate the number of bytes actually used by *analloc*.  This is equal to `context->page << context->depth` where `context` is an `analloc_t`.

To allocate some data, use `analloc_alloc()`, and to free data use `analloc_free()`. Data resizing functions are still a work in progress.

# License

This software is under the GNU GPL v1.
