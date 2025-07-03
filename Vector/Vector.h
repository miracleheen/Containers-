#pragma once

#include <memory>
#include <stdexcept>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cstddef>
#include <string>
#include <type_traits>

namespace miv
{
	struct Range_error : std::out_of_range
	{
		std::int32_t index;
		Range_error(std::int32_t i)
			: std::out_of_range("Range error: " + std::to_string(i)), index(i)
		{
		}
	};

	template <typename T>
	class Allocator
	{
	public:
		using value_type = T;
		using pointer = T *;
		using const_pointer = const T *;
		using reference = T &;
		using const_reference = const T &;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		template <typename U>
		struct rebind
		{
			using other = Allocator<U>;
		};

		Allocator() noexcept = default;
		template <typename U>
		Allocator(const Allocator<U> &) noexcept {}

		pointer allocate(size_type n)
		{
			if (n > max_size())
				throw std::bad_alloc();
			return static_cast<pointer>(::operator new(n * sizeof(T)));
		}
		void deallocate(pointer p, size_type) noexcept
		{
			::operator delete(p);
		}

		size_type max_size() const noexcept
		{
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		template <typename... Args>
		void construct(pointer p, Args &&...args)
		{
			::new ((void *)p) T(std::forward<Args>(args)...);
		}
		void destroy(pointer p) noexcept
		{
			p->~T();
		}

		bool operator==(const Allocator &) const noexcept { return true; }
		bool operator!=(const Allocator &) const noexcept { return false; }
	};

	// Random-access итератор
	template <typename T>
	class VectorIterator
	{
	private:
		template <typename U>
		friend class VectorIterator;
		T *ptr_;

	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T *;
		using reference = T &;

		VectorIterator() noexcept : ptr_(nullptr) {}
		explicit VectorIterator(T *p) noexcept : ptr_(p) {}

		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
		VectorIterator(const VectorIterator<U> &other) noexcept
			: ptr_(other.ptr_)
		{
		}

		constexpr reference operator*() const noexcept { return *ptr_; }
		constexpr pointer operator->() const noexcept { return ptr_; }
		constexpr reference operator[](difference_type n) const noexcept { return ptr_[n]; }

		constexpr VectorIterator &operator++() noexcept
		{
			++ptr_;
			return *this;
		}
		constexpr VectorIterator operator++(int) noexcept
		{
			VectorIterator tmp(*this);
			++ptr_;
			return tmp;
		}
		VectorIterator &operator--() noexcept
		{
			--ptr_;
			return *this;
		}
		VectorIterator operator--(int) noexcept
		{
			VectorIterator tmp(*this);
			--ptr_;
			return tmp;
		}

		constexpr VectorIterator operator+(difference_type n) const noexcept { return VectorIterator(ptr_ + n); }
		VectorIterator operator-(difference_type n) const noexcept { return VectorIterator(ptr_ - n); }
		constexpr difference_type operator-(const VectorIterator &o) const noexcept { return ptr_ - o.ptr_; }

		bool operator==(const VectorIterator &o) const noexcept { return ptr_ == o.ptr_; }
		bool operator!=(const VectorIterator &o) const noexcept { return ptr_ != o.ptr_; }
		bool operator<(const VectorIterator &o) const noexcept { return ptr_ < o.ptr_; }
		bool operator>(const VectorIterator &o) const noexcept { return ptr_ > o.ptr_; }
		bool operator<=(const VectorIterator &o) const noexcept { return ptr_ <= o.ptr_; }
		bool operator>=(const VectorIterator &o) const noexcept { return ptr_ >= o.ptr_; }
	};

	// Основная реализация vector<T>
	template <typename T, typename A = Allocator<T>>
	class Vector
	{
	public:
		using value_type = T;
		using allocator_type = A;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = T &;
		using const_reference = const T &;
		using pointer = T *;
		using const_pointer = const T *;
		using iterator = VectorIterator<T>;
		using const_iterator = VectorIterator<const T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using alloc_traits = std::allocator_traits<allocator_type>;

		Vector(const allocator_type &alloc = allocator_type()) noexcept
			: alloc_(alloc), elem_(nullptr), sz_(0), space_(0)
		{
		}

