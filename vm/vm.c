/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"


/* --- Project 3: VM --- */
struct list frame_table;
static struct frame *vm_get_frame (void);
bool vm_claim_page (void *va);
bool vm_do_claim_page (struct page *page);

/* --- Project 3: VM --- */

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */

/* --- Project 3: VM --- */

list_init(&frame_table);
start = list_begin(&frame_table);

/* --- Project 3: VM --- */

}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
/*
인자로 받은 VA(가상주소)에 해당하는 페이지를 spt에서 찾아 반환한다. 
*/
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function. */

	/* --- Project 3: VM --- */
	
    struct page* page = (struct page*)malloc(sizeof(struct page));
    struct hash_elem *e;

    page->va = pg_round_down(va);  
    e = hash_find(&spt->spt_hash, &page->hash_elem); //e와 같은 해시값을 같는 페이지를 spt에서 얻은 뒤, hash_elem 구조체를 얻는다. 

    free(page);

    return e != NULL ? hash_entry(e, struct page, hash_elem) : NULL;






    /* --- Project 3: VM --- */ 
	
	// return page;

}


/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */

	return page_insert(&spt->spt_hash,page);
	// return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
/* 프레임을 받아 제거한다
*/
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	/* --- Project 3: VM --- */

	 struct thread *curr = thread_current();
	struct list_elem *e, *start;

	for (start = e; start != list_end(&frame_table); start = list_next(start)) {
		victim = list_entry(start, struct frame, frame_elem);
		if (pml4_is_accessed(curr->pml4, victim->page->va))
			pml4_set_accessed(curr->pml4, victim->page->va, 0);
		else
			return victim;
	}

	for (start = list_begin(&frame_table); start != e; start = list_next(start)) {
		victim = list_entry(start, struct frame, frame_elem);
		if (pml4_is_accessed(curr->pml4, victim->page->va))
			pml4_set_accessed(curr->pml4, victim->page->va, 0);
		else
			return victim;
	}

	/* --- Project 3: VM --- */


	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
/* 
해당 페이지에 속한 프레임의 공간을 디스크로 내리는 Swap-out을 실행하는 함수. 
디스크로 내리고자 하는 프레임이 victim이다. 
이 victim에 연결되어 있는 가상 페이지를 swap_out함수의 인자로 넣는다. 
*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim (); //Vm_get_victim를 통해 제거
	/* TODO: swap out the victim and return the evicted frame. */

	/* --- Project 3: VM --- */
	swap_out(victim->page); //swap_out은 define(매크로)되어 있다.
							//swap_out함수는, page구조체 내의 멤버인 Operations	에 속한 swap_out에 대응한다. 
	/* --- Project 3: VM --- */
	
	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
/*
Vm_get_frame()함수는 Palloc_get_page()함수의 호출로 물리 페이지(프레임)을 할당 받는다. 
*/
static struct frame *
vm_get_frame (void) {
	// struct frame *frame = NULL;
	// /* TODO: Fill this function. */

	// ASSERT (frame != NULL);
	// ASSERT (frame->page == NULL);

	/* --- Project 3: VM --- */

	struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
	
	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	frame->kva = palloc_get_page(PAL_USER); //커널 풀 대신 사용자 풀에서 메모리를 할당하고자 PAL_USER사용
	if (frame->kva == NULL) { 
		frame = vm_evict_frame();  //만약 사용가능한 메모리가 없다면, 공간을 만들기 위해 현재 사용중인 프레임을 축출한다. 
		frame->page = NULL;
		return frame;
	}
	list_push_back(&frame_table, &frame->frame_elem);
	frame->page = NULL;


	/* --- Project 3: VM --- */


	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
/*
Claim()함수는 프레임을 페이지에 할당한다. vm_get_frame이 Frame을 가져오는 함수였다면, vm_claim_page는 인자의 va를 이용해
spt에서 frame과 연결할 페이지를 찾는다. 
*/
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */

	/* --- Project 3: VM --- */

	page = spt_find_page(&thread_current()->spt, va);
	if (page == NULL)
		return false;

	/* --- Project 3: VM --- */

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	/* --- Project 3: VM --- */

	if(install_page(page->va, frame->kva, page->writable)) {
		return swap_in(page, frame->kva);
	}
	return false;

	/* --- Project 3: VM --- */
	// return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {

	/* --- Project 3: VM --- */
	hash_init(&spt ->spt_hash, page_hash, page_less ,NULL);
	/* --- Project 3: VM --- */

}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}

/* --- Project 3: VM --- */

/*
해시 테이블 초기화 시, 해당 해시 값을 구해주는 함수의 포인터이다. */
unsigned page_hash (const struct hash_elem *p_ , void *aux UNUSED){
	const struct page* p = hash_entry(p_, struct page, hash_elem); //hash_elem을 토대로 해당 페이지가 속한 해시 테이블의 주소를 가져옴
	return hash_bytes(&p -> va, sizeof(p -> va));

}

/*
해시 테이블 초기화 시, 해시 요소들을 비교하는 함수의 포인터이다. 
a가 b보다 작으면 true를 반환한다. */
static unsigned page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
	const struct page *p_a = hash_entry(a, struct page, hash_elem);
	const struct page *p_b = hash_entry(b, struct page, hash_elem);
	return p_a -> va < p_b -> va;

}

bool page_insert(struct hash *h, struct page *p){
	 
	 if(!hash_insert(h, &p -> hash_elem))
	 	return true;
	 else
	 	return false;

}

bool page_delete(struct hash*h, struct page *p){

	if (!hash_delete(h, &p ->hash_elem))
		return true;
	else 
		return false;
}

/* --- Project 3: VM --- */

