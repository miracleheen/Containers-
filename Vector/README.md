<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License MIT">
  <img src="https://img.shields.io/badge/Build-passing-brightgreen.svg" alt="Build status">
</p>

# Vector

**Лёгкий, но мощный** контейнер `Vector<T,A>` — почти полный аналог `std::vector<T,A>` (C++17), написанный "с нуля" с поддержкой allocators, `emplace`, `insert`/`erase`, `reverse_iterator`, `assign`, `swap`, `compare` и многим другим.

---

## 🤙 Краткое введение

`Vector` — самодостаточный заголовочный файл, в котором:

- **Свой аллокатор** `Allocator<T>` (совместим со `std::allocator_traits`).  
- **Полный набор конструкторов**: default, fill, range, initializer_list, copy/move.  
- **Управление ёмкостью**: `reserve`, `shrink_to_fit`, `max_size`.  
- **Динамическое добавление**: `push_back`, `emplace_back`, `emplace`.  
- **Вставка и удаление**: `insert` (single/fill/range/list), `erase` (single/range).  
- **Удобные методы**: `assign`, `swap`, `clear`, `resize`, `front/back/data`.  
- **Итераторы**: forward, const, reverse.  
- **Операторы сравнения** и **free `swap`** для ADL.  
- **Специализация** `Vector<bool>` с упакованными битами.

Библиотека не требует внешних зависимостей: весь код в одном заголовочном файле `Vector.h`.

---

## ✨ Особенности и возможности

| Возможность                          | Описание                                                 |
|--------------------------------------|----------------------------------------------------------|
| **Allocator-aware**                  | Совместимость с любыми аллокаторами через `allocator_traits` |
| **Полный API `std::vector`**         | `assign`, `insert`, `erase`, `resize`, `reserve` и др.  |
| **`emplace` & `emplace_back`**       | Конструирование на месте без временных объектов          |
| **Reverse Iterators**                | `rbegin()`, `rend()`, `crbegin()`, `crend()`             |
| **Comparison & Swap**                | Операторы `==, !=, <, >, <=, >=` и ADL-`swap`            |
| **Exception Safety**                 | Strong guarantee в конструкторе копирования, базовая в других методах |
| **Header-only**                      | Один файл, можно легко встраивать в проект                  |
| **`Vector<bool>`**                   | Компактная упаковка битов                                |

---

## 📚 Небольшой пример использование (API)

```cpp
#include "Vector.h"     
using miv::Vector;

// Конструкторы
Vector<int> v1;                          // default
Vector<int> v2(5, 42);                   // 5 элементов = 42
Vector<int> v3{1,2,3,4};                 // initializer_list
Vector<int> v4(v3.begin(), v3.end());    // range ctor

// Capacity
v1.reserve(100);
size_t cap = v1.capacity();
size_t mx  = v1.max_size();

v1.assign(3, 7);     // {7,7,7}
v1.push_back(8);
v1.emplace_back(9);

// insert/erase
Vector<int> chunk{100,200};
v1.insert(v1.begin()+1, chunk.begin(), chunk.end()); 
v1.erase(v1.begin(), v1.begin()+2);

// elem access
int x  = v1[0];
int y  = v1.at(1);     // ex range_error
auto& r = v1.front();
auto& e = v1.back();
int* data = v1.data();

// iterators
for(auto it = v1.begin(); it != v1.end(); ++it) { /*...*/ }
for(auto rit = v1.rbegin(); rit != v1.rend(); ++rit) { /*...*/ }

// Swap - Comparison
Vector<std::string> a{"a","b"}, b{"A","B"};
std::swap(a,b);  // через free swap
bool eq = (a == b);

// 8) allocator
auto alloc = v1.get_allocator();
```
Исключения:
+  ```Range_error``` при выходе за границы в ```at()``` и некорректном ```operator[]``` в debug-режиме.
+  ```std::bad_alloc``` из ```Allocator::allocate``` при нехватке памяти.

## 🔗 Ресурсы и ссылки

- [std::vector (cppreference)](https://en.cppreference.com/w/cpp/container/vector)  
  Официальная документация по `std::vector` на cppreference.com.

- [LearnCpp](https://www.learncpp.com)
  Раздел **Dynamic arrays: std::vector**

- Youtube-каналы для изучения темы: 
> 1. [The Cherno](https://www.youtube.com/@TheCherno) => TheCherno
> 2. [CppCon: Andrei Alexandrescu ```std::allocator...```](https://www.youtube.com/@CppCon) => CppCon
> 3. [C++. Внутреннее устройство std::vector](https://www.youtube.com/@lectory_fpmi) => Лекторий ФПМИ
> 4. [C++ Weekly With Jason Turner](https://www.youtube.com/@cppweekly/videos) => Jason Turner

- [allocator_traits (cppreference)](https://en.cppreference.com/w/cpp/memory/allocator_traits)  
  Подробное описание `std::allocator_traits`.

- [StackOverflow](https://stackoverflow.com/q/30230989)</br>
  Тема: Как устроен `std::vector` 

- [```Notepad```](https://github.com/dasfex/notes/blob/master/README.md)</br>
  Неплохие записи для изучения. Можно пройтись по разделам касаемо ```vector```

- *Effective STL* (Скотт Майерс) — главы про контейнеры (книга для глубокого понимания идиом и оптимизаций STL в целом).

<p align="center">... to be continued🤡</p>