		explicit Vector(size_type n, const T &value = T(),
						const allocator_type &alloc = allocator_type())
			: alloc_(alloc), elem_(alloc_traits::allocate(alloc_, n)), sz_(n), space_(n)
		{
			std::uninitialized_fill(elem_, elem_ + n, value);
		}

		template <typename InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
		Vector(InputIt first, InputIt last,
			   const allocator_type &alloc = allocator_type())
			: alloc_(alloc), elem_(nullptr), sz_(0), space_(0)
		{
			for (; first != last; ++first)
				push_back(*first);
		}

		Vector(std::initializer_list<T> il,
			   const allocator_type &alloc = allocator_type())
			: Vector(il.begin(), il.end(), alloc)
		{
		}

		Vector(const Vector &other)
			: alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_)), elem_(alloc_traits::allocate(alloc_, other.sz_)), sz_(other.sz_), space_(other.sz_)
		{
			std::uninitialized_copy(other.elem_, other.elem_ + sz_, elem_);
		}

		Vector(Vector &&other) noexcept
			: alloc_(std::move(other.alloc_)), elem_(other.elem_), sz_(other.sz_), space_(other.space_)
		{
			other.elem_ = nullptr;
			other.sz_ = other.space_ = 0;
		}

		Vector &operator=(const Vector &other)
		{
			if (this == &other)
				return *this;
			if constexpr (alloc_traits::propagate_on_container_copy_assignment::value)
			{
				if (alloc_ != other.alloc_)
					alloc_ = other.alloc_;
			}
			Vector tmp(other);
			swap(tmp);
			return *this;
		}

		Vector &operator=(Vector &&other) noexcept(
			alloc_traits::propagate_on_container_move_assignment::value)
		{
			clear();
			if constexpr (alloc_traits::propagate_on_container_move_assignment::value)
			{
				alloc_ = std::move(other.alloc_);
			}
			elem_ = other.elem_;
			sz_ = other.sz_;
			space_ = other.space_;
			other.elem_ = nullptr;
			other.sz_ = other.space_ = 0;
			return *this;
		}

		constexpr bool empty() const noexcept { return sz_ == 0; }
		constexpr size_type size() const noexcept { return sz_; }
		constexpr size_type capacity() const noexcept { return space_; }

		void reserve(size_type new_cap)
		{
			if (new_cap <= space_)
				return;
			pointer new_elem = alloc_traits::allocate(alloc_, new_cap);
			for (size_type i = 0; i < sz_; ++i)
			{
				alloc_traits::construct(alloc_, new_elem + i,
										std::move_if_noexcept(elem_[i]));
				alloc_traits::destroy(alloc_, elem_ + i);
			}
			if (elem_)
				alloc_traits::deallocate(alloc_, elem_, space_);
			elem_ = new_elem;
			space_ = new_cap;
		}

		void shrink_to_fit()
		{
			if (space_ > sz_)
				reserve(sz_);
		}

		// Модификаторы
		void clear() noexcept
		{
			for (size_type i = 0; i < sz_; ++i)
				alloc_traits::destroy(alloc_, elem_ + i);
			sz_ = 0;
		}

		void resize(size_type count, const T &value = T())
		{
			if (count < sz_)
			{
				for (size_type i = count; i < sz_; ++i)
					alloc_traits::destroy(alloc_, elem_ + i);
			}
			else
			{
				reserve(count);
				for (size_type i = sz_; i < count; ++i)
					alloc_traits::construct(alloc_, elem_ + i, value);
			}
			sz_ = count;
		}

		void push_back(const T &v)
		{
			if (space_ == 0)
				reserve(8);
			else if (sz_ == space_)
				reserve(2 * space_);
			alloc_traits::construct(alloc_, elem_ + sz_, v);
			++sz_;
		}
		void push_back(T &&v)
		{
			if (space_ == 0)
				reserve(8);
			else if (sz_ == space_)
				reserve(2 * space_);
			alloc_traits::construct(alloc_, elem_ + sz_, std::move(v));
			++sz_;
		}

		template <typename... Args>
		reference emplace_back(Args &&...args)
		{
			if (space_ == 0)
				reserve(8);
			else if (sz_ == space_)
				reserve(2 * space_);
			alloc_traits::construct(alloc_, elem_ + sz_, std::forward<Args>(args)...);
			return elem_[sz_++];
		}

