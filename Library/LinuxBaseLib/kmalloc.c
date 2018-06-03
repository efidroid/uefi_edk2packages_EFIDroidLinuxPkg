#include <LinuxBase.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/bug.h>

#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>

#define KMEM_HEAD_SIGNATURE   SIGNATURE_32('k','m','e','m')

struct kmem_cache {
	unsigned int object_size;/* The original size of the object */
	unsigned int align;	/* Alignment as calculated */
	slab_flags_t flags;	/* Active flags on the slab */
	const char *name;	/* Slab name for sysfs */
	void (*ctor)(void *);	/* Called on object slot creation */
};

enum mem_type {
	MEMTYPE_POOL,
	MEMTYPE_ALIGNED_PAGES,
};

struct mem_head {
	uint32_t signature;
	void *base_ptr;
	size_t base_size;

	gfp_t flags;
	size_t size;
	size_t align;
	enum mem_type type;
	u8 data[0];
};

static void *__do_alloc(size_t size, gfp_t flags, size_t align);

struct kmem_cache *kmem_cache_create(const char *name, size_t size,
			size_t align, slab_flags_t flags,
			void (*ctor)(void *))
{
	struct kmem_cache *c;

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return NULL;

	c->object_size = size;
	c->align = align;
	c->flags = flags;
	c->name = name;
	c->ctor = ctor;

	return c;
}

void kmem_cache_destroy(struct kmem_cache *c) {
	if (unlikely(!c))
		return;

	kfree(c);
}

void *kmem_cache_alloc(struct kmem_cache *c, gfp_t flags) {
	void *b;

	b = __do_alloc(c->object_size, flags, c->align);
	b = kmalloc(c->object_size, flags);

	if (b && c->ctor)
		c->ctor(b);

	return b;
}

void kmem_cache_free(struct kmem_cache *c, void *p) {
	kfree(p);
}

void kmem_cache_free_bulk(struct kmem_cache *c, size_t nr, void **p)
{
	size_t i;

	for (i = 0; i < nr; i++) {
		if (c)
			kmem_cache_free(c, p[i]);
		else
			kfree(p[i]);
	}
}
EXPORT_SYMBOL(kmem_cache_free_bulk);

int kmem_cache_alloc_bulk(struct kmem_cache *c, gfp_t flags, size_t nr,
								void **p)
{
	size_t i;

	for (i = 0; i < nr; i++) {
		void *x = p[i] = kmem_cache_alloc(c, flags);
		if (!x) {
			kmem_cache_free_bulk(c, i, p);
			return 0;
		}
	}
	return i;
}
EXPORT_SYMBOL(kmem_cache_alloc_bulk);

static __always_inline void *__do_krealloc(const void *p, size_t new_size,
					   gfp_t flags)
{
	void *ret;
	size_t ks = 0;

	if (p)
		ks = ksize(p);

	ret = kmalloc_track_caller(new_size, flags);
	if (ret && p)
		memcpy(ret, p, ks);

	return ret;
}

void *krealloc(const void *p, size_t new_size, gfp_t flags)
{
	void *ret;

	if (unlikely(!new_size)) {
		kfree(p);
		return ZERO_SIZE_PTR;
	}

	ret = __do_krealloc(p, new_size, flags);
	if (ret && p != ret)
		kfree(p);

	return ret;
}
EXPORT_SYMBOL(krealloc);

void kzfree(const void *p)
{
	size_t ks;
	void *mem = (void *)p;

	if (unlikely(ZERO_OR_NULL_PTR(mem)))
		return;
	ks = ksize(mem);
	memset(mem, 0, ks);
	kfree(mem);
}
EXPORT_SYMBOL(kzfree);

void *__kmalloc_track_caller(size_t size, gfp_t flags, unsigned long caller) {
	return kmalloc(size, flags);
}

static void *__do_alloc(size_t size, gfp_t flags, size_t align)
{
	void *base_mem;
	struct mem_head *head;
	size_t head_size;
	size_t base_size;
	enum mem_type type;

	if (size == 0) {
		return ZERO_SIZE_PTR;
	}

	if (align == 0)
		align = 8;

	if (align == 8)
		type = MEMTYPE_POOL;
	else
		type = MEMTYPE_ALIGNED_PAGES;

	head_size = ALIGN_VALUE(sizeof(*head), align);
	base_size = head_size + size;

	if (type == MEMTYPE_POOL)
		base_mem = AllocatePool(base_size);
	else if (type == MEMTYPE_ALIGNED_PAGES)
		base_mem = AllocateAlignedPages(EFI_SIZE_TO_PAGES(base_size), align);
	else
		BUG();
	if (base_mem == NULL) {
		return NULL;
	}

	head = base_mem + head_size - sizeof(*head);
	head->signature = KMEM_HEAD_SIGNATURE;
	head->base_ptr = base_mem;
	head->base_size = base_size;
	head->flags = flags;
	head->size = size;
	head->align = align;
	head->type = type;

	return head->data;
}

void *__kmalloc(size_t size, gfp_t flags) {
	if (!size) {
		return ZERO_SIZE_PTR;
	}

	return __do_alloc(size, flags, 8);
}
EXPORT_SYMBOL(__kmalloc);

static __always_inline struct mem_head* get_mem_head(const void *p) {
	struct mem_head *head;

	head = BASE_CR(p, struct mem_head, data);
	BUG_ON(head->signature != KMEM_HEAD_SIGNATURE);

	return head;
}

void kfree(const void *p) {
	struct mem_head *head;

	if (unlikely(ZERO_OR_NULL_PTR(p)))
		return;

	head = get_mem_head(p);
	BUG_ON(!head);

	if (head->type == MEMTYPE_POOL)
		FreePool (head->base_ptr);
	else if (head->type == MEMTYPE_ALIGNED_PAGES)
		FreeAlignedPages (head->base_ptr, EFI_SIZE_TO_PAGES(head->base_size));
	else
		BUG();
}
EXPORT_SYMBOL(kfree);

size_t ksize(const void *p) {
	struct mem_head *head;

	BUG_ON(!p);

	if (unlikely(p == ZERO_SIZE_PTR))
		return 0;

	head = get_mem_head(p);
	BUG_ON(!head);

	return head->size;
}
EXPORT_SYMBOL(ksize);
