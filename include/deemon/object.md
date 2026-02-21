## Object Creation

For object creation, see `DeeObject_New()`

## Object Destruction

An object can be destroyed in 1 of 2 ways:
- When it's `ob_refcnt` drops to `0`
- When the garbage collector determines that the object is part of a (potentially larger) set of objects all referencing each other in a cyclic manner, which none of those objects being referenced from the outside
	- Here _referenced from the outside_ means "has an `ob_refcnt` that can't be explained by other objects from the same set"
	- The process of determining and handling unreachable GC objects with reference loops is described below, but sufficed to say: it ends with calling `DeeObject_Destroy()` just as happens when a regular object's `ob_refcnt` hits 0

When an object can be destroyed, `DeeObject_Destroy()` is called.

- `DeeObject_Destroy()` is simply a wrapper around `tp_destroy`
- When `tp_destroy` isn't defined, a default implementation is injected on first use, based on properties of the object's type (a custom override for `tp_destroy` must keep in mind that it needs to emulate these details):

```c
void default__destroy(DeeObject *self) {
	DeeTypeObject *tp = Dee_TYPE(self);

	if (HAS_CUSTOM__tp_finalize(tp)) { // Including "tp_finalize" from base-classes
		if (!DeeType_IsGC(tp) ||                                 // In GC objects...
		    !(atomic_fetchor(&DeeGC_Head(self)->gc_info.gi_flag, // ... only called once
		                     Dee_GC_FLAG_FINALIZED) &
		      Dee_GC_FLAG_FINALIZED)) {
			atomic_write(&self->ob_refcnt, 1); // Temporarily revive the object
			INVOKE_tp_finalize(self);          // Including "tp_finalize" from base-classes
			if (atomic_decfetch(&self->ob_refcnt) != 0)
				return; // Object was revived
		}
	}

	if (DeeType_IsGC(tp)) {
		// This (and everything below) can actually happen
		// asynchronously if the GC is locked right now
		DeeGC_Untrack(self);
	}

	// Invoke regular destructors
	do {
		if (tp->tp_init.tp_dtor)
			(*tp->tp_init.tp_dtor)(self);
	} while ((tp = DeeType_Base(tp)) != NULL);
	tp = Dee_TYPE(self);

	// Free heap memory
	if (tp->tp_init.tp_alloc.tp_free) {
		(*tp->tp_init.tp_alloc.tp_free)(self);
	} else if (DeeType_IsGC(tp)) {
		DeeGCObject_Free(self);
	} else {
		DeeObject_Free(self);
	}
	Dee_Decref(tp);
}
```


---

In order to resolve/fix reference loops of GC objects once a set of unreachable objects has been determined:

1. Use GC to determine set the of unreachable (GC and non-GC) objects that should be destroyed.  
   If this set contains any objects with custom `tp_finalize` callbacks that have yet to-be invoked:
   - Put all unreachable objects without `tp_finalize` back into the GC generation
   - Put all unreachable objects with `tp_finalize` into a temporary (custom) GC generation
   - Acquire references to every object from the temporary (custom) GC generation (i.e.: those with a custom `tp_finalize`)
   - Unlock GC
   - Set the `Dee_GC_FLAG_FINALIZED` flag on every object from the temporary (custom) GC generation, and only invoke `tp_finalize` for those where that flag wasn't already set
   - Drop temporary references previously acquired for objects from the temporary (custom) GC generation
   - Move all objects from the temporary (custom) GC generation into the proper GC generation **0**
   - Start over. The second time around, only newly added objects can still have a `tp_finalize` that hasn't been called, yet
2. While still locking the GC, kill `ob_weakrefs` of all those objects (weakref callbacks that would need to be invoked here are instead scheduled for later execution via `Dee_weakref_list_transfer_to_dummy()`)
3. Restore `ob_refcnt` values that were altered during step **1.**
4. Acquire 1 additional reference to every object
5. Unlock GC
6. Invoke pending weakref callbacks (`Dee_weakref_list_kill_dummy()`)
7. Invoke the internal `tp_clear` operator on every unreachable GC object  
   This operator then unbinds all `member`-style attributes of the object, which is always enough to kill all possible forms of cyclic dependencies between GC objects
8. Decref the references acquired during step **4.**  
   At this point, the objects should actually end up being destroyed, as per the usual `DeeObject_Destroy` path