		void pop_back() noexcept
		{
			if (sz_ > 0)
				alloc_traits::destroy(alloc_, elem_ + --sz_);
		}

		// assign
		void assign(size_type count, const T &value)
		{
			clear();
			reserve(count);
			for (size_type i = 0; i < count; ++i)
				alloc_traits::construct(alloc_, elem_ + i, value);
			sz_ = count;
		}

		template <typename InputIt,
				  typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
		void assign(InputIt first, InputIt last)
		{
			clear();
			for (; first != last; ++first)
				push_back(*first);
		}

		void assign(std::initializer_list<T> il)
		{
			assign(il.begin(), il.end());
		}

		// insert - emplace - erase
		iterator insert(const_iterator pos, const T &value)
		{
			return emplace(pos, value);
		}

		iterator insert(const_iterator pos, size_type count, const T &value)
		{
			size_type idx = pos - begin();
			if (sz_ + count > space_)
				reserve(std::max(space_ * 2, sz_ + count));
			for (size_type i = sz_; i > idx; --i)
			{
				alloc_traits::construct(alloc_, elem_ + i + count - 1,
										std::move_if_noexcept(elem_[i - 1]));
				alloc_traits::destroy(alloc_, elem_ + i - 1);
			}
			for (size_type i = 0; i < count; ++i)
				alloc_traits::construct(alloc_, elem_ + idx + i, value);
			sz_ += count;
			return iterator(elem_ + idx);
		}

		template <typename InputIt,
				  typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
		iterator insert(const_iterator pos, InputIt first, InputIt last)
		{
			size_type idx = pos - begin();
			size_type count = std::distance(first, last);
			if (sz_ + count > space_)
				reserve(std::max(space_ * 2, sz_ + count));
			for (size_type i = sz_; i > idx; --i)
			{
				alloc_traits::construct(alloc_, elem_ + i + count - 1,
										std::move_if_noexcept(elem_[i - 1]));
				alloc_traits::destroy(alloc_, elem_ + i - 1);
			}
			for (; first != last; ++first, ++idx)
				alloc_traits::construct(alloc_, elem_ + idx, *first);
			sz_ += count;
			return iterator(elem_ + (pos - begin()));
		}

		iterator insert(const_iterator pos, std::initializer_list<T> il)
		{
			return insert(pos, il.begin(), il.end());
		}

		template <typename... Args>
		iterator emplace(const_iterator pos, Args &&...args)
		{
			size_type idx = pos - begin();
			if (space_ == 0)
				reserve(8);
			else if (sz_ == space_)
				reserve(2 * space_);
			for (size_type i = sz_; i > idx; --i)
			{
				alloc_traits::construct(alloc_, elem_ + i,
										std::move_if_noexcept(elem_[i - 1]));
				alloc_traits::destroy(alloc_, elem_ + i - 1);
			}
			alloc_traits::construct(alloc_, elem_ + idx, std::forward<Args>(args)...);
			++sz_;
			return iterator(elem_ + idx);
		}

		iterator erase(const_iterator pos)
		{
			size_type idx = pos - begin();
			alloc_traits::destroy(alloc_, elem_ + idx);
			for (size_type i = idx; i < sz_ - 1; ++i)
			{
				alloc_traits::construct(alloc_, elem_ + i,
										std::move_if_noexcept(elem_[i + 1]));
				alloc_traits::destroy(alloc_, elem_ + i + 1);
			}
			--sz_;
			return iterator(elem_ + idx);
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			size_type idx = first - begin();
			size_type count = last - first;
			for (size_type i = idx; i < idx + count; ++i)
				alloc_traits::destroy(alloc_, elem_ + i);
			for (size_type i = idx + count; i < sz_; ++i)
			{
				alloc_traits::construct(alloc_, elem_ + i - count,
										std::move_if_noexcept(elem_[i]));
				alloc_traits::destroy(alloc_, elem_ + i);
			}
			sz_ -= count;
			return iterator(elem_ + idx);
		}

