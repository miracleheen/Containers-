#ifndef Vector
#include <string>

//miv --> my implementation of the vector :P
namespace miv {
	struct Range_error : std::out_of_range {
		std::int32_t index;
		Range_error(std::int32_t i) : std::out_of_range("Range error: " + std::to_string(i)), index(i) { }
	}; 

	template<typename T>
	class Allocator {
	public:
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;
	public:
		template<typename U>
		struct rebind {
			typedef Allocator<U> other;
		};
	public:
		explicit Allocator() {}
		explicit Allocator(Allocator const&) {}

		template<typename U>
		explicit Allocator(Allocator<U> const&) {}

		pointer address(reference r) const {
			return &r;
		}

		const_pointer address(const_reference r) const {
			return &r;
		}

		pointer allocate(size_type cnt,
			typename std::allocator<void>::const_pointer = 0) {
			return reinterpret_cast<pointer>(::operator new(cnt * sizeof(T)));
		}

		void deallocate(pointer p, size_type) {
			::operator delete(p);
		}

		size_type max_size() const {
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		void construct(pointer p, const T& t) {
			new(p) T(t);
		}

		void destroy(pointer p) {
			p->~T();
		}

		bool operator== (Allocator const&) {
			return true;
		}

		bool operator!= (Allocator const& a) {
			return !operator==(a);
		}

		~Allocator() {}
	};

	template<typename T, typename A = Allocator<T>>
	class Vector {
	private:
		T* elem{};
		A alloc{};
		std::size_t sz{};
		std::size_t space{};
		using alloc_traits = std::allocator_traits<A>;
	public:
		Vector(const A& alloc = A()) :sz{ 0 }, space{ 0 }, elem{ nullptr } {}

		explicit Vector(std::size_t N, const T& value = T(), const A& a = A()) : sz{ N }, space{ N } {
			elem = alloc_traits::allocate(alloc, N);
			try {
				std::uninitialized_fill(elem, elem + N, value);
			}
			catch (...) {
				alloc_traits::deallocate(alloc, elem, N);
				throw;
			}
		}

		explicit Vector(std::initializer_list<T> lst, const A& a = A()) :sz{lst.size()}, space{lst.size()} {
			elem = alloc_traits::allocate(alloc, lst.size());
			try {
				std::copy(lst.begin(), lst.end(), elem);
			}
			catch (...) {
				alloc_traits::deallocate(alloc, elem, lst.size());
				throw;
			}
		}

		Vector(const Vector& args) {
			elem = alloc_traits::allocate(alloc, args.sz);
			try {
				std::copy(args.elem, args.elem + args.sz, this->elem);
			}
			catch (...) {
				alloc_traits::deallocate(alloc, elem, args.sz);
				throw;
			}
			space = sz = args.sz;
		}

		Vector& operator=(const Vector& another)& {
			if (this == &another) {
				return *this;
			}

			if (another.sz <= space) {
				for (std::size_t i{ 0 }; i < another.sz; ++i) {
					alloc_traits::construct(alloc, elem + i, std::move(another.elem[i]));
				}
				sz = another.sz;
				return *this;
			}

			if (alloc_traits::propagate_on_container_copy_assignment::value && alloc != another.alloc) {
				alloc = another.alloc;//exception?!
			}

			T* copy_elem = alloc_traits::allocate(alloc, another.space);
			std::size_t i = 0;
			try {
				for (; i < another.sz; ++i) {
					alloc_traits::construct(alloc, copy_elem + i, std::move(another.elem[i]));
				}
			}
			catch (...) {
				for (std::size_t j = { 0 }; j < i; ++j) {
					alloc_traits::destroy(alloc, copy_elem + j);
				}
				alloc_traits::deallocate(alloc, copy_elem, another.space);
				throw;
			}

			for (std::size_t i = { 0 }; i < sz; ++i) {
				alloc_traits::destroy(alloc, copy_elem + i);
			}

			alloc_traits::deallocate(alloc, elem, space);
			elem = copy_elem;
			space = sz = another.sz;
			return *this;
		}

		Vector(Vector&& args) noexcept : sz{ 0 }, space{ 0 }, elem{ nullptr } {
			*this = std::move(args);
		}

		constexpr Vector& operator=(Vector&& args) noexcept {
			elem = args.elem;
			sz = args.sz;
			space = args.space;

			args.elem = nullptr;
			args.sz = args.space = 0;
			return *this;
		}

		void reserve(std::size_t newalloc) {
			if (newalloc <= space) {
				return;
			}
			T* new_elem = alloc_traits::allocate(alloc, newalloc);

			std::size_t i = 0;
			try {
				for (; i < sz; ++i) {
					alloc_traits::construct(alloc, new_elem + i, std::move_if_noexcept(elem[i]));
				}
			}
			catch (...) {
				for (size_t j = { 0 }; j < i; ++j) {
					alloc_traits::destroy(alloc, new_elem + j);
				}
				alloc_traits::deallocate(alloc, new_elem, newalloc);
				throw;
			}

			for (std::size_t i = { 0 }; i < sz; ++i) {
				alloc_traits::destroy(alloc, new_elem + i);
			}

			alloc_traits::deallocate(alloc, elem, space);
			elem = new_elem;
			space = newalloc;
		}

		void resize(std::size_t newsize, T value = T()) {
			if (newsize <= 0) return;
			reserve(newsize);
			for (std::size_t i{ sz }; i < newsize; ++i)
				elem[i] = 0;
			sz = newsize;
		}

