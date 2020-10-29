#ifndef ARENA_ALLOCATOR_HPP__
#define ARENA_ALLOCATOR_HPP__

#include <memory>
#include <cstdlib>

// structure of arena:
// an arena has several pool items, linked with singly-linked list

template <typename T> 
struct minipool 
{
private:
	union minipool_item 
	{
	public:
	    minipool_item * get_next_item() const { return next; }

	    void set_next_item(minipool_item * n) { next = n; }

	    T * get_storage() { return reinterpret_cast<T *>(datum); }

	    static minipool_item * storage_to_item(T * t) 
	    {
			minipool_item *current_item = reinterpret_cast<minipool_item *>(t);
			return current_item;
	    }

	private:
	    using StorageType = char[sizeof(T)];
	    minipool_item * next;
	    StorageType datum;
	}; // minipool_item

	struct minipool_arena 
	{
	public:
		minipool_arena(size_t arena_size) : storage(new minipool_item[arena_size]) 
		{
			for (size_t idx = 1; idx < arena_size; ++idx) 
			{
				storage[idx - 1].set_next_item(&storage[idx]);
			}
			storage[arena_size - 1].set_next_item(nullptr);
		}

		minipool_item * get_storage() const { return storage.get(); }

		void set_next_arena(std::unique_ptr<minipool_arena> && n) 
		{
			assert(!next);
			next.reset(n.release());
		}

	private:
		std::unique_ptr<minipool_item[]> storage; // a pool consists several pool items
		std::unique_ptr<minipool_arena> next;
  	}; // minipool_arena

public:
	minipool(size_t arena_size) : 
	  	arena_size { arena_size }, 
	  	arena { new minipool_arena(arena_size) },
	  	free_list { arena->get_storage() } 
	    {}

	template <typename... Args> 
	T * alloc(Args && ... args) 
	{
		if (free_list == nullptr) 
		{
			std::unique_ptr<minipool_arena> new_arena(new minipool_arena(arena_size));
			new_arena->set_next_arena(std::move(arena));
			arena.reset(new_arena.release());
			free_list = arena->get_storage();
		}

	    minipool_item * current_item = free_list;
	    free_list = current_item->get_next_item();
	    T * result = current_item->get_storage();
	    new (result) T(std::forward<Args>(args)...);
	    return result;
	}

	void free(T * t) 
	{
		t->T::~T();
		minipool_item * current_item = minipool_item::storage_to_item(t);
		current_item->set_next_item(free_list);
		free_list = current_item;
	}

private:
	size_t arena_size;
	std::unique_ptr<minipool_arena> arena; // current arena
	minipool_item * free_list;

}; // minipool<T>

#endif