		void swap(Vector &other) noexcept(
			alloc_traits::propagate_on_container_swap::value)
		{
			if constexpr (alloc_traits::propagate_on_container_swap::value)
				std::swap(alloc_, other.alloc_);
			std::swap(elem_, other.elem_);
			std::swap(sz_, other.sz_);
			std::swap(space_, other.space_);
		}

		constexpr reference operator[](size_type i) noexcept { return elem_[i]; }
		constexpr const_reference operator[](size_type i) const noexcept { return elem_[i]; }

		reference at(size_type i)
		{
			if (i >= sz_)
				throw Range_error(i);
			return elem_[i];
		}
		const_reference at(size_type i) const
		{
			if (i >= sz_)
				throw Range_error(i);
			return elem_[i];
		}

		reference front() noexcept { return elem_[0]; }
		const_reference front() const noexcept { return elem_[0]; }
		reference back() noexcept { return elem_[sz_ - 1]; }
		const_reference back() const noexcept { return elem_[sz_ - 1]; }

		constexpr pointer data() noexcept { return elem_; }
		constexpr const_pointer data() const noexcept { return elem_; }

		iterator begin() noexcept { return iterator(elem_); }
		const_iterator begin() const noexcept { return const_iterator(elem_); }
		iterator end() noexcept { return iterator(elem_ + sz_); }
		const_iterator end() const noexcept { return const_iterator(elem_ + sz_); }

		reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
		reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
		const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
		const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

		allocator_type get_allocator() const noexcept
		{
			return alloc_;
		}

		size_type max_size() const noexcept
		{
			return alloc_traits::max_size(alloc_);
		}

		~Vector()
		{
			clear();
			if (elem_)
				alloc_traits::deallocate(alloc_, elem_, space_);
		}