		void push_back(const T& value) {
			if (space == 0)
				reserve(8);
			else if (sz == space)
				reserve(2 * space);

			alloc_traits::construct(alloc, elem + sz, value);
			++sz;
		}

		void push_back(T&& value) {
			if (space == 0)
				reserve(8);
			else if (sz == space)
				reserve(2 * space);

			alloc_traits::construct(alloc, elem + sz, std::move(value));
			++sz;
		}

		template<typename...Args>
		void emplace_back(Args&&...args) {
			if (space == 0)
				reserve(8);
			else if (sz == space)
				reserve(2 * space);

			alloc_traits::construct(alloc, elem + sz, std::forward<Args>(args)...);
			++sz;
		}

		void pop_back() {
			alloc_traits::destroy(alloc, elem + sz - 1);
			--sz;
		}

		T& operator[](std::size_t index) {
			if (index < 0 || this->size() <= index) {
				throw Range_error(index);
			}
			return elem[index];
		}

		const T& operator[](std::size_t index) const {
			if (index < 0 || this->size() <= index) {
				throw Range_error(index);
			}
			return elem[index];
		}

		std::size_t capacity() const {
			return space;
		}

		std::size_t size() const {
			return sz;
		}

		void shrink_to_fit() {
			space = sz;
		}

		std::size_t at(std::size_t index) {
			if (index < 0 || this->size() <= index) {
				throw Range_error(index);
			}
			else {
				return elem[index];
			}
		}

		const std::size_t at(std::size_t index) const {
			if (index < 0 || this->size() <= index) {
				throw Range_error(index);
			}
			else {
				return elem[index];
			}
		}

		template<typename T>
		struct Is_Vector {
			static const bool value = false;
		};

		template<typename T, typename A>
		struct Is_Vector<Vector<T, A>> {
			static const bool value = true;
		};

		class iterator {
		private:
			T* ptr;
		public:
			iterator() : ptr{ nullptr } {}
			explicit iterator(T* p) : ptr{ p } {}

			iterator& operator=(const iterator& iter) = default;
			~iterator() = default;

			T& operator*() const {
				return *ptr;
			}

			T& operator[](int num) {
				return *(ptr + num);
			}

			T* operator->() const {
				return ptr;
			}

			iterator& operator++() {
				++ptr;
				return *this;
			}

			iterator operator++(int) {
				iterator temp(*this);
				++* this;
				return temp;
			}

			iterator& operator--() {
				--ptr;
				return *this;
			}

			iterator operator--(int) {
				iterator temp(*this);
				--* this;
				return temp;
			}

			iterator operator+(int num) {
				iterator temp(ptr + num);
				return temp;
			}

			iterator operator-(int num) {
				iterator temp(ptr - num);
				return temp;
			}

			bool operator!=(const iterator& rhs) const {
				return !(*this == rhs);
			}

			bool operator==(const iterator& iter) const {
				return ptr == iter.ptr;
			}

			bool operator<(const iterator& iter) const {
				return ptr < iter.ptr;
			}
			bool operator>(const iterator& iter) const {
				return ptr > iter.ptr;
			}

			bool operator<=(const iterator& iter) const {
				return ptr <= iter.ptr;
			}

			bool operator>=(const iterator& iter) const {
				return ptr >= iter.ptr;
			}
		};

		iterator begin() const
		{
			return iterator(elem);
		}

		iterator end() const
		{
			return iterator(elem + sz);
		}
		
		bool operator>=(const Vector& v) const {
			if (sz >= v.sz || space >= v.space) {
				return true;
			}
			return false;
		}

		bool operator<=(const Vector& v) const {
			if (sz <= v.sz || space <= v.space) {
				return true;
			}
			return false;
		}

		bool operator>(const Vector& v) const {
			if (sz > v.sz || space > v.space) {
				return true;
			}
			return false;
		}

		bool operator<(const Vector& v) const {
			if (sz < v.sz || space < v.space) {
				return true;
			}
			return false;
		}

		bool operator!=(const Vector& v) const {
			return !operator==(v);
		}

		bool operator==(const Vector& v) const {
			if (sz != v.sz || space != v.space) {
				return false;
			}

			for (std::size_t i = { 0 }; i < sz; ++i) {
				if (elem[i] != v.elem[i]) {
					return false;
				}
			}
			return true;
		}

		~Vector() {
			//alloc_traits::deallocate(alloc, elem, space);
			/*for (std::size_t i{ 0 }; i < sz; ++i) {//dest.of placement new.
				(elem + i)->~T();
			}*/
		}
	};

	template <>//modest implementation
	class Vector<bool> {
	private:
		std::int8_t* elem{};
		struct bit_reference {
			std::int8_t* cell{};
			std::uint8_t num{};
			bit_reference(std::uint8_t, std::uint8_t) {}

			bit_reference operator=(bool log) {
				if (log) {
					*cell |= (1u << num);
				}
				else {
					*cell &= ~(1u << num);
				}
				return *this;
			}

			operator bool() const {
				return *cell & (1u << num);
			}
		};
	public:
		bit_reference operator[](std::size_t i) {
			std::uint8_t pos = i % 8;
			std::uint8_t ptr = *elem + i / 8;
			return bit_reference(ptr, pos);
		}
	};
}
#endif
