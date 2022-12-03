#ifndef VM_VM_H
#define VM_VM_H
#include <stdbool.h>
#include "threads/palloc.h"
#include "lib/kernel/hash.c"

enum vm_type {
	/* page not initialized */
	VM_UNINIT = 0,
	/* page not related to the file, aka anonymous page */
	VM_ANON = 1,
	/* page that realated to the file */
	VM_FILE = 2,
	/* page that hold the page cache, for project 4 */
	VM_PAGE_CACHE = 3,

	/* Bit flags to store state */

	/* Auxillary bit flag marker for store information. You can add more
	 * markers, until the value is fit in the int. */
	VM_MARKER_0 = (1 << 3),
	VM_MARKER_1 = (1 << 4),

	/* DO NOT EXCEED THIS VALUE. */
	VM_MARKER_END = (1 << 31),
};

#include "vm/uninit.h"
#include "vm/anon.h"
#include "vm/file.h"
#ifdef EFILESYS
#include "filesys/page_cache.h"
#endif

struct page_operations;
struct thread;

#define VM_TYPE(type) ((type) & 7)

/* The representation of "page".
 * This is kind of "parent class", which has four "child class"es, which are
 * uninit_page, file_page, anon_page, and page cache (project4).
 * DO NOT REMOVE/MODIFY PREDEFINED MEMBER OF THIS STRUCTURE. */
/*
 가상주소의 key

*/
struct page {
	const struct page_operations *operations;
	void *va;              /* Address in terms of user space */
	struct frame *frame;   /* Back reference for frame */ /* 가상 주소에 매핑되는 frame 저장을 위한?*/

	/* Your implementation */

	/* --- Project 3: VM-SPT ---*/
	struct hash_elem hash_elem;
	/* --- Project 3: VM-SPT ---*/

	/* Per-type data are binded into the union.
	 * Each function automatically detects the current union */
	/*
	union으로 구성한 이유 : page 는 3가지 타입을 가지므로
	uninit_page : 페이지가 생성 될 당시의 타입
	anon_page : 매핑된 파일이 없는 비 디스크 기반 페이지
	file_backed_page : 파일 기반 페이지. 디스크에서 읽음
	*/
	union {
		struct uninit_page uninit;
		struct anon_page anon;
		struct file_page file;
#ifdef EFILESYS
		struct page_cache page_cache;
#endif
	};
};

/* The representation of "frame" */
struct frame {
	void *kva;
	struct page *page;
};

/* The function table for page operations.
 * This is one way of implementing "interface" in C.
 * Put the table of "method" into the struct's member, and
 * call it whenever you needed. */
/*
 함수 포인터를 담고 있는 함수 테이블
*/
struct page_operations {
	bool (*swap_in) (struct page *, void *);
	bool (*swap_out) (struct page *);
	void (*destroy) (struct page *);
	enum vm_type type;
};

#define swap_in(page, v) (page)->operations->swap_in ((page), v)
#define swap_out(page) (page)->operations->swap_out (page)
#define destroy(page) \
	if ((page)->operations->destroy) (page)->operations->destroy (page)

/* Representation of current process's memory space.
 * We don't want to force you to obey any specific design for this struct.
 * All designs up to you for this. */
/*
	프로세스 별로 생성되는 별도의 구조체.
	evicted page, mmaped page 등과 같이 기본 페이지 테이블인 Pml4 table이 담지 못한 정보들은 추가적으로 저장
	즉, pml4는 page fault 및 자원 관리 능력이 부족
	페이지에 대한 추가적인 데이터를 담는 (pml4에는 담기지 않는) 구조체이다. 
	또한, 페이지 사용을 완료 한 뒤, 종료될 떄 메모리 할당 해제를 위해서도 필요하다. 
	어쨌거나 spt 구조체는 thread의 Pml4와 물리메모리간 매핑을 돕기위한 보조도구에 불과
*/
struct supplemental_page_table {
	
	/* --- Project 3: VM-SPT ---*/

	/*
	프로세스가 요청한 페이지들에 한해서 메모리를 할당하고자 해시 자료구조를 활용*/
	struct hash spt_hash; //key에는 page->va값이 들어가고, 이에 호응하는 value에는 페이지의 구조체가 들어간다.

	/* --- Project 3: VM-SPT ---*/
};

#include "threads/thread.h"
void supplemental_page_table_init (struct supplemental_page_table *spt);
bool supplemental_page_table_copy (struct supplemental_page_table *dst,
		struct supplemental_page_table *src);
void supplemental_page_table_kill (struct supplemental_page_table *spt);
struct page *spt_find_page (struct supplemental_page_table *spt,
		void *va);
bool spt_insert_page (struct supplemental_page_table *spt, struct page *page);
void spt_remove_page (struct supplemental_page_table *spt, struct page *page);

void vm_init (void);
bool vm_try_handle_fault (struct intr_frame *f, void *addr, bool user,
		bool write, bool not_present);

#define vm_alloc_page(type, upage, writable) \
	vm_alloc_page_with_initializer ((type), (upage), (writable), NULL, NULL)
bool vm_alloc_page_with_initializer (enum vm_type type, void *upage,
		bool writable, vm_initializer *init, void *aux);
void vm_dealloc_page (struct page *page);
bool vm_claim_page (void *va);
enum vm_type page_get_type (struct page *page);

#endif  /* VM_VM_H */