	private:
		allocator_type alloc_;
		pointer elem_;
		size_type sz_, space_;
	};

	// vector<bool>
	template <typename A>
	class Vector<bool, A>
	{
	public:
		using value_type = bool;
		using size_type = std::size_t;
		using allocator_type = A;
		using byte_allocator =
			typename std::allocator_traits<A>::template rebind_alloc<unsigned char>;
		using alloc_traits = std::allocator_traits<byte_allocator>;
		using reference = struct bit_reference;
		using const_reference = bool;

		// proxy для 1 бита
		struct bit_reference
		{
			unsigned char *byte_;
			unsigned char mask_;
			bit_reference(unsigned char *b, unsigned char m) noexcept
				: byte_(b), mask_(m) {}
			bit_reference &operator=(bool v) noexcept
			{
				if (v)
					*byte_ |= mask_;
				else
					*byte_ &= ~mask_;
				return *this;
			}
			operator bool() const noexcept
			{
				return (*byte_ & mask_) != 0;
			}
		};

	private:
		byte_allocator alloc_;
		unsigned char *data_;
		size_type sz_, space_;

		static size_type bytes_for_bits(size_type b) noexcept
		{
			return (b + 7) / 8;
		}

	public:
		Vector() noexcept
			: alloc_(byte_allocator()), data_(nullptr), sz_(0), space_(0)
		{
		}

		explicit Vector(size_type n, bool v = false,
						const allocator_type & = allocator_type())
			: alloc_(byte_allocator()), sz_(n), space_(n)
		{
			size_type bytes = bytes_for_bits(n);
			data_ = alloc_traits::allocate(alloc_, bytes);
			unsigned char fill = v ? 0xFFu : 0x00u;
			std::fill_n(data_, bytes, fill);
		}

		Vector(std::initializer_list<bool> il,
			   const allocator_type &alloc = allocator_type())
			: Vector(il.size(), false, alloc)
		{
			size_type i = 0;
			for (bool b : il)
				(*this)[i++] = b;
		}

		Vector(const Vector &other)
			: alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_)), sz_(other.sz_), space_(other.space_)
		{
			size_type bytes = bytes_for_bits(space_);
			data_ = alloc_traits::allocate(alloc_, bytes);
			std::copy(other.data_, other.data_ + bytes, data_);
		}

		Vector(Vector &&other) noexcept
			: alloc_(std::move(other.alloc_)), data_(other.data_), sz_(other.sz_), space_(other.space_)
		{
			other.data_ = nullptr;
			other.sz_ = other.space_ = 0;
		}

		Vector &operator=(const Vector &other)
		{
			if (this == &other)
				return *this;
			Vector tmp(other);
			swap(tmp);
			return *this;
		}

		Vector &operator=(Vector &&other) noexcept
		{
			if (this != &other)
			{
				if (data_)
					alloc_traits::deallocate(alloc_, data_, bytes_for_bits(space_));
				alloc_ = std::move(other.alloc_);
				data_ = other.data_;
				sz_ = other.sz_;
				space_ = other.space_;
				other.data_ = nullptr;
				other.sz_ = other.space_ = 0;
			}
			return *this;
		}

		void swap(Vector &other) noexcept
		{
			std::swap(alloc_, other.alloc_);
			std::swap(data_, other.data_);
			std::swap(sz_, other.sz_);
			std::swap(space_, other.space_);
		}

		bool empty() const noexcept { return sz_ == 0; }
		size_type size() const noexcept { return sz_; }
		size_type capacity() const noexcept { return space_; }

		void reserve(size_type new_cap)
		{
			if (new_cap <= space_)
				return;
			size_type new_bytes = bytes_for_bits(new_cap);
			unsigned char *new_data = alloc_traits::allocate(alloc_, new_bytes);
			size_type old_bytes = bytes_for_bits(sz_);
			std::copy(data_, data_ + old_bytes, new_data);
			if (data_)
				alloc_traits::deallocate(alloc_, data_, bytes_for_bits(space_));
			data_ = new_data;
			space_ = new_cap;
		}

		void resize(size_type new_size, bool v = false)
		{
			if (new_size > space_)
				reserve(new_size);
			for (size_type i = sz_; i < new_size; ++i)
				(*this)[i] = v;
			sz_ = new_size;
		}

		void push_back(bool v)
		{
			if (space_ == 0)
				reserve(8);
			else if (sz_ == space_)
				reserve(2 * space_);
			(*this)[sz_++] = v;
		}
		void pop_back()
		{
			if (sz_ > 0)
				--sz_;
		}

		// element access
		reference operator[](size_type i) noexcept
		{
			unsigned char *byte = data_ + (i / 8);
			unsigned char mask = static_cast<unsigned char>(1u << (i % 8));
			return bit_reference(byte, mask);
		}
		const_reference operator[](size_type i) const noexcept
		{
			return (data_[i / 8] >> (i % 8)) & 1u;
		}

		reference at(size_type i)
		{
			if (i >= sz_)
				throw Range_error(i);
			return (*this)[i];
		}
		const_reference at(size_type i) const
		{
			if (i >= sz_)
				throw Range_error(i);
			return (*this)[i];
		}

		void clear() noexcept { sz_ = 0; }

		unsigned char *data() noexcept { return data_; }
		const unsigned char *data() const noexcept { return data_; }

		~Vector()
		{
			if (data_)
				alloc_traits::deallocate(alloc_, data_, bytes_for_bits(space_));
		}
	};

	// ADL (free swap) [add 02.06.2025]
	template <typename T, typename A>
	void swap(Vector<T, A> &x, Vector<T, A> &y) noexcept(noexcept(x.swap(y)))
	{
		x.swap(y);
	}

	// (spaceship mb добавить в будущем?)
	template <typename T, typename A>
	bool operator==(const Vector<T, A> &x, const Vector<T, A> &y)
	{
		if (x.size() != y.size())
			return false;
		return std::equal(x.begin(), x.end(), y.begin());
	}
	template <typename T, typename A>
	bool operator!=(const Vector<T, A> &x, const Vector<T, A> &y)
	{
		return !(x == y);
	}
	template <typename T, typename A>
	bool operator<(const Vector<T, A> &x, const Vector<T, A> &y)
	{
		return std::lexicographical_compare(x.begin(), x.end(),
											y.begin(), y.end());
	}
	template <typename T, typename A>
	bool operator>(const Vector<T, A> &x, const Vector<T, A> &y)
	{
		return y < x;
	}
	template <typename T, typename A>
	bool operator<=(const Vector<T, A> &x, const Vector<T, A> &y)
	{
		return !(y < x);
	}
	template <typename T, typename A>
	bool operator>=(const Vector<T, A> &x, const Vector<T, A> &y)
	{
		return !(x < y);
	}
}