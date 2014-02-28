# analloc

This is a completely stand-alone memory allocator which uses [Buddy memory allocation](http://en.wikipedia.org/wiki/Buddy_memory_allocation) under the hood. The point of *analloc* is to provide a flexible, lightweight allocator to operating system kernels.

# Usage

Initializing *analloc* is a simple matter. The usage is roughly as follows:

 * Reserve a chunk of physical memory for use with *analloc*. This chunk may begin with some pre-allocated memory.
 * Allocate some memory for an `analloc_t` structure. This memory may be part of the pre-allocated memory in the physical memory chunk you plan to use for *analloc*.
 * Call `analloc_with_chunk()`. The first argument is a pointer to the analloc_t structure, the next is a pointer to the absolute beginning of the buffer, the next is the number of bytes *used* by your pre-allocated data at the beginning of the buffer. If you use another location to store the `analloc_t`, this may very well be 0. The next argument is the *page* size. This specifies the minimum amount of bytes the user should be able to allocate.
 * Calculate the number of bytes actually used by *analloc*.  This is equal to `context->page << context->depth` where `context` is an `analloc_t`.

To allocate some data, use `analloc_alloc()`, and to free data use `analloc_free()`. To resize data, the `analloc_realloc()` function will do the job.

# Things to Note

### Page Size

When you initialize an allocator, you specify a `page` argument.  Whenever a call to `analloc_alloc()` or `analloc_realloc()` succeeds, the allocated chunk of memory will have length of the form `2^n * page` where `n` is some non-negative integer.  This means, for instance, that if your page size is 4096, and you try to allocate 4097 bytes, you will be given a chunk of 8192 bytes.  This will not be a problem in my operating system, where mainly single pages will be allocated at once.

### Synchronization

The `analloc_t` structure is designed for single-threaded access.  If you plan on having multiple threads all capable of allocating memory (as I do in my OS), you will need a separate mutex system.

### I don't want to have to store the damn size

Don't worry, soon enough I will add a helper function, something like `analloc_mem_size`, which takes an allocator and a poitner and returns the size of the pointer as it is currently allocated.

# License

This software is under the GNU GPL v1.
